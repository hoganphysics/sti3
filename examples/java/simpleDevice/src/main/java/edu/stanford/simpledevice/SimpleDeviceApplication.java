package edu.stanford.simpledevice;

import edu.stanford.sti.STIJava;
import edu.stanford.sti.JNetworkDeviceHub;


public class SimpleDeviceApplication {

    public static void main(String[] args) {
        
        STIJava.LoadLibrary();

        SimpleDevice simpleDevice = new SimpleDevice("Simple Device", "localhost", 2, "localhost/0/STI Server");    
        
        JNetworkDeviceHub hub = new JNetworkDeviceHub("192.168.1.4:2809");

        hub.addNode(simpleDevice);
        hub.run();
    }
}
