# Assignment
For this assignment, we had to build a project that implemented [PID control](https://en.wikipedia.org/wiki/PID_controller) in some way. People in the engineering class made groups, and our group was Vinnie Jones [@vjones2906](https://github.com/vjones2906) and River Lewis [@rivques](https://github.com/rivques). We chose to make a car that would use a PID loop to keep a certain distance between it and whatever was in front of it. To spice it up, we decided to make the car tuneable wirelessly.
# Parts
- 2 Adafruit M4 Metro Express Airlifts
- 2 prototyping shields
- 1 .96" 128x64 OLED
- 1 HC-SR04 ultrasonic distance sensor
- 1 SPDT panel-mount toggle switch
- 1 DRV8833 motor driver
- 2 TT motors
- 2 wheels
- 1 caster
- 1 LSM303DLHC magnetometer/accelerometer*
- 1 6xAA batter pack with an extra tap at 6V
- 6 AA batteries
- 3D printed and laser-cut parts as called for by CAD
- Assorted wires
- Assorted hardware
# Media
actual pictures and videos go here
# CAD
For the final design of this project we created a two-story box. On the lower level the batteries were stored, while the top held both Metro Expresses. The bottom of the design included a caster ball and its extension and 2 different types of braces to hold 2 TT motors. Holes were made to mount the ultrasonic sensor on the front, and there were holes for a power switch and OLED on the back of the car. On the side there was a "mast" which was a vertical piece of acrylic that was meant to keep the magnetometer away from the magnets of the motors providing for more of a precise reading.

**Note: Depending on how you're viewing this document, these pictures may have captions or you may need to hover over each image to read the caption.**

![Wide view of car](/docs/WTPIDC/cad1.png "Wide view of car")
![Close up of back mounts](/docs/WTPIDC/cad2.png "Close up of back mounts")
![Close up of TT motor braces](/docs/WTPIDC/cad3.png "Close up of TT motor braces")
![Side view of mast and caster/extension](/docs/WTPIDC/cad4.png "Side view of mast and caster/extension")
# Circuit
For reasons we will get into in the next section, there are two Metros communicating with each other over UART. One Metro is connected to an OLED, while the other is connected to the distance sensor, the motors, and the magnetometer. A power switch switches the ground. Normally it is bad practice to switch the ground, and ideally we would have had a double pole switch to switch both voltage lines instead, but it doesn't cause any problems here. 
![A screenshot of a Fritzing diagram of the circuit.](/docs/WTPIDC/circuit.png "The circuit.")
# Code
The code is split into three parts: The car controller, which is in CircuitPython and runs the PID loops, the WebSocket coprocessor(https://github.com/rivques/wireless-tuneable-pid-car/tree/main/ws-coproc), which is in C++ and handles the communication between the car and the computer, and the web client, which is in HTML/CSS/JS and connects to the car and allows for wireless PID tuning.
## Car Controller
Source code available [here.](https://github.com/rivques/wireless-tuneable-pid-car/blob/main/car-controller.py)

This runs on one of the Metros. It controls the motors, reads the distance sensor, and runs the PID loops. It sends its current PID state to the WS coprocessor over UART to be sent to the web client and occasionally receives new gains back. It utilizes [Copper280z's port to CircuitPython](https://github.com/Copper280z/CircuitPython_simple-pid) of [m-lundberg's `simple-pid` for Python](https://github.com/m-lundberg/simple-pid) based on [Brett Beauregard's PID library for Arduino](https://playground.arduino.cc/Code/PIDLibrary/).
## WebSocket Coprocessor
Source code available [here.](https://github.com/rivques/wireless-tuneable-pid-car/tree/main/ws-coproc) Please note that this program expects to have a file called `arduino_secrets.h` in this folder, which defines `const char* ssid = "YOUR_SSID_HERE"` and `const char* password = "YOUR_PASSWORD_HERE"`. 

This runs on the other Metro. 
## Web Client
Hosted [here](https://rivques.dev/random-raw-files/wtpidc.html?ref=docsdotdev), source code available here: [HTML](https://github.com/rivques/wireless-tuneable-pid-car/blob/main/index.html)/[CSS](https://github.com/rivques/wireless-tuneable-pid-car/blob/main/index.css)/[JS](https://github.com/rivques/wireless-tuneable-pid-car/blob/main/index.js).

This is a Web site which is capable of connecting to the car over WebSockets.
# Fabrication
Our planning document is located [here.](https://github.com/rivques/wireless-tuneable-pid-car/blob/main/PLANNING.md)
The fabrication of V1 went very smoothly... until we tried to install the OLED. All the bolts went in smoothly and all the parts fit as planned, but the space needed for the ends of the wires to connect to the OLED was overlooked and it was impossible the wire the OLED without bending the wires and compromising the connection. The fabrication of V2 came with a few problems though. The allowance of the laser joint was too little and the parts would not fit together. We messed with the allowance and re-cut a few of the parts. At that point they were so close to fitting so we filed the tabs down a tad and pushed them together with a lot of force. All the attachments that needed to be screwed to the bottom were assembled first, then all the attachments to the second layer. The box was then assembled by putting the sides and second floor in simultaneously. Finally, all the side attachments were fabricated. Later on we decided that we needed the mast so we laser-cut it and installed it. 
# Reflections
## Code-CAD split
For this project we chose to have a very hard code-CAD split: Vince would do all of the CAD and River would do all of the code. This ended up working fairly well because both sections were finished within a couple days of each other, so one person didn't have to wait on the other too much. 
## Code
### Host elsewhere and connect
### Memory issues! (This time in C++! *even more fun!*)
Unlike [last time](https://rivques.dev/high-school-engineering/auto-music-box-hole-puncher/#memory-issues-in-python-what-fun), this memory issue was in the C++ code.
## CAD
### V1
The first version of the car had no second layer which squished the battery pack and both Metro Expresses so tight that there was no room for the OLED wires to connect. The braces for the TT motors were not strong enough and posed a threat of breaking. It was also a pain if we had to change any of the wires on the bottom Metro Express because the other one was bolted directly on top.
### V2
The second version had everything we needed, but it would have been nice to have added a few extra features. Two holes to bolt in the ultrasonic sensor could have kept it from falling out. If I had figured out the exact size of the different bolts before making the holes so some of them didn't go through the acrylic or 3D printed parts. There are also a couple of parts that have virtually no allowance and made our hearts skip a beat when we tried to install them for the fist time (luckily they all went in). 

## The magnetometer :(
