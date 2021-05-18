
package edu.stanford.testdevice;

import edu.stanford.sti.*;

public class TestJDevice extends JLocalDevice {
    
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
        listenerID.setName("test");
        listenerID.setType(DeviceMessageType.Refresh);
        edu.stanford.sti.RefreshDeviceMessageListener listener = new edu.stanford.sti.RefreshDeviceMessageListener() {
            public void handleMessage(edu.stanford.sti.RefreshDeviceMessage mess) {
                System.out.println(mess.sourceID().getID());
            }
        };
        messageReceiver.addListener(getID(), listenerID, listener);
      


        edu.stanford.sti.DeviceMessageListenerID listenerID2 = new DeviceMessageListenerID();
        listenerID2.setName("enginelistener");
        listenerID2.setType(DeviceMessageType.EngineScheduler);
        edu.stanford.sti.EngineSchedulerMessageListener listener2 = new edu.stanford.sti.EngineSchedulerMessageListener() {
            public void handleMessage(edu.stanford.sti.EngineSchedulerMessage mess) {
                System.out.println("Engine Message:" + mess.sourceID().getID());
                System.out.println("Engine state:" + mess.getEngineState());
            }
        };
        messageReceiver.addListener(serverID, listenerID2, listener2);


        EngineID id = new EngineID((short) 0);

        addEventEngine(id);
    }

    public void parseEvents(int temp)
    {
        System.out.println("parsing in java");
    }
}
