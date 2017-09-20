// Project:   ESP8266 IoT Contest Entry
// Author:    James R. Behrens
// Compiler:  Arduino IDE Ver 1.8.0 with ESP8266 plug-in

// ESP8266 Contest entry. Uses an ESP8266 to send commands to an Insteon Hub to control Insteon
// devices and/or scenes. Does not use MQTT to access an outside server to send commans to your
// Insteon hub. All communications is done using your local network. An Internet connection
// is not required.

// HTTP Client for ESP8266 to control Insteon Devices
// This should work on any ESP8266 based board.

// This has been tested and works with the following Insteon devices
// Fanlinc                          - 2475F    - fan and light controller, fan uses extended format commands
// On/Off Module                    - 2635-232 - relay module that can control lights and motors (plug in)
// SwitchLinc Remote Control Dimmer - 2477D    - works with CFL, incandesant and LED lights (in wall)
// LED Light Bulb                   - 2672-222 - socket must be always on
// Insteon Hub                      - 2245-222 - Alexa compatible, Insteon & X10 protocols

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <Arduino.h>

// uncomment this to enable debug messages
//#define DEBUG

// ESP8266 module on Insteon Switch board
// In Arduino Board Manager, Select the following;
// Arduino Board: "WeMos D1 R2 & mini"
// CPU Frequency: "80MHz"
// Flash Size:    "4M(3M SPIFFS)"
// Upload Speed:  "921600"


// ***** Change these for your WiFi SSID and passkey *****
#define SSID    "yourssid"
#define PASSKEY "yourpasskey"

// Change these for the User name, Password, IP address and HTTP Port of your Insteon Hub
#define HUBUSER       "user"
#define HUBPASSWORD   "password"
#define HUBIP         "192.168.1.100"
#define HTTPPORT      "25105"       // default

// Insteon IDs of all of my devices I want to control. The ID is in Hex ASCII.
// These should be replaced with the IDs and names of your devicess. The Insteon
// ID is printed on a label on the device's case or can be obtained from the
// Insteon app on your phone/tablet.
#define SmartBulb   "2F39E3"    // LED smart bulb - dimmable
#define Kitchen     "339EFC"    // Kitchen        - wall switch dimmer
#define Torch       "3608AB"    // Torch          - plug-in on/off module
#define Xmas        "2837FF"    // Xmas lights    - plug-in on/off module
#define Spare1      "360773"    // Spare          - plug-in on/off module
#define Bedroom     "3F8E35"    // Bedroom Fanlinc module - light and fan  

// Insteon Group numbers (scenes) for my Bedroom FanLinc. The scenes are needed for
// my Amazon Echo Dot to control the Fanlinc device.
// Scenes can be either on or off.
// ***** These should be replaced with the scene/group numbers of your scenes/groups *****
#define Light       "10"        // Fanlinc light
#define Fan1        "07"        // low speed
#define Fan2        "08"        // meduim speed
#define Fan3        "09"        // full speed

// Insteon commands
// for a device the on cmd includes the brightness, "11FF" -> full bright, "117F" -> half bright.
// for a device the off cmd includes the brightness, "1300" -> full off.
// for a scene the on/off cmds do not use the brightness setting, still must be present.
// Insteon commands are two bytes, Cmd1 and Cmd2. Cmd1 is the actual command, Cmd2 is the brightness level.
// If a device is dimmable then the on cmd uses Cmd2 to set the brightness, otherwise any value
// except zero will set it on.
// The off command always has Cmd2 set to zero.
// The fan section of a Fanlinc module uses Cmd2 to select light ("00") or fan ("03") status request.
// Commands to turn a Fanlinc fan on or off use extended command format.

// All Commands require a level (00-FF), ie; "11FF", "1300". On/off devices ignore the level, but
// it still must be there. Levels are only used by dimmers and Fanlinc
#define CmdOn           "11FF"    // command to turn on a scene or light, requires level (00->FF)
#define CmdHalf         "117F"    // command to turn a light on at half brightness, level = 7F
#define CmdOnFast       "12FF"    // command to fast turn on a scene or light
#define CmdOff          "1300"    // command to turn off a scene or light
#define CmdOffFast      "1400"    // command to turn off a scene or light
#define CmdBrighten     "1500"    // command to brighten by one step
#define CmdDim          "1600"    // command to dim by one step

