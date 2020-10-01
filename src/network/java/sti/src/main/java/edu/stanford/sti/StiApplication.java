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

		LocalDevice dev = new LocalDevice("Test Dev", "localhost", 0, "root");
		//STI_Collection collection = new STI_Collection();
		//DeviceCollection collection = new DeviceCollection();
		STI_Collection collection = null;

		dev.getCollection(collection);
		collection.add(dev.getId(), dev);

		System.out.println(collection.size());
		System.out.println(dev.getId().getID());
		
	}

}
