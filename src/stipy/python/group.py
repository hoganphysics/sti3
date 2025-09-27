
from stipy.stipybase.stipybase import RawEventGroup
from stipy.python.stacktrace import makeStackTrace

_group = RawEventGroup.group
_var = RawEventGroup.var
_addvar = RawEventGroup._addvar
_addtag = RawEventGroup._addtag
_addEvent = RawEventGroup._addEvent
_addMeas = RawEventGroup._addMeas


def group(self, name, color="") :
    g = _group(self, name)
    if color != "" :
        g.setcolor(color)
    return g

def setcolor(self, color) :
    self.addMetaData("color", color)

def var(self, fullVarName) :
    v = _var(self, fullVarName, makeStackTrace())
    if v.isBound():
        return v.value().getValue()
    else:
        return v

def setvar(self, fullVarName, value):
    return _addvar(self, fullVarName, value, makeStackTrace())
def settag(self, fullTagName) :
    return _addtag(self, fullTagName, makeStackTrace())
def event(self, target, time, value) :
    return _addEvent(self, target, time, value, makeStackTrace())
def meas(self, target, time, value) :
    return _addMeas(self, target, time, value, makeStackTrace())



setattr(RawEventGroup, 'group', group)
setattr(RawEventGroup, 'setcolor', setcolor)

setattr(RawEventGroup, 'var', var)
setattr(RawEventGroup, 'setvar', setvar)
setattr(RawEventGroup, 'settag', settag)
setattr(RawEventGroup, 'event', event)
setattr(RawEventGroup, 'meas', meas)


class RawEventGroupNode:
    def __init__(self, group: RawEventGroup, nodeID: int):
        if not isinstance(group, RawEventGroup):
            raise TypeError("Expected RawEventGroup in group argument")
        if not isinstance(nodeID, int):
            raise TypeError("Expected int in nodeID argument")
        
        self.rawEventGroup = group
        self.nodeID = nodeID
        self.subgroups_indices = []

    def __getattr__(self, name):
        # Forward attribute access to the underlying RawEventGroup
        return getattr(self.rawEventGroup, name)
    def __repr__(self):
        return f"RawEventGroupNode(index={self.nodeID}, name={self.name()})"

    def name(self):
        return self.rawEventGroup.getName()

    def parentName(self):
        return self.rawEventGroup.getParentGroupName()
    
    def startTime(self):
        return self.rawEventGroup.startTime()

    def endTime(self):
        return self.rawEventGroup.endTime()

    def stats(self):
        return self.rawEventGroup.getStats()

    def referencePoints(self):
        return self.rawEventGroup.getReferencePoints()

    def subgroups(self) -> list[int]:
        return self.subgroups_indices
    
    def setSubgroups(self, indices: list[int]):
        if not isinstance(indices, list):
            raise TypeError("Expected list of integer ids")
        self.subgroups_indices = indices

    def events(self):
        return self.rawEventGroup.events()

    def vars(self):
        return self.rawEventGroup.vars()

    def tags(self):
        return self.rawEventGroup.tags()

    def overwrittenVars(self):
        return self.rawEventGroup.overwrittenVars()
    
    def metaData(self):
        return self.rawEventGroup.metaData()

    def stackTraceData(self):
        return self.rawEventGroup.getStackTraceData()



def _flatten(self):
    """
    Recursively flattens a RawEventGroup and its subgroups into a list of RawEventGroupNode,
    assigning each a unique integer ID and recording the IDs of its children.
    Returns a list of RawEventGroupNode.
    """
    nodes: list[RawEventGroupNode] = []

    def add_node(nodes, group, node_id):
        next_id = node_id
        node = RawEventGroupNode(group, next_id)
        nodes.append(node)
        next_id += 1

        child_ids = []
        for subgroup in group.subgroups():
            child_ids.append(next_id)
            next_id = add_node(nodes, subgroup, next_id)
        node.setSubgroups(child_ids)
        return next_id

    add_node(nodes, self, 0)
    return nodes

setattr(RawEventGroup, 'flatten', _flatten)

def _unflatten(nodes: list[RawEventGroupNode]) -> RawEventGroup:
    return _unflatten_list(nodes, 0)

def _unflatten_list(nodes: list[RawEventGroupNode], level) -> RawEventGroup:
    """
    Reconstructs a RawEventGroup from a flattened list of RawEventGroupNode.
    The level argument is used to determine the depth of the group.
    """
    if level < 0 or level >= len(nodes):
        raise IndexError("Level out of range")

    group = nodes[level].rawEventGroup
    for child_id in nodes[level].subgroups:
        child_group = _unflatten_list(nodes, child_id)
        group.addSubgroup(child_group)

    return group

setattr(RawEventGroup, 'unflatten', _unflatten)
