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
	}

}
