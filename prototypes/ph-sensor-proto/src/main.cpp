#include <Arduino.h>

/*
    IMPORTANT: This project/code was tested on Arduino Uno

    e-Gizmo PH Sensor E-201-C
      This sample sketch is for pH sensor module
    an easy output voltage to compare it to pH value.
    See the table below:
    pH Value  |   Output (V)
    ========================
        4         3.071
        7         2.535
        10        2.066


    DANK NOTE: Looks very linear, so...
    pH = m * V + b
    m = -5.9705
    b = 22.3343

    The device out of the box produces numnbers around 3.7
    whihc is a negative pH.  So possibly need to calibrate this

    For pH scale table for reference of the known substance.
    Here's the link: http://www.seafriends.org.nz/dda/ph.htm
    (See the pH scale)

    Modified by e-Gizmo Mechatronix Central
    http://www.e-gizmo.com
    July 18,2017

*/
#define SENSOR A0            //pH meter Analog output to Arduino Analog Input 0
#define OFFSET 0.00            //deviation compensate
#define LED 13
#define SAMPLING_INTERVAL 20
#define PRINT_INTERVAL 800
#define ARRAY_LENGTH  40    //times of collection
int PH_ARRAY[ARRAY_LENGTH];   //Store the average value of the sensor feedback
int PH_ARRAY_INDEX=0;

double slope = -5.9705;
double intercept = 22.3343;

// Declare functions
double AVERAGE_ARRAY(int* ARR, int NUMBER);

void setup(void)
{
  pinMode(LED,OUTPUT);
  Serial.begin(9600);
  Serial.println("PH SENSOR KIT VOLTAGE TEST!");    //Test the serial monitor
}

void loop(void)
{
  static unsigned long SAMPLING_TIME = millis();
  static unsigned long PRINT_TIME = millis();
  static float VOLTAGE;

  if(millis()-SAMPLING_TIME > SAMPLING_INTERVAL)
  {
      PH_ARRAY[PH_ARRAY_INDEX++]=analogRead(SENSOR);
      if(PH_ARRAY_INDEX==ARRAY_LENGTH)PH_ARRAY_INDEX=0;
      VOLTAGE = AVERAGE_ARRAY(PH_ARRAY, ARRAY_LENGTH)*5.0/1024 - 1.5;
      SAMPLING_TIME=millis();
  }

  if(millis() - PRINT_TIME > PRINT_INTERVAL)   //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
  {
	   Serial.print("VOLATGE OUTPUT: ");
     Serial.println(VOLTAGE,2);

     Serial.print("pH: ");
     Serial.println( VOLTAGE * slope + intercept);
     digitalWrite(LED,digitalRead(LED)^1);
     PRINT_TIME=millis();
  }
}



double AVERAGE_ARRAY(int* ARR, int NUMBER){
  int i;
  int max,min;
  double AVG;
  long AMOUNT=0;
  if(NUMBER<=0){
    Serial.println("ERROR!/n");
    return 0;
  }
  if(NUMBER<5){   //less than 5, calculated directly statistics
    for(i=0;i<NUMBER;i++){
      AMOUNT+=ARR[i];
    }
    AVG = AMOUNT/NUMBER;
    return AVG;
  }else{
    if(ARR[0]<ARR[1]){
      min = ARR[0];max=ARR[1];
    }
    else{
      min=ARR[1];max=ARR[0];
    }
    for(i=2;i<NUMBER;i++){
      if(ARR[i]<min){
        AMOUNT+=min;        //arr<min
        min=ARR[i];
      }else {
        if(ARR[i]>max){
          AMOUNT+=max;    //arr>max
          max=      AMOUNT+=ARR[i];
        }else{
          AMOUNT+=ARR[i]; //min<=arr<=max
        }
      }//if
    }//for
    AVG = (double)AMOUNT/(NUMBER-2);
  }//if
  return AVG;
}


// void setup() {
//  // initialize serial communication at 9600 bits per second:
//  Serial.begin(9600);
// }

// // the loop routine runs over and over showing the voltage on A0
// void loop() {
//  // read the input on analog pin 0:
//  int sensorValue = analogRead(A0);
//  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
//  float voltage = (sensorValue) * (5.0 / 1023.0) - 1.5;
//  // print out the value you read:
//  Serial.println(voltage);
//  delay(300);
// }