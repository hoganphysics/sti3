.. _devicelib:

================
Device Interface
================

Channels
--------

Attributes
----------


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

         Java Main Function


Device Collection
-----------------

.. tabs::

   .. code-tab:: c++

         C++ Main Function

   .. code-tab:: py

         Python Main Function

   .. code-tab:: java

         Java Main Function

Event Engine
------------

Device Messages
---------------

Persistence
-----------