// Fan Speed commands for FanLinc. 00->off, 01-7F->low, 80-FE->medium, FF->full.
// The levels defined by these commands will also work with dimmable lights.
#define CmdFanOff       CmdOff    // fan off
#define CmdFanLow       "113F"    // fan on low, any value from 01 to 7F
#define CmdFanMed       "1180"    // fan on medium, any value from 80 to FE
#define CmdFanHigh      CmdOn     // fan on high

// ***** Do not use these or change these *****
#define CmdStatus       "19"      // command to get status
#define Cmd2Lights      "00"      // second byte of status request for lights
#define Cmd2Fans        "03"      // second byte of status request for fans

// Special PLM messages
#define PlmClrBuf       "/1?XB=M=1"         // PLM command to clear the buffer
#define PlmGetBuf       "/buffstatus.xml"   // PLM command to get the buffer contents
#define PlmCmd          "/sx.xml"           // Send as PLM command, requires more data

// Buffer contents
// last byte is number of bytes in the buffer
// response to a command is formatted as: "0250" + DeviceId + "0F" + Cmdxx + "06" + "000...000" + number of bytes
// "0250" + DeviceId + Hub ID + "2x" + "00FF",  "2x" is Ack + hop cnt, indicates light is on  ('FF')
// "0250" + DeviceId + Hub ID + "2x" + "007F",  "2x" is Ack + hop cnt, indicates light is dim ('7F')
// "0250" + DeviceId + Hub ID + "2x" + "0000",  "2x" is Ack + hop cnt, indicates light is off ('00')
// "025C" + DeviceId + Hub ID + "2x" + "0000",  "2x" is Ack + hop cnt, indicates device is not present

// ESP8266 GPIO Pins used for switch inputs, GPIO15, ADC, RX and TX are not used.
// My Insteon Switch board has 10K pullups on all of these GPIO pins. Add switches to
// the ones you want to use.
#define Sw1   16
#define Sw2   14
#define Sw3   12
#define Sw4   13
#define Sw5   5
#define Sw6   4
#define Sw7   0     // this pin is used for programming, do not tie high or low
#define Sw8   2

#define NUMSWITCHES   8     // hardware supports 8 switches max

// twelve bits used for debounce. switch must be in the same state for 12 consecutive
// reads to be considered debounced. Change this if your switches are bouncier or less
// bouncy. Current value says switch must be in the same state for 12mS (1mS loop delay)
#define PINMASK   0x00000FFF

// Insteon device IDs are six characters in length
#define DEVIDLEN  6

// Unknow state for insteon devices
#define UnknownState  -1

// declare the action pointer type. The action pointer points to the action function invoked
// by a button press. The single parameter is a pointer to the TSwitch entry for the pressed
// switch
typedef void (*ActionPtr)(void *p);

// forward references to action functions
void actionToggle(void *p);     // reads status and then sends the 'on' or 'off' cmd to toggle the state
void actionOn(void *p);         // sends only the 'on' command
void actionOff(void *p);        // sends only the 'off' cmd
void actionFanSpeed(void *p);   // cycle through the fan speed settings

// this structure defines the parameters for one switch
typedef struct
{
  byte      pin;        // GPIO pin number
  int16_t   state;      // last known state of the Insteon device
  uint32_t  debounce;   // switch debouncer
  ActionPtr action;     // action to take when switch is pressed
  char      *devId;     // device ID or scene/group number as 'C' string
  char      *cmd2;      // cmd2 to send with status request  
  char      *cmdOn;     // on command string to send
  char      *cmdOff;    // off command string to send
} TSwitch;

// system parameters
typedef struct
{
  char *ssid;       // SSID of network to connect to
  char *passkey;    // network WPA passkey
  char *hub;        // IP address of the Insteon hub
  char *hubuser;    // user name for Insteon hub
  char *hubpass;    // password for Insteon hub

  // array of switches
  TSwitch sws[NUMSWITCHES]; // switch parameters
} TBox;

