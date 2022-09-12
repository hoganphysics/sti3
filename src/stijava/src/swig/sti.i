%module(directors="1") sti
%feature("director");


%rename(opEquals) operator==;
%rename(opLess) operator<;
%rename(opNotEquals) operator!=;
%rename(opEvaluate) operator();

%include utils.i

%include EventEngineDependencyTree.i

%include StackTraceData.i

%include RawEventGroup.i

%include device.i

%include ParseResult.i

%include ShotResult.i



%include JEventEngine.i

%include JEventEngineJob.i



