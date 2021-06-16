
package edu.stanford.testdevice;

import edu.stanford.sti.*;
import edu.stanford.sti.JEngineJobUpdateDeviceMessageListener;
import edu.stanford.sti.EngineStateMessageListener;
import edu.stanford.sti.EngineParsingMessage;


public class TestJDevice extends JLocalDevice {
    
    private JEngineJobUpdateDeviceMessageListener joblistener;
    private EngineStateMessageListener statelistener;


    public TestJDevice(String name, String address, int module, String targetServer) {
        super(name, address, module, targetServer);
      
    //    test();
    
        edu.stanford.sti.DeviceID serverID = new edu.stanford.sti.DeviceID("STI Server", "localhost", 0);
        addPartner(serverID);
        
        addChannel(1, ChannelType.Output, MixedValueType.Empty, MixedValueType.Double, "testch"); //.addMetaData(key, value);

        JEventEngineScheduler engineScheduler = getEngineScheduler();
        engineScheduler.getId();
        

        JDeviceMessageReceiver messageReceiver = this.getMessageReceiver();

        //edu.stanford.sti.DeviceID sourceDeviceID = new DeviceID("dev2", "localhost", 0);
        edu.stanford.sti.DeviceMessageListenerID listenerID = new DeviceMessageListenerID();
        listenerID.setName("Test JobUpdate");
        listenerID.setType(DeviceMessageType.EngineJobUpdate);
        
        joblistener = new edu.stanford.sti.JEngineJobUpdateDeviceMessageListener() {
            public void handleMessage(edu.stanford.sti.JEngineJobUpdateDeviceMessage mess) {
                // System.out.println("Job:" + mess.getJEngineJob().getJobID().getPid().getParseTimestamp().getTimestamp());
                System.out.println("Job: " + mess.getTargetList().toString() + " : " + mess.getDeviceTrace().print());
            }
        };
        messageReceiver.addListener(serverID, listenerID, joblistener);
      


        edu.stanford.sti.DeviceMessageListenerID listenerID2 = new DeviceMessageListenerID();
        listenerID2.setName("enginelistener");
        listenerID2.setType(DeviceMessageType.EngineScheduler);
        edu.stanford.sti.EngineSchedulerMessageListener listener2 = new edu.stanford.sti.EngineSchedulerMessageListener() {
            public void handleMessage(edu.stanford.sti.EngineSchedulerMessage mess) {
                // System.out.println("Engine Message:" + mess.sourceID().getID());
                // System.out.println("Engine state:" + mess.getEngineState());
                System.out.println("Scheduler Message: " + mess.getSchedulerMessageType().toString());
                for (EngineParsingMessage m : mess.getMessages()) {
                    System.out.println("---- Message: " + m.getMessage());
                    
                }
            }
        };
        messageReceiver.addListener(serverID, listenerID2, listener2);


        edu.stanford.sti.DeviceMessageListenerID listenerID3 = new DeviceMessageListenerID();
        listenerID3.setName("enginestatelistener");
        listenerID3.setType(DeviceMessageType.EngineStatus);
        statelistener = new EngineStateMessageListener() {
            public void handleMessage(edu.stanford.sti.EngineStateMessage mess) {
                // System.out.println("Engine Message:" + mess.sourceID().getID());
                System.out.println("Engine state:" + mess.getEngineStates().values().iterator().next());
            }
        };
        messageReceiver.addListener(serverID, listenerID3, statelistener);


        EngineID id = new EngineID((short) 0);
        addEventEngine(id);
    }

    public void parseEvents(int temp)
    {
        System.out.println("parsing in java");
    }
}