// Modify this table to add the device ID of your Insteon devices. Add the commands to send
// when the switch is pressed. The 'actionXxxx' is a pointer to the function that performs the
// desired action action when the switch is pressed.
// TSwitch.action == null indicates an unused/unassigned switch
// system initializer, change to values for your WiFi network and Insteon hub
//             SSID, WPA PassKey, IP adrs of Hub, Hub Username, Hub Password
TBox    box = {SSID, PASSKEY, HUBIP, HUBUSER, HUBPASSWORD,
// initializer for all the switches. fill in the switches that you want to use.
//          GPIO Pin, state, debounce, action, Device ID/Scene number, status cmd2, On cmd, Off cmd
               {{Sw1, UnknownState, 0, actionToggle,   Bedroom,   Cmd2Lights, CmdOn,   CmdOff},
                {Sw2, UnknownState, 0, actionToggle,   Bedroom,   Cmd2Lights, CmdHalf, CmdOff},
                {Sw3, UnknownState, 0, actionOn,       Bedroom,   Cmd2Lights, CmdDim,  CmdOff},
                {Sw4, UnknownState, 0, actionToggle,   Kitchen,   Cmd2Lights, CmdOn,   CmdOff},
                {Sw5, UnknownState, 0, actionFanSpeed, Bedroom,   Cmd2Fans,   CmdOn,   CmdOff},
                {Sw6, UnknownState, 0, actionOff,      Bedroom,   Cmd2Fans,   CmdOn,   CmdOff},
                {Sw7, UnknownState, 0, NULL,           Bedroom,   Cmd2Lights, CmdOn,   CmdOff},
                {Sw8, UnknownState, 0, NULL,           Fan3,      Cmd2Lights, CmdOn,   CmdOff}}};

// The hardware has two colums of four switches each arranged as;
// SW1 - SW5
// SW2 - SW6
// SW3 - SW7
// SW4 - SW8
// If you want the switches opposite each other to turn on and off one device then use two switches
// that are SWx and SWx + 4.
               
String    bfr;
char      cmdMsg[256];

