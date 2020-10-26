#include "utils/DependencyTree.h"


#include <vector>
#include <iostream>
using std::cout;
using std::endl;


void printNodes(const std::vector<int>& nodes)
{
    std::cout << "(";
    for(auto& node : nodes) {
        std::cout << node << ", ";
    }
    std::cout << ")" << std::endl;
}

int main(int argc, char **argv)
{

    STI::Utils::DependencyTree<int> tree;

    tree.addVertex(1);
    tree.addEdge(1, 2);
    tree.addEdge(1, 9);
    tree.addEdge(1, 3);

    tree.addEdge(2, 4);
    tree.addEdge(2, 5);
    tree.addEdge(2, 6);

    tree.addEdge(3, 7);
    tree.addEdge(3, 8);

    tree.addEdge(4, 5);
    tree.addEdge(8, 5);
    tree.addEdge(6, 7);
    tree.addEdge(7, 9);

    //tree.addEdge(8, 1);       //causes cycle

    std::vector<int> orderedNodes;
    bool success = tree.sortTree(orderedNodes);

    std::cout << (success ? "DAG:" : "Not DAG:") << " ";
    printNodes(orderedNodes);
    
    if(!success) {
        std::vector<int> cycle;
        if(tree.getCycle(cycle)) {
            std::cout << " cycle: ";
            printNodes(cycle);
        }
    }

    int tmp2;
    tree.getDependentNodeCount(7, tmp2);
    std::cout << "Count: " << tmp2 << std::endl;

    tree.getDependedentNodes(3, orderedNodes);
    printNodes(orderedNodes);

    if(tree.isDependedentNode(1, 7)) {
        std::cout << "Yes" << std::endl;
    }

    STI::Utils::DependencyTree<int> subtree;
    tree.getSubtree(2, subtree);
    success = subtree.sortTree(orderedNodes);
    std::cout << "Subtree: " << (success ? "DAG:" : "Not DAG:") << " ";
    printNodes(orderedNodes);
    subtree.getDependedentNodes(7, orderedNodes);
    printNodes(orderedNodes);

    int tmp;
    subtree.getDependentNodeCount(8, tmp);
    std::cout << "Count: " << tmp << std::endl;

    // STI::Utils::DependencyTree<int> testtree;
    // testtree.addVertex(10);
    // testtree.addEdge(10, 1);
    // testtree.addTree(tree);
    // success = testtree.sortTree(orderedNodes);
    // std::cout << "Test tree: " << (success ? "DAG:" : "Not DAG:") << " ";
    // printNodes(orderedNodes);

	return 0;
}