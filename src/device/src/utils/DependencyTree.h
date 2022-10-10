#ifndef STI_UTILS_DEPENDENCYTREE_H
#define STI_UTILS_DEPENDENCYTREE_H

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
namespace bgl = boost;

#include <mutex>
#include <utility>
#include <set>
#include <map>
#include <list>

namespace STI
{
namespace Utils
{

template<class T> class DependencyTree;


/// Really should be DependencyGraph.
template<class T>
class DependencyTree
{
private:

//	typedef typename bgl::adjacency_list<bgl::vecS, bgl::vecS, bgl::bidirectionalS, T, int> Graph;
	typedef typename bgl::adjacency_list<bgl::vecS, bgl::vecS, bgl::bidirectionalS, T, int> Graph;
	typedef typename bgl::graph_traits<Graph>::vertex_descriptor vertex_t;
	typedef typename bgl::graph_traits<Graph>::edge_descriptor edge_t;
	typedef typename std::map<const T, typename DependencyTree<T>::vertex_t> VertexMap;

public:

	DependencyTree() : sortedDAG(false) {}
	DependencyTree(const std::set<T>& nodes) : sortedDAG(false)
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);

		for (typename std::set<T>::iterator it = nodes.begin(); it != nodes.end(); ++it) {
			_addVertex(*it);
		}
	}

	DependencyTree(const DependencyTree<T>& tree)
	{
		g = tree.g;
		vertices = tree.vertices;
		sortedDAG = tree.sortedDAG;
	}

	virtual ~DependencyTree() {}

	DependencyTree<T>& operator=(const STI::Utils::DependencyTree<T>& tree)
	{
		g = tree.g;
		vertices = tree.vertices;
		sortedDAG = tree.sortedDAG;

		return (*this);
	}

	void addTree(const DependencyTree<T>& tree)
	{
		std::vector<T> nodes;
		std::vector<T> depNodes;

		tree.getNodes(nodes);

		for(auto& node : nodes) {
			if(!hasVertex(node)) {
				addVertex(node);
			}

			tree.getDependedentNodes(node, depNodes);

			for(auto& dep : depNodes) {
				addEdge(node, dep);
			}
		}
	}

