#ifndef STI_UTILS_INDEXEDGRAPH_H
#define STI_UTILS_INDEXEDGRAPH_H

#include "DependencyTree.h"

#include <vector>
#include <map>
#include <algorithm>

namespace STI
{
namespace Utils
{

///Used to convert DependencyTree into a flat, index-based graph for transport.

template<class T>
class IndexedGraphNode
{
public:
    T node;
    std::vector<unsigned> outConnections;
};


template<class T>
class IndexedGraph
{
public:

    IndexedGraph();
    IndexedGraph(const DependencyTree<T>& tree);
    virtual ~IndexedGraph();

    const std::vector<IndexedGraphNode<T>>& getNodes() const;

    int indexOf(const T& node) const;
    bool contains(const T& node) const;

    void addNode(const T& node);
    bool addEdge(const T& source, const T& target);

private:

    std::vector<IndexedGraphNode<T>> nodes;

};


} //Utils
} //STI


template<class T>
STI::Utils::IndexedGraph<T>::IndexedGraph()
{
}

template<class T>
STI::Utils::IndexedGraph<T>::IndexedGraph(const STI::Utils::DependencyTree<T>& tree)
{
    std::vector<T> treeNodes;
    tree.getNodes(treeNodes);

    //setup vertexMap (T -> index)
    std::map<T, unsigned> vertexMap;
    for (unsigned i = 0; i < treeNodes.size(); ++i) {
        vertexMap[treeNodes.at(i)] = i;
    }
    
    std::vector<T> outNodes;

    for (unsigned i = 0; i < treeNodes.size(); ++i) {
        IndexedGraphNode<T> newNode;
        newNode.node = treeNodes.at(i);
        
        tree.getDependedentNodes(treeNodes.at(i), outNodes);
        
        for (auto& o : outNodes) {
            newNode.outConnections.push_back( vertexMap[o] );
        }

        nodes.push_back(newNode);
    }
}

template<class T>
STI::Utils::IndexedGraph<T>::~IndexedGraph()
{
}


template<class T>
const std::vector<STI::Utils::IndexedGraphNode<T>>& STI::Utils::IndexedGraph<T>::getNodes() const
{
    return nodes;
}

template<class T>
int STI::Utils::IndexedGraph<T>::indexOf(const T& node) const
{
    auto it = std::find_if(nodes.begin(), nodes.end(), 
        [&node](const IndexedGraphNode<T>& gn) {
            return gn.node == node;
        });
    if (it == nodes.end()) return -1;   //not found

    return static_cast<int>(it - nodes.begin());
}

template<class T>
bool STI::Utils::IndexedGraph<T>::contains(const T& node) const
{
    return indexOf(node) >= 0;
}

template<class T>
void STI::Utils::IndexedGraph<T>::addNode(const T& node)
{
    if (!contains(node)) {
        IndexedGraphNode<T> newNode;
        newNode.node = node;
        nodes.push_back(newNode);
    }
}

template<class T>
bool STI::Utils::IndexedGraph<T>::addEdge(const T& source, const T& target)
{
    int s = indexOf(source);
    int t = indexOf(source);

    if (s < 0 || t < 0) return false;   //one or both nodes are not found

    //add edge
    auto& connections = nodes.at(s).outConnections;
    connections.push_back(t);

    //remove any duplicates
    std::sort( connections.begin(), connections.end() );
    connections.erase( std::unique( connections.begin(), connections.end() ), connections.end() );

    return true;
}


#endif
