%module(directors="1") sti

%feature("director");

%include "std_string.i"
%include "std_shared_ptr.i"
%include "std_set.i"
%include "std_vector.i"
%include "std_map.i"
%include "std_pair.i"
%include "typemaps.i"


%rename(opEquals) operator==;
%rename(opLess) operator<;
%rename(opNotEquals) operator!=;
%rename(opEvaluate) operator();
%rename(opAddAssign) operator+=;
%rename(opAssign) operator=;


//Utils
%include utils.i
%include TimeStamp.i
%include FileID.i
%include FileHolder.i
%include BinaryData.i
%include MixedValue.i
%include MetaData.i
%include FileServer.i
%include Image.i

%include EngineJobSourceID.i

%include SequenceID.i

%include EventEngineDependencyTree.i
%include StackTraceData.i
%include RawEventGroup.i

%include DeviceID.i
%include attributes.i
%include device.i

%include network.i



%include ParseResult.i
%include ShotResult.i

%include JEventEngine.i
%include DeviceIDIndexedGraph.i
%include JEventEngineJob.i

%include Sequence.i
%include JEventEngineScheduler.i

%include LogManager.i
%include TaskManager.i
%include ProfileManager.i