	bool getSubtree(const T& vertex, DependencyTree<T>& tree)
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);

		//Check if it's a DAG (sort if not sorted)
		if (!sortedDAG) {
			std::vector<T> orderedNodes;
			if(!_sortTree(orderedNodes)) {
				return false;
			}
		}

		tree.clear();

		return _getSubtree(vertex, tree);
	}

	bool hasVertex(const T& vertex) const
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);

		return _hasVertex(vertex);
	}

	void addVertex(const T& vertex)
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);
		_addVertex(vertex);
	}

	bool addEdge(const T& source, const T& target)
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);

		typename VertexMap::iterator it_source = vertices.find(source);
		typename VertexMap::iterator it_target = vertices.find(target);

		if (it_source == vertices.end()) {
			return false;
		}

		if (it_target == vertices.end()) {
			//new target vertex
			_addVertex(target);

			it_target = vertices.find(target);
		}

		if (it_target == vertices.end()) {
			return false;
		}

		if(_hasEdge(it_source, target)) {
			return false;
		}

		std::pair<edge_t, bool> result = bgl::add_edge(it_source->second, it_target->second, g);
		sortedDAG = false;

		return result.second;
	}

	bool removeNode(const T& node)
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);
		return _removeNode(node);
	}

	void getNodes(std::vector<T>& nodes) const
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);
		_getNodes(nodes);
	}

	void getNodes(std::set<T>& nodes) const
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);
		_getNodes(nodes);
	}
	
	void getDependedentNodes(const T& node, std::vector<T>& depNodes) const
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);
		_getDependedentNodes(node, depNodes);
	}
	
	void getParentNodes(const T& node, std::vector<T>& parentNodes) const
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);

		typename VertexMap::const_iterator it = vertices.find(node);
		if (it != vertices.end()) {

			typename bgl::graph_traits <Graph>::in_edge_iterator ei, ei_end;
			for (bgl::tie(ei, ei_end) = in_edges(it->second, g); ei != ei_end; ++ei) {
				parentNodes.push_back( g[bgl::source(*ei, g)] );
			}
		}
	}

	bool isDependedentNode(const T& source, const T& target) const
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);
		
		typename VertexMap::const_iterator it_source = vertices.find(source);
		typename VertexMap::const_iterator it_target = vertices.find(target);
		if (it_source == vertices.end() || it_target == vertices.end())
			return false;
		
		typename bgl::graph_traits <Graph>::out_edge_iterator ei, ei_end;
		for (bgl::tie(ei, ei_end) = out_edges(it_source->second, g); ei != ei_end; ++ei) {

			if (g[bgl::target(*ei, g)] == target) {
				return true;
			}
		}
		return false;
	}

	bool getDependentNodeCount(const T& node, int& count) const
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);

		count = 0;
		typename VertexMap::const_iterator it = vertices.find(node);
		if (it != vertices.end()) {

			typename bgl::graph_traits <Graph>::in_edge_iterator ei, ei_end;
			for (bgl::tie(ei, ei_end) = in_edges(it->second, g); ei != ei_end; ++ei) {
			// typename bgl::graph_traits <Graph>::out_edge_iterator ei, ei_end;
			// for (bgl::tie(ei, ei_end) = out_edges(it->second, g); ei != ei_end; ++ei) {
				count++;
			}

			return true;
		}
		return false;
	}

	bool sortTree(std::vector<T>& orderedNodes)
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);
		return _sortTree(orderedNodes);
	}

	bool isSorted() const
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);
		return sortedDAG;
	}

	//This should only be called if sortTree failed (because there is a cycle.
	bool getCycle(std::vector<T>& cycle) const
	{
		//returns the first cycle in the graph

		std::unique_lock< std::mutex > writeLock(graphMutex);
		typename VertexMap::const_iterator it;

		bool cycleFound = false;
		for (it = vertices.begin(); it != vertices.end() && !cycleFound; ++it) {
			cycleFound = findCycle(it->first, cycle);
		}

		return cycleFound;
	}

	bool hasCycle() const
	{
		std::vector<T> cycle;
		return getCycle(cycle);
	}

	unsigned vertexCount()
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);
		return static_cast<unsigned>(vertices.size());
	}

	void clear()
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);

		std::vector<T> nodes;
		_getNodes(nodes);

		for(auto& node : nodes) {
			_removeNode(node);
		}

		//removeNode doesn't actually completely remove unfortunately, it just clears edges for the vertex.
		//This is because removing a node would invalidate all the other vertex numbers and edges...
		//So to completely clear we have to swap in a new Graph instance.
		Graph newG;
		g.swap(newG);
		vertices.clear();
	}

