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

namespace STI
{
namespace Utils
{

template<class T>
class DependencyTree
{
private:

	typedef typename bgl::adjacency_list<bgl::vecS, bgl::vecS, bgl::bidirectionalS, T, int> Graph;
	typedef typename bgl::graph_traits<Graph>::vertex_descriptor vertex_t;
	typedef typename bgl::graph_traits<Graph>::edge_descriptor edge_t;
	typedef typename std::map<const T, typename DependencyTree<T>::vertex_t> VertexMap;

public:
	DependencyTree() {}
	DependencyTree(const std::set<T>& nodes)
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);

		for (typename std::set<T>::iterator it = nodes.begin(); it != nodes.end(); ++it) {
			_addVertex(*it);
		}
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

		std::pair<edge_t, bool> result = bgl::add_edge(it_source->second, it_target->second, g);

		return result.second;
	}

	bool removeNode(const T& node)
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);

		typename VertexMap::iterator it = vertices.find(node);
		if (it != vertices.end()) {
			bgl::clear_vertex(it->second, g);	//can't use remove_vertex here because the graph is based on a vector, so the indices get messed up
			return true;
		}
		return false;
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
				count++;
			}

			return true;
		}
		return false;
	}

	bool sortTree(std::vector<T>& orderedNodes)
	{
		std::unique_lock< std::mutex > writeLock(graphMutex);

		orderedNodes.clear();
		std::vector<vertex_t> sortedNodes;		//vector of vertex_descriptors
		try {
			bgl::topological_sort(g, std::back_inserter(sortedNodes));
		}
		catch (bgl::not_a_dag&) {
			//Cycle detected: the graph is not a DAG.
			return false;
		}

		//topological_sort returns a vector with the most dependent vertex at the beginning.
		//sortTree returns the reverse of this, so the first element is the least dependent
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

private:

	void _addVertex(const T& vertex)
	{
		vertices.insert(std::pair<T, vertex_t>(vertex, bgl::add_vertex(vertex, g)));
	}


	
	Graph g;
	VertexMap vertices;

	mutable std::mutex graphMutex;

	bool findCycle(const T& index, std::vector<T>& cycle) const
	{
		std::vector<T> path;
		return findCycle_(index, path, cycle);
	}

	bool findCycle_(const T& index, std::vector<T> path, std::vector<T>& cycle) const
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
				loopFound = findCycle_(*it, newpath, cycle);
			}
		}

		return loopFound;
	}
};


} //Utils
} //STI

#endif
