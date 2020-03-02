## Sonoff Smart Plug (S31) Heater Controller  

This sketch is used on a Sonoff S31 Smart Plug connected to a "dumb" heater with a mechanical thermostat.
Using this will allow electronic temperature control without having to modify the heater itself. The mechanical
thermostat remains functioning and can be set higher that the desired range to act as a sort of safety.


## Warning  

As this deals with a heater, there are still certain dangers. By using this software, you are taking full responsibility
and liability for anything that goes wrong. Any damage, injury, or death are solely you responsibility and I take none. 
You must feel confident in your hardware and software before using this. I would recommend only using your heater on a low 
setting below 75% of the smart switches capacity.

## Requirements

As you can not install sensors onto the S31 (which wouldn't be in a good position for temperature anyway) you must use an 
external device to send the temperature to the switch. This can be any device with a sensor that is also connected to the Blynk
server. The temperature information is sent via the [Bridge](http://docs.blynk.cc/#widgets-other-bridge) feature in Blynk.

## Features  
* Temperature Control (Duh)  
* Temperature Offset: Allows the temperature to drop a set amount before turning on.  
* Anti Short Cycle: Require a minimum "cooldown" before restarting the heater.
* Overide Control; The ability to control the relay directly.
* Amperage, Wattage, and Voltage Display
* Session Power Consumption: (KWh) Resets upon power loss.

## Credits  

The code reading the power monitoring chip and other functions is from the below link and authored by James Eli.  
https://ucexperiment.wordpress.com/2019/01/20/using-a-sonoff-s31-with-blynk/  
I used it as a base and added in the features that I needed.

![Blynk App](https://github.com/mrcory/Sonoff_Controlled_Heater/blob/master/Extra/Blynk_Screenshot.png?raw=true)
