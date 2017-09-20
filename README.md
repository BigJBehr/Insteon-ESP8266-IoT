Insteon ESP8266 Lighting Controller

# Insteon ESP8266 IoT

## Why I did this:
Home automation is a hot topic these days. It covers everything from window and door sensors to WiFi enabled thermostats to remote control of lights and appliances. I use Insteon devices to control some of the lights in my house and a ceiling fan/light in my bedroom. I originally chose Insteon because it is backward compatible with my older X10 devices. With the inclusion of an Insteon Hub, you can install the Insteon app on your Android or IOS device and control your Insteon devices with your phone or tablet. Insteon is also compatible with the Amazon Echo Dot, a.k.a Alexa. With Insteon and Alexa you can issue voice commands to control your Insteon devices. Very cool. So why would you need yet another way to control your Insteon devices ? The reasons are that Alexa is expensive and does not always work! Your local Amazon Echo recognizes the key phrase “Alexa” and then uses your Internet connection to forward your voice command to an Amazon server for processing. The reply is a digital command sequence that your Alexa forwards to the Insteon Hub. The Insteon Hub then performs the magic. Sometimes Alexa tells me that my Insteon Hub cannot be found or that she is having trouble connecting to the Amazon servers. The end result is that my voice commands do not work. So I have to find my phone or a tablet, wait for it to boot up and then issue the command using the Insteon app. This can be a hassle if you just got into bed and want to turn off the light. You have to get back up and locate an Android device. Ugh.

It would be so much nicer if you had a simple WiFi device with switches that could be programmed to turn on/off your Insteon devices on your bedside table. It would also be great if said device did not rely on having to send messages over the Internet to remote servers to send commands to your local Insteon Hub. This is exactly what I have designed. A simple, low cost, ESP8266 powered IoT device that sends commands to your Insteon Hub, using your local WiFi and wired network. Internet is not required and no outside servers are used.
I designed my Insteon Switch Box to have up to eight pushbutton switches. Each switch can be programmed to turn on or off one Insteon device or scene. A switch may also be programmed to read the state of an Insteon device and then issue a command to change the state. The result is one switch that with one press turns on an Insteon device and a second press turns off the same device. I use an Insteon Fanlinc device to control a ceiling fan in my bedroom. A Fanlinc has two sections. One section controls the light and the other controls the fan. The fan can have three speeds and the light is dimmable. Alexa cannot work directly with an Insteon Fanlinc. In order to use a Fanlinc with Alexa you must create scenes. One scene for the light and one scene for each of the fan speeds. Because scenes are either on or off, Alexa cannot dim the Fanlinc’s light. My Insteon Switch box can send commands to the Insteon Hub to turn on or off or dim the Fanlinc’s light and to control the fan without the need for scenes. This is an improvement over what Alexa can do. My Insteon Switch box was not designed to control a thermostat or listen for door or window sensors. However, with Insteon On/Off plug-in modules that use a relay to turn on/off the plugged in object, it is possible to control drapes or a coffee maker, toaster or electric kettle. You merely have to press a button when you get out of bed. When you exit the shower, your coffee will be ready. All without waking up your mate by talking to Alexa.

The remainder of this document will detail how to build one of these for yourself and how to configure the software so it will work with your Insteon devices. There is also some small lessons in writing ‘C’ code. 

## What You will Need:
-	Any ESP8266-12 module or board with an ESP8266-12 module on it. 
-	USB-to-Serial bridge with RTS and DTR unless the ESP8266 board has it built-in.
-	5V, 2A power supply.
-	Power Jack that mates with the power supply.
-	Eight 10K resisters
-	Eight Pushbutton Switches
-	Custom Circuit Board or hand-wired circuit board
-	See individual Bills of Materials for complete parts list.

### Software apps and online services:
-	Arduino IDE, version 1.8.0 or higher
-	ESP8266 Plug-in for Arduino
-	Optional, OpenSCAD

### Hand tools and fabrication machines:
-	Soldering iron
-	Solder, preferably with no clean flux
-	Diagonal cutters
-	Screwdriver
-	Optional, 3D printer

