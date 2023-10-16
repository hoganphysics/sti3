.. _devicelib:

==============
Device library
==============


Setup
-----

* Installation
* Building from source


Device interface
----------------

* Device collection
* Channels
* Attributes
* Event engines
* Device messages
* Data persistence


Device collection
*****************

Channels
********

.. tabs::

   .. code-tab:: c++
    
	    void getChannelManager(shared_ptr<ChannelManager>& manager);

   .. code-tab:: py
        
        getChannelManager() -> ChannelManager

   .. code-tab:: java

        public JChannelManager getChannelManager();






Attributes
**********


.. tabs::

   .. code-tab:: c++
    
	    void getAttributeManager(shared_ptr<AttributeManager>& manager);

   .. code-tab:: py
        
        getAttributeManager() -> AttributeManager

   .. code-tab:: java

        public JAttributeManager getAttributeManager();


Event engine
************

Persistence
***********

.. tabs::

   .. code-tab:: c++
    
	    bool getPersistenceManager(shared_ptr<PersistenceManager>& manager);

   .. code-tab:: py
        
        getPersistenceManager() -> PersistenceManager

   .. code-tab:: java

        public JPersistenceManager getPersistenceManager();




.. tabs::

   .. code-tab:: c++

        shared_ptr<PersistenceManager> persistenceManager;
        getPersistenceManager(persistenceManager);
        
        ParseID pid;  //make valid ParseID somehow...

        if (persistenceManager->getParseResult(pid, parsePesult)) {
            //parsePesult found
        }

   .. code-tab:: py
        
        ParseID pid;  #make valid ParseID somehow...
        parsePesult = getPersistenceManager().getParseResult(pid);

   .. code-tab:: java

        ParseID pid;  //make valid ParseID somehow...
        ParseResult parsePesult = getPersistenceManager().getParseResult(pid);






.. tabs::

   .. code-tab:: c++
    
        bool getParseResult(const ParseID& pid, shared_ptr<ParseResult>& parseResult);

   .. code-tab:: py
        
        getParseResult(pid: ParseID) -> ParseResult

   .. code-tab:: java

        public ParseResult getParseResult(ParseID pid);



.. tabs::

   .. code-tab:: c++
    
        bool getShotResult(const ShotID& sid, shared_ptr<ShotResult>& result);

   .. code-tab:: py
        
        getShotResult(sid: ShotID) -> ShotResult

   .. code-tab:: java

        public ShotResult getShotResult(ShotID sid);




Creating a device
-----------------

Each piece of hardware in the STI network is controlled by dedicated (stand-alone) device drivers.
To create an STI device, you implement an extension of the LocalDevice class.  LocalDevice is part of
the STI device library, and contains all funtionality needed to connect to the STI network and 
receive events for timing sequences.

The LocalDevice class may be used to create a new device in a variety of programming languages.
In all cases, you create a derived class using LocalDevice as a base class. All custom behavior of 
your device is then implemented by overriding function hooks provided by LocalDevice.

