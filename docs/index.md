# Assignment
For this assignment, we had to build a project that implemented [PID control](https://en.wikipedia.org/wiki/PID_controller) in some way. 
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

![Wide view of car](/docs/WTPIDC/cad1.png)
![Close up of back mounts](/docs/WIPIDC/cad2.png)
![Close up of TT motor braces](/docs/WIPIDC/cad3.png)
![Side view of mast and caster/extension](/docs/WIPIDC/cad4.png)
# Circuit
For reasons we will get into in the next section, there are two Metros communicating with each other over UART. One Metro is connected to an OLED, while the other is connected to the distance sensor, the motors, and the magnetometer. A power switch switches the ground. Normally it is bad practice to switch the ground, and ideally we would have had a double pole switch to switch both voltage lines instead, but it doesn't cause any problems here. 
![A screenshot of a Fritzing diagram of the circuit.](/docs/WTPIDC/circuit.png "The circuit.")
# Code
The code is split into three parts: The car controller, which is in CircuitPython and runs the PID loops, the WebSocket coprocessor(https://github.com/rivques/wireless-tuneable-pid-car/tree/main/ws-coproc), which is in C++ and handles the communication between the car and the computer, and the web client, which is in [HTML](https://github.com/rivques/wireless-tuneable-pid-car/blob/main/index.html)/[CSS](https://github.com/rivques/wireless-tuneable-pid-car/blob/main/index.css)/[JS](https://github.com/rivques/wireless-tuneable-pid-car/blob/main/index.js) and connects to the car and allows for wireless PID tuning.
## Car Controller
Source code available [here.](https://github.com/rivques/wireless-tuneable-pid-car/blob/main/car-controller.py)

This runs on one of the Metros. It controls the 
## WebSocket Coprocessor
Source code available [here.](https://github.com/rivques/wireless-tuneable-pid-car/tree/main/ws-coproc) Please note that this program expects to have a file called `arduino_secrets.h` in this folder, which defines `const char* ssid = "YOUR_SSID_HERE"` and `const char* password = "YOUR_PASSWORD_HERE"`.

This runs on the other Metro. 
## Web Client
# Fabrication
Our planning document is located [here.](https://github.com/rivques/wireless-tuneable-pid-car/blob/main/PLANNING.md)
The fabrication of V1 went very smoothly... until we tried to install the OLED. There was
# Reflections
## Code-CAD split
## Code
## CAD
 The first version of the car had no second layer which squished the battery pack and both Metro Expresses so tight that there was no room for the OLED wires to connect. The braces for the TT motors were not strong enough and posed a threat of breaking.