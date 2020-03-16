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
- References: [PH4502C How to](https://scidle.com/how-to-use-a-ph-sensor-with-arduino/)
- References: [PH4502C How to](https://www.botshop.co.za/how-to-use-a-ph-probe-and-sensor/)


**Thermocouple** HiLetgo Thermocouple Temperature Sensor - MAX6675 - won't keep this long-term
- Purchased: [Amazon](https://www.amazon.com/gp/product/B01HT871SO/ref=ppx_yo_dt_b_asin_title_o02_s00?ie=UTF8&psc=1)
- Reference: [Video](https://www.youtube.com/watch?v=0KJ8H_SUWp0)
- Reference: [Blog with details](http://henrysbench.capnfatz.com/henrys-bench/arduino-temperature-measurements/max6675-temp-module-arduino-manual-and-tutorial/)
- [MAX6675 Library](https://github.com/adafruit/MAX6675-library)

**LED Display** Grove LCD RGB Backlit 16x2
- Purchased: [Amazon](https://www.amazon.com/gp/product/B079GFWPRB/ref=ppx_yo_dt_b_asin_title_o04_s00?ie=UTF8&psc=1)
- Reference: [Grove Github](https://github.com/SeeedDocument/Grove_LCD_RGB_Backlight)
- Reference: [Grove Doc](http://wiki.seeedstudio.com/Grove-LCD_RGB_Backlight/)

**Temperature & Humidity** Grove 
- Purchased: [Seed Studio](https://www.seeedstudio.com/Grove-Temperature-Humidity-Sensor-DHT11.html)
- Reference: [Grove Guthub](https://github.com/Seeed-Studio/Grove_Temperature_And_Humidity_Sensor)
- Reference: [adfruit Github](https://github.com/adafruit/DHT-sensor-library?utm_source=platformio&utm_medium=piohome)

** TDS Sensor** Grove TDS Sensor
- Purchased: [Seed Studio](https://www.seeedstudio.com/Grove-TDS-Sensor-p-4400.html)
- Reference: [Seed Studio Wiki] (http://wiki.seeedstudio.com/Grove-TDS-Sensor/)


### Other References
- IoT Core integration
- [Best practices for IoT Core](https://d1.awsstatic.com/whitepapers/Designing_MQTT_Topics_for_AWS_IoT_Core.pdf)



## First time setup
Video on setup of AWS IoT Core and adding a device



## Operating




## TODO
New accessories
- Water temp
- Figure out display - use uno 
- Figure out pH - use uno 


Cleanup
- Get mac-address to string in messages, ip address to herald message
- Play with SDK and figure out how to connect to MQTT
- Create other files/classes to use, to clean up code (see journal notes)
- Fix json message structure so code not confused.  Use json library?

Physical
- Build a box to install (power, protection, water temp, ambient, water depth, ec) - prototype wood box... 
- Mount for water depth...  Top of container?  Whole box goes on top of container... 


At AWS
- Create rule for notifications (test email integration)
- Work on a simple web-page template
- Figure out how to get data to display

Add display!
- Print out current sensor readings

Sensors to add
- Temperature - wait for new on one.  
- pH - Next on pH - redo prototype on clean MKR 1010 board
Then refactor into existing code for sensor.  
Will need to understand calibration though.  I have sensor..  strips... and fluids.




## More good references**
https://d1.awsstatic.com/whitepapers/Designing_MQTT_Topics_for_AWS_IoT_Core.pdf


How to have a client to IoT
https://stackoverflow.com/questions/48044761/possible-to-subscribe-to-aws-iot-topic-in-lambda
