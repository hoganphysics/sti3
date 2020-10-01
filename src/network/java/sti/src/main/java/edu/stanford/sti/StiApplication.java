package edu.stanford.sti;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

//import edu.stanford.sti.*;

@SpringBootApplication
public class StiApplication {

	static {
        System.loadLibrary("sti");
    }

	public static class MyJDevice extends JDevice {
		MyJDevice(String name, String address, int module, String targetServer)
		{
			super(name, address, module, targetServer);
		}

		public void write(long input)
		{
			System.out.println("Overridden write: " + input);
		}
	}

	public static void main(String[] args) {
		//SpringApplication.run(StiApplication.class, args);
		
		DeviceID devID = new DeviceID("Test Dev", "localhost", 0, "root");

		System.out.println(devID.getID());

		JDevice dev = new JDevice("Java Dev", "localhost", 0, "localhost/0/dev0");
		MyJDevice dev2 = new MyJDevice("Java Dev 2", "localhost", 2, "localhost/0/dev0");
		//STI_Collection collection = new STI_Collection();
		//DeviceCollection collection = new DeviceCollection();
		//STI_Collection collection = null;

		//dev.getCollection(collection);
		//collection.add(dev.getId(), dev);

		//System.out.println(collection.size());
		System.out.println(dev.getID().getID());

		//dev.getCollection().add(dev2.getID(), dev2);
		//System.out.println("From collection" + dev.getCollection().get(dev2.getID()).getID().getID());
		//dev.getCollection().get(dev2.getID()).write(23);

		JNetworkDeviceHub hub = new JNetworkDeviceHub("192.168.1.6:2809");
		
		hub.addNode(dev.getID(), dev);
		hub.addNode(dev2.getID(), dev2);



		hub.run(false);

		printNetwork(hub.walk());
	}

	public static void printNetwork(JNodeWalker network)
	{
		printHub(network.getNode());
		
		for (JNodeWalker walker : network.getConnections()) {
			printNetwork(walker);
		}
	}

	public static void printHub(JHubGraphNode hub)
	{
		System.out.println("* " + hub.getHubID().id());

		for(JDeviceGraphNode dev : hub.getNodes()) {
			printDevice(dev);
		}
	}
	public static void printDevice(JDeviceGraphNode dev)
	{
		System.out.println("** " + dev.getID().getID());

		for(DeviceID id : dev.getOutConnections()) {
			System.out.println("*** " + id.getID());
		}
	}

}
