#include <catch2/catch_test_macros.hpp>

#include "utils/DependencyTree.h"

#include <algorithm>
#include <set>
#include <vector>

using STI::Utils::DependencyTree;

namespace {

template <typename T>
std::size_t indexOf(const std::vector<T>& values, const T& value)
{
    auto it = std::find(values.begin(), values.end(), value);
    REQUIRE(it != values.end());
    return static_cast<std::size_t>(std::distance(values.begin(), it));
}

template <typename T>
std::set<T> asSet(const std::vector<T>& values)
{
    return std::set<T>(values.begin(), values.end());
}

} // namespace

TEST_CASE("DependencyTree sorts DAGs and reports direct relationships", "[dependencytree][utils]")
{
    DependencyTree<int> tree;
    tree.addVertex(1);

    CHECK_FALSE(tree.addEdge(42, 1));
    REQUIRE(tree.addEdge(1, 2));
    CHECK_FALSE(tree.addEdge(1, 2));
    REQUIRE(tree.addEdge(1, 9));
    REQUIRE(tree.addEdge(1, 3));
    REQUIRE(tree.addEdge(2, 4));
    REQUIRE(tree.addEdge(2, 5));
    REQUIRE(tree.addEdge(2, 6));
    REQUIRE(tree.addEdge(3, 7));
    REQUIRE(tree.addEdge(3, 8));
    REQUIRE(tree.addEdge(4, 5));
    REQUIRE(tree.addEdge(8, 5));
    REQUIRE(tree.addEdge(6, 7));
    REQUIRE(tree.addEdge(7, 9));

    std::vector<int> orderedNodes;
    REQUIRE(tree.sortTree(orderedNodes));
    CHECK(orderedNodes.size() == 9);

    CHECK(indexOf(orderedNodes, 1) < indexOf(orderedNodes, 2));
    CHECK(indexOf(orderedNodes, 1) < indexOf(orderedNodes, 3));
    CHECK(indexOf(orderedNodes, 2) < indexOf(orderedNodes, 4));
    CHECK(indexOf(orderedNodes, 4) < indexOf(orderedNodes, 5));
    CHECK(indexOf(orderedNodes, 6) < indexOf(orderedNodes, 7));
    CHECK(indexOf(orderedNodes, 7) < indexOf(orderedNodes, 9));

    int count = -1;
    REQUIRE(tree.getDependentNodeCount(1, count));
    CHECK(count == 0);
    REQUIRE(tree.getDependentNodeCount(5, count));
    CHECK(count == 3);
    CHECK_FALSE(tree.getDependentNodeCount(99, count));

    std::vector<int> dependentNodes;
    tree.getDependedentNodes(3, dependentNodes);
    CHECK(asSet(dependentNodes) == std::set<int>{7, 8});

    std::vector<int> parentNodes;
    tree.getParentNodes(5, parentNodes);
    CHECK(asSet(parentNodes) == std::set<int>{2, 4, 8});

    CHECK(tree.isDependedentNode(1, 3));
    CHECK_FALSE(tree.isDependedentNode(1, 7));
}

TEST_CASE("DependencyTree extracts subtrees and merges trees", "[dependencytree][utils]")
{
    DependencyTree<int> tree;
    tree.addVertex(1);
    REQUIRE(tree.addEdge(1, 2));
    REQUIRE(tree.addEdge(1, 3));
    REQUIRE(tree.addEdge(2, 4));
    REQUIRE(tree.addEdge(3, 5));
    REQUIRE(tree.addEdge(5, 6));

    DependencyTree<int> subtree;
    REQUIRE(tree.getSubtree(3, subtree));

    std::vector<int> subtreeNodes;
    subtree.getNodes(subtreeNodes);
    CHECK(asSet(subtreeNodes) == std::set<int>{3, 5, 6});

    std::vector<int> orderedSubtree;
    REQUIRE(subtree.sortTree(orderedSubtree));
    CHECK(indexOf(orderedSubtree, 3) < indexOf(orderedSubtree, 5));
    CHECK(indexOf(orderedSubtree, 5) < indexOf(orderedSubtree, 6));

    DependencyTree<int> merged;
    merged.addVertex(10);
    REQUIRE(merged.addEdge(10, 1));
    merged.addTree(tree);

    std::vector<int> orderedMerged;
    REQUIRE(merged.sortTree(orderedMerged));
    CHECK(indexOf(orderedMerged, 10) < indexOf(orderedMerged, 1));
    CHECK(indexOf(orderedMerged, 1) < indexOf(orderedMerged, 2));
    CHECK(indexOf(orderedMerged, 3) < indexOf(orderedMerged, 5));

    CHECK(merged.removeNode(5));
    CHECK_FALSE(merged.hasVertex(5));
    CHECK_FALSE(merged.isDependedentNode(3, 5));

    merged.clear();
    CHECK(merged.vertexCount() == 0);
}

TEST_CASE("DependencyTree detects cycles", "[dependencytree][utils]")
{
    DependencyTree<int> tree;
    tree.addVertex(1);
    REQUIRE(tree.addEdge(1, 2));
    REQUIRE(tree.addEdge(2, 3));
    REQUIRE(tree.addEdge(3, 1));

    std::vector<int> orderedNodes;
    CHECK_FALSE(tree.sortTree(orderedNodes));
    CHECK(tree.hasCycle());

    std::vector<int> cycle;
    REQUIRE(tree.getCycle(cycle));
    REQUIRE(cycle.size() >= 4);
    CHECK(cycle.front() == cycle.back());
    CHECK(asSet(cycle) == std::set<int>{1, 2, 3});

    DependencyTree<int> subtree;
    CHECK_FALSE(tree.getSubtree(1, subtree));
}
