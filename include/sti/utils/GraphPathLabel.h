#ifndef STI_UTILS_GRAPHPATHLABEL_H
#define STI_UTILS_GRAPHPATHLABEL_H

#include <vector>

namespace STI
{
namespace Utils
{

typedef std::vector<unsigned> GraphPathLabel;

//This class is for graphs where the child nodes of a given parent node are given a unique number
//label associated with that parent. Subsequent grandchildren of a child nodes inherit this label,
//as well as an additional number associated with the child. When labeled in this way, each
//node has a unique label that describes the path to the node from the root node.

//template<class T>
//class GraphPathLabel
//{
//private:
//	GraphPathLabel() {}
//	void add(const T& label) { graphPath.push_back(label); }
//
//	bool operator<(const GraphPathLabel& rhs) const { return graphPath < rhs.graphPath; }
//	bool operator==(const GraphPathLabel& rhs) const { return parseID == rhs.parseID && shotTimeStamp == rhs.shotTimeStamp; }
//	bool operator!=(const GraphPathLabel& rhs) const { return !((*this) == rhs); }
//
//
//private:
//	std::vector<T> graphPath;
//};


} // UTILS
} // STI


#endif
