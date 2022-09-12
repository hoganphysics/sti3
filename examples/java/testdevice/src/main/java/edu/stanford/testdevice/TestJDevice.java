
package edu.stanford.testdevice;

import edu.stanford.sti.*;


public class TestJDevice extends JLocalDevice {
    
    private JEngineJobUpdateDeviceMessageListener joblistener;
    private EngineStateMessageListener statelistener;


    public TestJDevice(String name, String address, int module, String targetServer) {
        super(name, address, module, targetServer);
    
        edu.stanford.sti.DeviceID serverID = new edu.stanford.sti.DeviceID("STI Server", "localhost", 0);
        addPartner(serverID);
        
        addChannel(1, ChannelType.Output, MixedValueType.Empty, MixedValueType.Double, "testch"); //.addMetaData(key, value);
        addChannel(2, ChannelType.Input, MixedValueType.Double, MixedValueType.Double, "testch2");

        addAttribute("testat", "5")
        .setRefresher( new AttributeRefresher() {
            public String refresh() {
                System.out.println("*** refresh: " + getID().getID());
                return "75";
            }
        })
        .setSetter(new AttributeSetter() {
            public boolean set(String value) {
                System.out.println("*** set: " + value);
                return true;
            }
        })
        .addMetaData("color", "red");
        
        // String[] vs= {"True", "False"};
        // addAttribute("at2", "True", new StringVector(vs));
        // addAttribute("at2", "True", new StringVector(new String[] {"True", "False"}));
        addAttribute("at2", "True", "True,False");

        addCollectionListener(new DeviceCollectionListener() {
            public void add(DeviceID id) {
                System.out.println("Collection listener add: " + id.getID());
            }
        });


        JEventEngineScheduler engineScheduler = getEngineScheduler();      

        JDeviceMessageReceiver messageReceiver = this.getMessageReceiver();

        //edu.stanford.sti.DeviceID sourceDeviceID = new DeviceID("dev2", "localhost", 0);
        edu.stanford.sti.DeviceMessageListenerID listenerID = new DeviceMessageListenerID();
        listenerID.setName("Test JobUpdate");
        listenerID.setType(DeviceMessageType.EngineJobUpdate);
        
        joblistener = new edu.stanford.sti.JEngineJobUpdateDeviceMessageListener() {
            public void handleMessage(edu.stanford.sti.JEngineJobUpdateDeviceMessage mess) {
                // System.out.println("Job:" + mess.getJEngineJob().getJobID().getPid().getParseTimestamp().getTimestamp());
                System.out.println("Job: " + mess.getTargetList().toString() + " : " + mess.getDeviceTrace().print());
                System.out.println("Job message: " + mess.getJEngineJob().getParsingMessages().toString());
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

    // public boolean writeChannel(int channel, MixedValue value)
    // {
    //     System.out.println("java writeChannel " + channel);
    //     return true;
    // }

    public MixedValue readChannel(int channel, MixedValue value)
    {
        System.out.println("java readChannel " + channel);
        MixedValue data = new MixedValue();
        data.setValue(104.8);
        return data;
    }

    public class TestJEvent extends SynchronousEventAdapter
    {
        public TestJEvent(double time)
        {
            super(time);
        }

        public void loadEvent()
        {
            System.out.println("--> java loadEvent " + getTime());
        }
        public void playEvent()
        {
            System.out.println("--> java playEvent ");
        }
    }

    public void parseEvents(RawEventMap events, SynchronousEventVector synchedEvents)
    {
        System.out.println("java parseEvents " + events.entrySet().iterator().next().getKey());
       

        synchedEvents.add( new TestJEvent(34.6) );
    }

}
