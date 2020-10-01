package edu.stanford.sti;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

//import edu.stanford.sti.*;

@SpringBootApplication
public class StiApplication {

	static {
        System.loadLibrary("sti");
    }

	public static void main(String[] args) {
		//SpringApplication.run(StiApplication.class, args);
		
		DeviceID devID = new DeviceID("Test Dev", "localhost", 0, "root");

		System.out.println(devID.getID());

		JDevice dev = new JDevice("Test Dev", "localhost", 0, "root");
		JDevice dev2 = new JDevice("Dev 2", "localhost", 2, "root");
		//STI_Collection collection = new STI_Collection();
		//DeviceCollection collection = new DeviceCollection();
		//STI_Collection collection = null;

		//dev.getCollection(collection);
		//collection.add(dev.getId(), dev);

		//System.out.println(collection.size());
		System.out.println(dev.getID().getID());

		dev.getCollection().add(dev2.getID(), dev2);

		System.out.println("From collection" + dev.getCollection().get(dev2.getID()).getID().getID());

		dev.getCollection().get(dev2.getID()).write(23);
		
	}

}
