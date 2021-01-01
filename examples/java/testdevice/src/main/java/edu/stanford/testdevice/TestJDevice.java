
package edu.stanford.testdevice;

import edu.stanford.sti.*;

public class TestJDevice extends JLocalDevice {
    
    public TestJDevice(String name, String address, int module, String targetServer) {
        super(name, address, module, targetServer);
      
    //    test();
        
        addChannel(1, ChannelType.Output, MixedValueType.Empty, MixedValueType.Double, "testch"); //.addMetaData(key, value);

        JEventEngineScheduler engineScheduler = getEngineScheduler();
        engineScheduler.getId();
        

        JDeviceEventReceiver eventReceiver = this.getEventReceiver();

        //edu.stanford.sti.DeviceID sourceDeviceID = new DeviceID("dev2", "localhost", 0);
        edu.stanford.sti.DeviceEventListenerID listenerID = new DeviceEventListenerID();
        listenerID.setName("test");
        listenerID.setType(DeviceEventType.Refresh);
        RefreshDeviceEventListener listener = new RefreshDeviceEventListener() {
            public void handleEvent(edu.stanford.sti.RefreshDeviceEvent evt) {
                System.out.println(evt.sourceID().getID());
            }
        };

        eventReceiver.addListener(getID(), listenerID, listener);
        

        EngineID id = new EngineID((short) 0);

        addEventEngine(id);
    }

    public void parseEvents(int temp)
    {
        System.out.println("parsing in java");
    }
}
