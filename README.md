# W203 canbus project

Originally a project to render custom text on the IC Cluster, but has grown quite a bit with new features


## Repo contents:
* IC_CUSTOM_TEXT - Contains Arduino code for project
* androidApp - Contains Android app code for project
* can_read - Simple tool to read can frames from car
* Replayer - Replays data captured by can_read in realtime. (Used to analyze can frames and work out functions of frames)

## Reverse engineering documentation
See [here](https://docs.google.com/spreadsheets/d/1krPDmjjwmlta4jAVcDMoWbseAokUYnBAHn67pOo00C0/edit?usp=sharing)

## Hardware setup
![Arduino setup](/setup.png?raw=true "Arduino hardware setup")

Currently, I have 2 MCP2515 modules. 1 with a 8Mhz Clock thats connected to CAN C, and 1 with a 16Mhz clock that is connected to Can B. (I found out that Can B is NOT compatible with the 8Mhz clock due to its odd bitrate of 83.3Kbps). If you end up using 2 modules with 16Mhz clocks, please change the following line in IC_CUSTOM_TEXT/canbuscomm.cpp:
```cpp
this->canC->setBitrate(CAN_500KBPS, MCP_8MHZ);
```
to
```cpp
this->canC->setBitrate(CAN_500KBPS);
```
Failing to do so may result in some absolutely wild behaviour with the rest of the car!

### Connection to Can B
I am using the X30/7 connector to connect to Canbus B. This is a hub (with multiple wires coming out of it) located on the panel below the drivers steering wheel. you will notice it has a few free slots. This is where I have ran the cables to.

### Connection to Can C
Canbus C is a little more difficult. I could not find a connector like for Bus B. Instead, I ran wires to the back of the Instrument cluster. Connected to the instrument are 2 green wires. The Solid green wire is CanH+, and the green wire with a white stripe is CanL-.

## Project status
So far, the project has grown well out of proportion and has ended up being a way to modernise the W203 slightly. The arduino now requires a HC-06 bluetooth module, which then connects to the android application running on an android device. This now serves to give 'hands free music control' to the car, the steering wheel button presses in the AUX Page are now relayed to the phone in order to change tracks. The phone also then tells the Arduino via bluetooth what track is playing. The arduino will then send can frames to the IC Cluster to display the current track playing.

## Controls enabled via the android application
* Turn on / off indicators and hazard lights
* Set custom interval for indicator clicks (faster / slower)
* Unlock doors
* Lock doors
* Toggle ESP
* Retract rear headrests
* Display custom text on the IC display
* Set scroll speed of text across the IC display (only if text > 8 characters)

### Limitations
Currently, I have only worked out how to display 1 line of maximum 8 characters on the IC Display. This means that if the track name is more than 8 characters, the Arduino will shift the string by 1 every 150MS, resulting in the text scrolling across the screen to display a longer message. However, the IC Displays response time is appaling, resulting in somtimes a slushy text animation. idealy, I would like to find out how to lower the size of the text on the display in order to display more than 8 characters at a time