//******************************************************************************
void setup()
{
  int i;
  TSwitch *psw;
  
  // make all switches inputs, ESP8266 does not support internal pullups
  pinMode(Sw1, INPUT);
  pinMode(Sw2, INPUT);
  pinMode(Sw3, INPUT);
  pinMode(Sw4, INPUT);
  pinMode(Sw5, INPUT);
  pinMode(Sw6, INPUT);
  pinMode(Sw7, INPUT);
  pinMode(Sw8, INPUT);
  
  Serial.begin(115200);   // Serial connection

  // connect to the WiFi network
  Serial.print("Connecting to ");
  Serial.println(box.ssid);
  
  WiFi.begin(box.ssid, box.passkey);   // WiFi connection

  while (WiFi.status()!= WL_CONNECTED)   // Wait for the WiFI connection completion
  {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(box.ssid);

  // try to get the current status of all the Insteon devices that are being used
  for (i = 0; i < NUMSWITCHES; i++)
  {
    psw = box.sws + i;
    if (psw->action)
    {
      updateState(psw);
      Serial.print("Button ");
      Serial.print(i + 1);
      Serial.print(" State: ");
      if (UnknownState == psw->state)
        Serial.println("Unknown");
      else
        Serial.println((0 == psw->state) ? "Off" : "On");
    }
  } //  for
} //  setup

//******************************************************************************
// Convert a String of hexadecimal characters to a signed 16 bit integer.
int16_t hexToInt(String s)
{
  char ch;
  uint16_t  x = 0;
  int   i;
  
  for (i = 0; i < s.length(); i++)
  {
    ch = s.charAt(i);
    x <<= 4;              // multiply x by 16
    if (ch >= '0' && ch <= '9')
      x |= (uint16_t)(ch - '0');
    else
    {
      ch &= 0xDF;           // convert to upper case alpha
      if (ch >= 'A' && ch <= 'F')
        x |= (uint16_t)(ch - 0x37);
    }
  } //  for

  return x;
} //  hexToInt

//******************************************************************************
// look for a status response message. message is prefixed with either;
// "0262" - cmd response, indicate last cmd sent
// "0250" - status response, indicates state of the device
// The reply message is a String in bfr.
// Reply message format; 0250 + devID + HubID + 2x + xx + status
// Reply message format; 025C + devID + HubID + 2x + xx + status
// x5C indicates device is not present (powered off)
// 2x is '2' for Ack and 0-F for hop count
// xx is any value
// status is 00-FF, where 00 is off and FF is on full bright

bool parseReply(String reply, TSwitch *psw)
{
  bool  b = false;
  int   idlen = strlen(psw->devId);
  int   msglen = 16 + idlen;
  int   index = 0;
  char  hdr[11];
  int   i;
  
  sprintf(hdr, "0250%s", psw->devId);
  while (index < reply.length() - msglen)
  {
    // look for starting header
    index = reply.indexOf(hdr, index);
    if (-1 != index)
    {
      // we found a header with matching device Id
      Serial.println();
      Serial.println(bfr.substring(index, index + idlen + 16));
      index += idlen + 14;
      String s = reply.substring(index, index + 2);

      psw->state = hexToInt(s);
      psw->state &= 0x00FF;
       
      b = true;
      break;
    }
  } //  while

  if (!b)
  {
    sprintf(hdr, "025C%s", psw->devId);
    index = 0;
    while (index < reply.length() - msglen)
    {
      // look for starting header
      index = reply.indexOf(hdr, index);
      if (-1 != index)
      {
        // we found a header with matching device Id
        // the device is not present
        Serial.println();
        Serial.println(bfr.substring(index, index + idlen + 16));
        // state is unknown, make it off
        psw->state = UnknownState;      // full 16 bit value
        b = true;
        break;
      }
    } //  while
  }

  return b;  
} //  parseReply

//******************************************************************************
// Status response from the device to the hub takes time. The hub is polled
// until a status response message is found in the buffer.
#define BfrDelay    100
void updateState(TSwitch *psw)
{
  char msg[25];

  for (int i = 0; i < 20; i++)
  {
    if (strlen(psw->devId) < DEVIDLEN)
    {
      // cannot get status of a scene
      psw->state = UnknownState;
      Serial.println();
      sprintf(msg, "Scene %s status is unknown", psw->devId);
      Serial.println(msg);
      break;
    }
    else
    {
      // clear PLM buffer
      sendPlmCmd(PlmClrBuf);
      delay(10);
  
      // get status of a device
      sendDeviceCmd(psw, CmdStatus);
      
      delay(BfrDelay);
        
      // get buffer
      sendPlmCmd(PlmGetBuf);
      if (parseReply(bfr, psw))
      {
        sprintf(msg, "State %04X, %s, Time: %dmS", psw->state,
               (UnknownState == psw->state) ? "Unknown" : (0 == psw->state) ? "Off" : "On", i * BfrDelay);
        Serial.println(msg);
        break;
      }
    }
  } //  for
} //  updateState

//******************************************************************************
// Send a message to the hub. The message is in the globla cmdMsg buffer.
int sendToHub(void)
{
  int httpCode = 404;
  
  if (WiFi.status()== WL_CONNECTED)   // Check WiFi connection status
  {
    HTTPClient http;                  // Declare object of class HTTPClient

//    Serial.println("Sending to hub");
//    Serial.println(cmdMsg);
    
    http.begin(cmdMsg);                             // Specify request destination
    http.addHeader("Content-Type", "text/plain");   // Specify content-type header
    httpCode = http.GET();                          // Send the request
    bfr = http.getString();                         // Get the response payload
    http.end();                                     // Close connection
  }
  else
  {
    Serial.println("Error in WiFi connection");   
  }

  return httpCode;
} //  sendToHub

//******************************************************************************
// Insteon uses two command formats, standard and extended. Most devices use the
// standard format. The fan part of a Fanlinc device uses extended format, the
// light part of a Fanlinc device uses standard format. The Cmd2 field of the
// TSwitch structure has "00" for devices that uses standard format and a value
// of "03" for devices that require extended format. A status request is always
// send using standard format
void stdCmd(TSwitch *psw, char *cmd)
{
  if (0 == strcmp(cmd, CmdStatus))
  {
    // the get status cmd uses the cmd2 byte from the TSwitch structure
    sprintf(cmdMsg, "http://%s:%s@%s:%s/3?0262%s0F%s%s=I=3", 
            box.hubuser, box.hubpass, box.hub, HTTPPORT, psw->devId, cmd, psw->cmd2);
  }
  else
  {
    sprintf(cmdMsg, "http://%s:%s@%s:%s/3?0262%s0F%s=I=3", 
            box.hubuser, box.hubpass, box.hub, HTTPPORT, psw->devId, cmd);
  }
} //  stdCmd

//******************************************************************************
// Extended commands use a flags byte of "1F" and fourteen extra bytes appended
// to the standard command. For the fan section of a Fanlinc the first byte of
// the extra fourteen is "02", indicates cmd is for the fan.
// flags byte changes, 0F -> 1F
// add 02 to the end of the message for device type (02 -> fan)
// add 13 bytes of 00 to the end of the message
void extCmd(TSwitch *psw, char *cmd)
{
  sprintf(cmdMsg, "http://%s:%s@%s:%s/3?0262%s1F%s0200000000000000000000000000=I=3", 
          box.hubuser, box.hubpass, box.hub, HTTPPORT, psw->devId, cmd);
} //  extCmd


//******************************************************************************
// Use the Cmd2 field to determine the format to use for sending a command. Get
// status is always sent using standard format.

// Status requests are always send using standard format. The fan section of a
// Fanlinc device require on/off commands to use extended format. The cmd itself
// and the cmd2 field of the TSwitch structure determeine the format to use.
int sendDeviceCmd(TSwitch *psw, char *cmd)
{
  // create cmd message in the global cmdMsg buffer
  if ((0 == strncmp(cmd, CmdStatus, 2)) || (0 == strcmp(psw->cmd2, Cmd2Lights)))
  {
    stdCmd(psw, cmd);
  }
  else
  {
    extCmd(psw, cmd);
  }
  
#ifdef DEBUG
  Serial.print("Cmd: ");
  Serial.println(cmdMsg);
#endif
  return sendToHub();
} //  sendDeviceCmd

//******************************************************************************
// Create a PLM style command and then send it to the hub.
void plmCmd(char *devId, char *cmd)
{
  char cmdbfr[25];
  
  sprintf(cmdbfr, "/sx.xml?%s=%s", devId, cmd);
  sendPlmCmd(cmdbfr);
} //  plmCmd

//******************************************************************************
// An alternative format to send commands to the hub. Used mainly for commands 
// that manipulate the PLM of the hub and not Insteon devices.
int sendPlmCmd(char *cmd)
{
  // create the command message to send
  sprintf(cmdMsg, "http://%s:%s@%s:%s%s", box.hubuser, box.hubpass, box.hub, HTTPPORT, cmd);
  return sendToHub();
} //  sendPlmCmd

//******************************************************************************
// Send a scene command using the short form of the standard command. 
int sendSceneCmd(TSwitch *psw, char *cmd)
{
  char cb[3];

  // we only want the first two bytes of the cmd
  cb[0] = cmd[0];
  cb[1] = cmd[1];
  cb[2] = '\0';

  // create the command message to send, use the short form, cmd is first, followed
  // by scene number. scenes can only be on or off, no level
  sprintf(cmdMsg, "http://%s:%s@%s:%s/0?%s%s=I=0", box.hubuser, box.hubpass, box.hub, HTTPPORT, cb, psw->devId);

#ifdef DEBUG
  Serial.print("Scene cmd: ");
  Serial.println(cmdMsg);
#endif
  return sendToHub();
} //  sendSceneCmd

//******************************************************************************
// Get the PLM buffer and return true if the command was accepted.
bool cmdAccepted(char *hdr)
{
  bool accepted = false;
  int pos;

  // check for command accepted by the device
  sendPlmCmd(PlmGetBuf);
#ifdef DEBUG
  Serial.print(bfr);
#endif
  pos = bfr.indexOf(hdr);
  if (-1 != pos)
  {
#ifdef DEBUG
    Serial.print(pos);
    Serial.print(", ");
    Serial.print(hdr);
    Serial.print(", Found: ");
    Serial.println(bfr.substring(pos));
#endif
    // command was accepted
    accepted = true;
  }

  return accepted;
} //  cmdAccepted

//******************************************************************************
// Reads the current state of the device and sends the command to change the
// state. Be patient, the hub takes a few seconds to get status from the devices.
// This can only be used with devices. Repeat the command until the device
// reports the command was accepted.
void actionToggle(void *p)
{
  TSwitch *psw = (TSwitch *)p;
  uint16_t target = hexToInt(psw->cmdOn);

  target &= 0x00FF;
  
  updateState(psw);

  // repeat ten times or until command is accepted by the device
  for (int i = 0; i < 10; i++)
  {
    sendDeviceCmd(psw, (target == psw->state) ? psw->cmdOff : psw->cmdOn);

    delay(100);
    
    // check for command accepted by the device
    if (cmdAccepted("0262"))
      break;
  } //  for
} //  actionToggle

//******************************************************************************
// Sends the command from the cmdOn field of the TSwitch structure. This works
// with devices and scenes. Repeat the command until the it is accepted.
void actionOn(void *p)
{
  TSwitch *psw = (TSwitch *)p;
  // messages prefixed with "0262" are from devices.
  // messages prefixed with "0261" are from scenes.
  const char *hdr = (strlen(psw->devId) < DEVIDLEN) ? "0261" : "0262";
  
  // repeat ten times or until command is accepted
  for (int i = 0; i < 10; i++)
  {
    // clear PLM buffer
    sendPlmCmd(PlmClrBuf);
    delay(10);

    // send the command
    if (strlen(psw->devId) < DEVIDLEN)
      sendSceneCmd(psw, psw->cmdOn);
    else
      sendDeviceCmd(psw, psw->cmdOn);

    delay(100);
    
    // check for command accepted by the device
    if (cmdAccepted((char *)hdr))
      break;
  } //  for
} //  actionOn

//******************************************************************************
// Sends the command from the cmdOff field of the TSwitch structure. This works
// with devices and scenes. Repeat the command until the it is accepted.
void actionOff(void *p)
{
  TSwitch *psw = (TSwitch *)p;
  // messages prefixed with "0262" are from devices.
  // messages prefixed with "0261" are from scenes.
  const char *hdr = (strlen(psw->devId) < DEVIDLEN) ? "0261" : "0262";
  
  // repeat ten times or until command is accepted
  for (int i = 0; i < 10; i++)
  {
    // clear PLM buffer
    sendPlmCmd(PlmClrBuf);
    delay(10);

    // send the command
    if (strlen(psw->devId) < DEVIDLEN)
      sendSceneCmd(psw, psw->cmdOff);
    else
      sendDeviceCmd(psw, psw->cmdOff);

    delay(100);
    
    // check for command accepted by the device
    if (cmdAccepted((char *)hdr))
      break;
  } //  for
} //  actionOff

//******************************************************************************
// Sends the command for the next fan speed setting or fan off. Each switch
// increases the fan speed. At full speed the next press turns off the fan.
// Repeats the command until it is accepted.
void actionFanSpeed(void *p)
{
  TSwitch *psw = (TSwitch *)p;

  // get the current fan speed by reading fan state
  updateState(psw);

  // if state comes back as unknown then the fan will not turn on

  for (int i = 0; i < 10; i++)
  {
    // use the current fan speed to select the next fan speed
    if (0 == psw->state)
      sendDeviceCmd(psw, CmdFanLow);
    else if (psw->state < 0x0080)
      sendDeviceCmd(psw, CmdFanMed);
    else if (psw->state < 0x00FF)
      sendDeviceCmd(psw, psw->cmdOn);
    else
      sendDeviceCmd(psw, psw->cmdOff);

    delay(100);
    
    // check for command accepted by the device
    if (cmdAccepted("0262"))
      break;
  } //  for
} //  actionFanSpeed


//******************************************************************************
// debounce a switch and perform the assigned task when pressed. A switch
// must be released before it can be pressed again. holding a switch down
// does not cause the same action to be repeated.
// rotate the current GPIO pin state into the debounce value then mask it
// with PINMASK. if the result is zero the the switch is debounced and pressed.
// if the result is the same as the PINMASK then the switch is debounced and released.
// no match to zero or PINMASK indicates that the switch is bouncing. Adjust the number
// one bits in PINMASK to exceed the bounce time of your switches. if the debounce
// matches last then the state of the debounced switch has not changed.
bool debouncer(TSwitch *psw)
{
  uint32_t  last = psw->debounce;
  char      bfr[20];
  
  psw->debounce <<= 1;
  psw->debounce |= digitalRead(psw->pin);
  psw->debounce & PINMASK;
 
  // check for change
  if (last != psw->debounce)
  {
    // the pin state has changed
    if (0 == psw->debounce)
    {
      // the switch is pressed, perform the desired action
#ifdef DEBUG      
      sprintf(bfr, "Sw %d pressed", psw->pin);
      Serial.println(bfr);
#endif      
      if (psw->action)
      {
        // the switch has an assigned action, execute it
        psw->action((void *)psw);
      }
    }
  }
} //  debouncer

//******************************************************************************
// run the debouncer on each of the eight switches. the debouncer detects when a
// debounced switch has been pressed and runs the associated action.
void loop()
{
  int i;
  TSwitch *psw;
  
  for (i = 0; i < NUMSWITCHES; i++)
  {
    psw = box.sws + i;
    debouncer(psw);
  }

  delay(1); 
} //  loop