private:

	bool _hasVertex(const T& vertex) const
	{
		auto it = vertices.find(vertex);

		return (it != vertices.end());
	}

	bool _sortTree(std::vector<T>& orderedNodes)
	{
		orderedNodes.clear();
		std::vector<vertex_t> sortedNodes;		//vector of vertex_descriptors
//		std::set<vertex_t> sortedNodes;		//vector of vertex_descriptors
		try {
			bgl::topological_sort(g, std::back_inserter(sortedNodes));
//			bgl::topological_sort(g, std::back_inserter< std::list<T> >(sortedNodes));
			sortedDAG = true;
		}
		catch (bgl::not_a_dag&) {
			//Cycle detected: the graph is not a DAG.
			return false;
		}

		//topological_sort returns a vector with the most dependent vertex at the beginning.
		//sortTree returns the reverse of this, so the first element is the least dependent
//		for (typename std::list<vertex_t>::reverse_iterator it = sortedNodes.rbegin(); it != sortedNodes.rend(); ++it) {
		for (typename std::vector<vertex_t>::reverse_iterator it = sortedNodes.rbegin(); it != sortedNodes.rend(); ++it) {
			orderedNodes.push_back(g[*it]);
		}

		//bgl::graph_traits <Graph>::out_edge_iterator ei, ei_end;
		//for (bgl::tie(ei, ei_end) = out_edges(sortedNodes.at(0), g); ei != ei_end; ++ei) {
		//	auto source = bgl::source(*ei, g);
		//	auto target = bgl::target(*ei, g);
		//	std::cout << "There is an edge from " << source << " to " << target << std::endl;
		//}

		return true;
	}

	void _getNodes(std::vector<T>& nodes) const
	{
		nodes.clear();

		for(auto& vertex : vertices) {
			nodes.push_back(vertex.first);
		}
	}
	void _getNodes(std::set<T>& nodes) const
	{
		nodes.clear();

		for(auto& vertex : vertices) {
			nodes.insert(vertex.first);
		}
	}
	
	void _getDependedentNodes(const T& node, std::vector<T>& depNodes) const
	{
		depNodes.clear();

		typename VertexMap::const_iterator it_node = vertices.find(node);
		if( it_node == vertices.end() ) {
			return;
		}
		
		typename bgl::graph_traits <Graph>::out_edge_iterator ei, ei_end;
		for (bgl::tie(ei, ei_end) = out_edges(it_node->second, g); ei != ei_end; ++ei) {
			
			depNodes.push_back( g[bgl::target(*ei, g)] );
		}
	}

	
	bool _hasEdge(const typename VertexMap::const_iterator& it_node, const T& target) const
	{
		typename bgl::graph_traits <Graph>::out_edge_iterator ei, ei_end;
		for (bgl::tie(ei, ei_end) = out_edges(it_node->second, g); ei != ei_end; ++ei) {
			
			if(target == g[bgl::target(*ei, g)]) {
				return true;
			}
		}
		return false;
	}


	void _addVertex(const T& vertex)
	{
		if (_hasVertex(vertex)) {
			return;
		}

		vertices.insert(std::pair<T, vertex_t>(vertex, bgl::add_vertex(vertex, g)));
		sortedDAG = false;
	}

	bool _removeNode(const T& node)
	{
		//There's a pretty serious issue with _removeNode that is hard to work around.  For topological sort to work
		//the Graph (seems!) to need to use the vecS template parameter (std::vector) to store the vertices.  But 
		//vector-based graphs do not let you remove a vertex cleanly, because removing one vertex invalidates all the 
		//other vertices and edge connections.  I think this is because everthing is just index based, and the vector 
		//indices change when an element is erased.  So instead of calling remove_vertex here, we just call clear_vertex,
		//which has the effect of removing all edge connections but leaves the vertex itself in the Graph.  We also
		//remove the vertex from the VertexMap, so at least hasVertex() will return false, even though the vertex is
		//still stored internally inside the Graph.
		//This issue could probably be avoided by using listS instead of vecS in the Graph, but this choice didn't seem to
		//work with topological_sort.  So this is a compromise.
		typename VertexMap::iterator it = vertices.find(node);
		if (it != vertices.end()) {
			bgl::clear_vertex(it->second, g);	//can't use remove_vertex here because the graph is based on a vector, so the indices get messed up
			//bgl::remove_vertex(it->second, g);
			sortedDAG = false;
			vertices.erase(it);
			return true;
		}
		return false;
	}
		
	bool _getSubtree(const T& vertex, DependencyTree<T>& tree)
	{
		if (!_hasVertex(vertex)) {
			return false;
		}
		
		tree.addVertex(vertex);

		std::vector<T> depNodes;
		_getDependedentNodes(vertex, depNodes);

		DependencyTree<T> subtree;
		for(auto& node : depNodes) {
			tree.addEdge(vertex, node);
			
			subtree.clear();
			_getSubtree(node, subtree);
			tree.addTree(subtree);
		}
		return true;
	}

	bool findCycle(const T& index, std::vector<T>& cycle) const
	{
		std::vector<T> path;
		return _findCycle(index, path, cycle);
	}

	bool _findCycle(const T& index, std::vector<T> path, std::vector<T>& cycle) const
	{
		//recursive DFS of graph, starting from index.
		if (std::find(path.begin(), path.end(), index) != path.end()) {
			//cycle found
			cycle = path;
			cycle.push_back(index);
			return true;
		}

		std::vector<T> newpath = path;
		newpath.push_back(index);
		std::vector<T> children;

		typename VertexMap::const_iterator it_source = vertices.find(index);
		typename bgl::graph_traits <Graph>::out_edge_iterator ei, ei_end;
		for (bgl::tie(ei, ei_end) = out_edges(it_source->second, g); ei != ei_end; ++ei) {
			vertex_t child = bgl::target(*ei, g);
			children.push_back(g[child]);
		}

		bool loopFound = false;
		if (children.size() > 0) {
			for (typename std::vector<T>::iterator it = children.begin(); it != children.end() && !loopFound; ++it) {
				loopFound = _findCycle(*it, newpath, cycle);
			}
		}

		return loopFound;
	}


	Graph g;
	VertexMap vertices;

	mutable std::mutex graphMutex;

	bool sortedDAG;

};


} //Utils
} //STI

#endif
