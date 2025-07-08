from stipy.bin.stipybase import MixedValue
from stipy.bin.stipybase import MixedValueType

from typing import List
import copy

def getIter(self):
    if self.isType(MixedValueType.Vector):
        return iter(self.getValue())
    else:
        raise TypeError("Attempted to use an iterator on a MixedValue that is not a Vector")

setattr(MixedValue, '__iter__', getIter)

class MixedValueNode:
    def __init__(self, value, index, type=None):
        if type != None and not isinstance(type, MixedValueType):
            raise TypeError("Expected MixedValueType in type argument")
        if not isinstance(value, MixedValue):
            if type == None:
                raise TypeError("Expected MixedValueType in type argument")
            self.value = value
            self.type = type
        else:   # value is a MixedValue
            self.value = value.getValue()
            self.type = value.getType()
        
        self.index = copy.deepcopy(index)

    def __repr__(self):
        return f"MixedValueNode(index={self.index}, type={self.type}, value={self.value})"

    def __lt__(self, other):
        if self.index == None or len(self.index) == 0:
            return True
        if other.index == None or len(other.index) == 0:
            return False

        min_len = min(len(self.index), len(other.index))
        
        if len(self.index) == len(other.index):
            return self.index[-1] < other.index[-1]
        else:
            return self.index[min_len - 1] < other.index[min_len - 1]


def _flatten(self):
    """
    Depth-first traversal of the MixedValue tree, emitting a list of
    MixedValueNodeModel instances.
    """
    out: list[MixedValueNode] = []

    def dfs(val: MixedValue, path: list[int]) -> None:
        if val.getType() == MixedValueType.Vector:
            # recurse into children
            for i in range(len(val)):
                path.append(i)                            # open new level
                dfs(val[i], path)
                path.pop()                                # close level
        else:
            # leaf ➜ emit node
            out.append(MixedValueNode(val, path))

    dfs(self, [])
    return out

setattr(MixedValue, 'flatten', _flatten)

def _unflatten_vector(nodes: List[MixedValueNode], level) -> MixedValue:
    # Vectors can be nested arbitrarily deeply.
    # The index array indicates the position within the nested hierarchy
    # at each level. So an index [1, 4] is a node at position 1 at level 0
    # and position 4 at level 1.

    out = MixedValue()
    i = 0

    while i < len(nodes):
        if len(nodes[i].index) - 1 == level or len(nodes[i].index) == 0:
            # catches missing index information to avoid infinite recursion
            out.addValue(nodes[i].value)
            i += 1
        else:
            # recurse into sublist
            sublist_start = i
            sublist_end = len(nodes)
            for j in range(i+1, len(nodes)):
                if len(nodes[j].index)-1 == level:
                    sublist_end = j
                    break
            
            out.addValue(_unflatten_vector(nodes[sublist_start:sublist_end], level + 1))
            i = sublist_end

    return out


def _unflatten(nodes: List[MixedValueNode]) -> MixedValue:
    """
    Reconstruct a MixedValue tree from a list of MixedValueNode instances.
    """
    if type(nodes) != list:
        raise TypeError("Expected a list of MixedValueNode instances")
    if len(nodes) == 0:
        return MixedValue()     # empty tree
    if not all(isinstance(node, MixedValueNode) for node in nodes):
        raise TypeError("Expected a list of MixedValueNode instances")
    
    nodes.sort()

    if len(nodes) == 1:
        return MixedValue(nodes[0].value)
    
    return _unflatten_vector(nodes, 0)

setattr(MixedValue, 'unflatten', _unflatten)
