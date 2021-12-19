%module(directors="1") sti
//%feature("director");

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


//Utils
%include FileHolder.i
%include MixedValue.i
%include MetaData.i

%include DeviceID.i
%include attributes.i
%include device3.i

%include network.i

%include ShotResult.i

%include EventEngineDependencyTree.i

%include JEventEngine.i

%include DeviceIDIndexedGraph.i

%include JEventEngineJob.i

%include JEventEngineScheduler.i

