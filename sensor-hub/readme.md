# Garden Sensor Hub
## Overview
This is the intro

## Build of Materials
___
**Arduino IoTerrific Bundle** MKR WiFi 1010, MRK Mem Shield, MKR Connector Carrier
- Purchased: [Arduino](https://store.arduino.cc/usa/ioterrific-bundle)
- Reference: [MRK Connector Carrier](https://store.arduino.cc/usa/arduino-mkr-connector-carrier)
- Reference: [MRK Connector Carrier Docs]  

**Ultrasonic Sensor** Grove Ultrasonic Ranger
- Purchased: [seeedstudio](https://www.seeedstudio.com/Grove-Ultrasonic-Distance-Sensor.html)
- Reference: [wiki](http://wiki.seeedstudio.com/Grove-Ultrasonic_Ranger/)
- Reference: [Beginners Guide] (https://www.evelta.com/blog/a-beginners-guide-to-ultrasonic-sensing-technology/)
- Reference: [US100 with Temp] (http://arduinoinfo.mywikis.net/wiki/UltraSonicDistance)


**pH Sensor** GAOHOU PH0-14 Value Detect Sensor Module + PH Electrode Probe BNC
- Purchased: [Amazon](https://www.amazon.com/gp/product/B0799BXMVJ/ref=ppx_yo_dt_b_asin_title_o03_s00?ie=UTF8&psc=1)
- References: [Spec pdf](http://www.baaqii.net/promanage/BU0203%2BBU0481.pdf)
- References: [Github sample code] (https://github.com/e-Gizmo/PH-Sensor-E-201-C/blob/master/pH_sensor_sample_code/pH_sensor_sample_code.ino)


**Thermocouple** HiLetgo Thermocouple Temperature Sensor - MAX6675 - won't keep this long-term
- Purchased: [Amazon](https://www.amazon.com/gp/product/B01HT871SO/ref=ppx_yo_dt_b_asin_title_o02_s00?ie=UTF8&psc=1)
- Reference: [Video](https://www.youtube.com/watch?v=0KJ8H_SUWp0)
- Reference: [Blog with details](http://henrysbench.capnfatz.com/henrys-bench/arduino-temperature-measurements/max6675-temp-module-arduino-manual-and-tutorial/)
- [MAX6675 Library](https://github.com/adafruit/MAX6675-library)

### Other References
- IoT Core integration
- [Best practices for IoT Core](https://d1.awsstatic.com/whitepapers/Designing_MQTT_Topics_for_AWS_IoT_Core.pdf)

___

## First time setup
Video on setup of AWS IoT Core and adding a device



## Operating




## TODO
- Fill out the rest of the message structure - both sensor and herald
- Play with SDK and figure out how to connect to MQTT
- Create other files/classes to use, to clean up code
- Fix json message structure so code not confused.  Use json library?


At AWS
- Check data I am storing
- Create rule for notifications (test email integration)

Sensors to add
- Temperature
- pH

pH Sensor, has limited doc (although there is a sketch file to play with as prorotype)
But given the analog reading, others may works as well... like the atlas
https://www.instructables.com/id/ARDUINO-PH-METER/




## More good refernces**
https://d1.awsstatic.com/whitepapers/Designing_MQTT_Topics_for_AWS_IoT_Core.pdf