.. Note::
    Several :ref:`examples <deviceexamples>` of device drivers may be found in the *examples/* subdirectory in the 
    STI source distribution.
    

Creating a simple device can be done in a few lines of code.  In the example below, a minimal STI 
device called **SimpleDevice** is defined:

.. tabs::

   .. code-tab:: c++

        #include <sti/LocalDevice.h>

        using STI::Device::LocalDevice;

        class SimpleDevice : public LocalDevice
        {
        public:

            SimpleDevice(const string& name, const string& address, unsigned short module, const string& targetServer) 
            : LocalDevice(name, address, module, targetServer) 
            {
            }
        };


   .. code-tab:: py
        
        from stipy.stidevicepy import *

        class SimpleDevice(LocalDevice):
            def __init__(self, name, address, module, targetServer):
                LocalDevice.__init__(self, name, address, module, targetServer)


   .. code-tab:: java

        import edu.stanford.sti.JLocalDevice;

        public class SimpleDevice extends JLocalDevice {
            
            public SimpleDevice(String name, String address, int module, String targetServer) {
                super(name, address, module, targetServer);
            }
        }



Here the arguments of the *LocalDevice* constructor accept the *name*, *address*, *module*, and 
*targetServer* of the device.  This information defines the device's 
:ref:`DeviceID <devicenetworkDeviceID>` and :ref:`Target Server <devicenetworkTargetServer>`.
Alternatively, this information can also be provided using configuration data which may be 
defined in separate config file.

Devices must connect to the STI network by attaching to a Hub.  To enable communcation over 
TCP/IP, STI includes the NetworkDeviceHub class.  Each stand-alone executable must 
contain an instance of NetworkDeviceHub, to which multiple device instances may be attached.
After creating a NetworkDeviceHub, devices can be added using the **addDevice** function.
Once all device have been attached, the NetworkDeviceHub can be activated using the **run** command.
Calling **run** tells the hub to connect to the STI network and allows all attached devices to 
connect to their respective target servers.

Here is a minimal example of creating a NetworkDeviceHub 
and attaching an instance of the SimpleDevice defined above:

.. tabs::

   .. code-tab:: c++

        #include <sti/sti.h>
        #include <memory>

        int main(int argc, char **argv)
        {
            auto device = std::make_shared<SimpleDevice>("Simple Device", "localhost", 0, "localhost/0/STI Server");

            auto hub = std::make_shared<NetworkDeviceHub>("192.168.1.4:2809");

            hub->addDevice(device);
            hub->run();

            return 0;
        }



   .. code-tab:: py
        
        device = SimpleDevice("Simple Device", "localhost", 0, "localhost/0/STI Server")

        hub = NetworkDeviceHub("192.168.1.4:2809")

        hub.addDevice(device)
        hub.run()

   .. code-tab:: java

        import edu.stanford.sti.STIJava;
        import edu.stanford.sti.JNetworkDeviceHub;

        public class SimpleDeviceApplication {

            public static void main(String[] args) {
                
                STIJava.LoadLibrary();

                SimpleDevice simpleDevice = new SimpleDevice("Simple Device", "localhost", 0, "localhost/0/STI Server");    
                
                JNetworkDeviceHub hub = new JNetworkDeviceHub("192.168.1.4:2809");

                hub.addNode(simpleDevice);
                hub.run();
            }
        }


.. Note::
    To create the NetworkDeviceHub, you must specify the IP address and port number of the 
    Name Service of the STI network.  Unlike the address field in a device's DeviceID, this 
    IP address must point to a valid computer on the local network.
    
    The Name Service is a stand-alone service that 
    provides object references required for the remote procedure calls over TCP/IP 
    used by the STI network.

By default, the *run* command is blocking.  The NetworkDeviceHub activates in a separate 
thread and begins communicating over the network.  To prevent premature termination of the 
program, the main thread is blocked by *run* until 
all connected devices are killed. To override this behavior, *run* accepts a boolean value 
that may be set to false to not block (default is true).  Calling *run(false)* activates the hub 
and returns control to the main thread.

The above example is the minimum working device that can connect to the network, but it doesn't 
have any additional functionality. In a realistic example, writing a device driver may also 
require some or all of the following:

* Defining the device's input and output channels
* Implementing device-specific event parsing
* Defining device attributes (key value pairs) to customize behavior
* Defining partner devices and event targets
* Installing custom message listeners

Each of these features are described in the following sections.

Defining channels
*****************



.. tabs::

   .. code-tab:: c++

        LocalChannel& addChannel(unsigned short channelNumber, ChannelType type,
                                 MixedValueType inputType, MixedValueType outputType, 
                                 const string& defaultName);

   .. code-tab:: py
        
        addChannel(channelNumber: int, type: ChannelType, inputType: MixedValueType, \
                   outputType: MixedValueType, defaultName: str) -> LocalChannel


   .. code-tab:: java

        public LocalChannel addChannel(int channelNumber, ChannelType type, 
                                       MixedValueType inputType, MixedValueType outputType, 
                                       String defaultName);



.. tabs::

   .. code-tab:: c++

        //in device constructor

        //Output channel
        addChannel(1, ChannelType::Output, MixedValueType::Empty, MixedValueType::Double, "name1");

        //Input channel
        addChannel(2, ChannelType::Input, MixedValueType::Double, MixedValueType::String, "name2");
        addChannel(3, ChannelType::Input, MixedValueType::String, MixedValueType::Vector, "name3")
            .addMetaData("key", "value");

   .. code-tab:: py

        #in device constructor

        //Output channel
        self.addChannel(1, ChannelType.Output, MixedValueType.Empty, MixedValueType.Double, "name1")
        
        //Input channel
        self.addChannel(2, ChannelType.Input, MixedValueType.Double, MixedValueType.String, "name2")
        self.addChannel(3, ChannelType.Input, MixedValueType.String, MixedValueType.Vector, "name3")
            .addMetaData("key", "value");
     
   .. code-tab:: java

        //in device constructor

        //Output channel
        addChannel(1, ChannelType.Output, MixedValueType.Empty, MixedValueType.Double, "name1");

        //Input channel
        addChannel(2, ChannelType.Input, MixedValueType.Double, MixedValueType.String, "name2");
        addChannel(3, ChannelType.Input, MixedValueType.String, MixedValueType.Vector, "name3")
            .addMetaData("key", "value");



Event parsing
*************


Adding attributes
*****************


.. tabs::

   .. code-tab:: c++

        LocalAttribute& addAttribute(const string& key, const string& initialValue);
        LocalAttribute& addAttribute(const string& key, const string& initialValue, 
                                     vector<string> allowedValues);

   .. code-tab:: py
        
        addAttribute(key: str, initialValue: str, allowedValues: str) -> LocalAttribute


   .. code-tab:: java

        public LocalAttribute addAttribute(String key, String initialValue);
        public LocalAttribute addAttribute(String key, String initialValue, StringVector allowedValues);


.. tabs::

   .. tab:: C++
    
        Add callback functions for set/refresh

   .. tab:: Python
        
        Add callback functions for set/refresh

   .. tab:: Java

        Define refresher/setter classes




.. tabs::

   .. code-tab:: c++

        class TestDevice : public STI::Device::LocalDevice
        {
            //...

            bool setTemperature(const std::string& value);
            std::string getTemperature();
        };

        //in TestDevice constructor:
        addAttribute("temperature", 57)                     //initial value is 57
            .setSetter(&TestDevice::setTemperature, this)
            .setRefresher(&TestDevice::getTemperature, this);

   .. code-tab:: py

        class TestDevice(stidevicepy.LocalDevice):
            def __init__(self, ...):
                #...

                self.addAttribute("temperature", "57")      #initial value is "57"
                    .setSetter(self.setTemperature)
                    .setRefresher(self.getTemperature)
        
            def setTemperature(self, value):
                #...
            def getTemperature(self):
                #...

   .. code-tab:: java

        //in device constructor:
        addAttribute("temperature", "57")                   //initial value is 57
            .setRefresher( new AttributeRefresher() {
                public String refresh() {
                    String result;
                    //...
                    return result;
                }
            })
            .setSetter(new AttributeSetter() {
                public boolean set(String value) {
                    //...
                    return true;
                }
            });


Attributes can also have meta data.  Meta data consists of key-value pairs 
of strings and may be used to describe things like formating instructions, units, 
user interface details, etc.  One or more meta data entries may be added to an 
attribute by chaining the *addMetaData* command during attribute construction.

.. tabs::

   .. code-tab:: c++

        //in constructor
        addAttribute(...)
            .setSetter(...)
            .setRefresher(...)
            .addMetaData(key, value);

   .. code-tab:: py

        #in constructor
        self.addAttribute(...)
            .setSetter(...)
            .setRefresher(...)
            .addMetaData(key, value);

   .. code-tab:: java

        //in constructor
        addAttribute(...)
            .setRefresher(...)
            .setSetter(...)
            .addMetaData(key, value);


Adding partner devices
**********************


Message listeners
*****************


Device Collection
-----------------

.. tabs::

   .. code-tab:: c++

         C++ Main Function

   .. code-tab:: py

         Python Main Function

   .. code-tab:: java

        //Create and add a custom DeviceCollectionListener by implementing
        //the following methods (as needed)

        addCollectionListener(new DeviceCollectionListener() {
            public void add(DeviceID id) {
                //...
            }
            public void remove(DeviceID id) {
                //...
            }
            public void refresh() {
                //...
            }
        });


Event Engine
------------

RawEvent
SynchronousEvent



.. tabs::

   .. code-tab:: c++
    
	    void parseEvents(const RawEventMap& events, SynchronousEventVector& synchedEvents)

   .. code-tab:: py
        
        parseEvents(events: list, synchedEvents: list) -> None

   .. code-tab:: java

        public void parseEvents(RawEventMap events, SynchronousEventVector synchedEvents);




.. tabs::

   .. code-tab:: c++

        class SynchronousEvent
        {
        public:
            //...
            virtual void loadEvent() = 0;
            virtual void playEvent() = 0;
            virtual void collectMeasurementData() = 0;
            virtual void stopEvent() = 0;
            virtual void pauseEvent() = 0;
            virtual void unpauseEvent(bool retrigger) = 0;
            //...
        };

   .. code-tab:: py

        class SynchronousEvent(SynchronousEventBase)
            #...
            loadEvent() -> None
            playEvent() -> None
            collectMeasurementData() -> None
            stopEvent() -> None
            pauseEvent() -> None
            unpauseEvent(retrigger: bool) -> None
        
   .. code-tab:: java

        public class SynchronousEventAdapter extends SynchronousEvent {
            //...
            public void loadEvent();
            public void playEvent();
            public void collectMeasurementData();
            public void stopEvent();
            public void pauseEvent();
            public void unpauseEvent(boolean retrigger);
        }


Device Messages
---------------

