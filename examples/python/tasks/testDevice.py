import stipy
import stipy.stidevicepy as stidevicepy

from datetime import datetime, timedelta

import time


# Optional: Define a custom Task class
# This is only necessary if the built-in Task types are not sufficient.
class CustomTask(stipy.Task):
    def __init__(self, id: str):
        stipy.Task.__init__(self, id)
        self.target_time = time.time() + 3
    def secondsToNextRun(self):
        wait_time = self.target_time - time.time()
        return wait_time
    def run(self):
        print("***** custom task *****")
        self.target_time = time.time() + 3
    def skipTask(self):
        return
    def repeat(self):
        return True



class TestDevice(stidevicepy.LocalDevice):
    def __init__(self, config):
        stidevicepy.LocalDevice.__init__(self, config)

        # *** Define channels *** #
        self.addOutputChannel(0, stipy.MixedValueType.Double, "coil current")       #outputs a Double
        self.addOutputChannel(1, stipy.MixedValueType.Number, "supply voltage")     #outputs a number
        self.addInputChannel(11, stipy.MixedValueType.Number, "magnetic field")     #measures a number (input)

        # *** Define attributes *** #
        self.addAttribute("x", "22")
        self.addAttribute("TriggerSource", "Hardware", ["Hardware", "Software"])


        # *** Task examples *** #

        def task_function():
            result = self.read(11)  # do something...
            print("Running IntervalTask: ", result)

        # Add a task that runs 'task_function' every 2 seconds
        task1 = stipy.IntervalTask("task#1", "00:00:02", task_function)
        # task1 = stipy.IntervalTask("task#1", 2, task_function)
        self.addTask(task1)


        def task_function_2():
            print(">>>>>> AppointmentTask!")
            self.write(1, 15)   # do something...

        target_time = datetime.now() + timedelta(seconds=5)

        # Add a task that runs once, five seconds after the program starts, and repeats everday at the same time
        task2 = stipy.AppointmentTask("task#2", target_time.strftime("%H:%M:%S"), stipy.AppointmentRepeatType.Everyday, task_function_2)
        self.addTask(task2)

        # Add a custom Task type
        task3 = CustomTask("task#3")
        self.addTask(task3)

        return


    def writeChannel(self, channel, value):
        success = False

        if channel == 0:
            #coil current (Double)
            print("Ch:" + str(channel) + ", " + "coil current: " + str(value))
            success = True
        elif channel == 1:
            #temperature setpoint (Int)
            print("Ch:" + str(channel) + ", " + "temperature setpoint: " + str(value))
            success = True
        return success

    def readChannel(self, channel, value):
        if channel == 11:
            #No arguments
            return 23.4
        

