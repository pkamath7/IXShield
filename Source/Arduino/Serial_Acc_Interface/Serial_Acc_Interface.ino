/*
 * File Description: 
 * 
 * This file contains code to initilaize accelerometers on the ISX shield when 
 * interfaced with ESP32 The program contains APIs that process raw accelerometer 
 * data from the ADXL 335 accelerometer on the ISX shield. peakDetector() provides 
 * an output indicating if three to and  fro swings of the hand is detected. The 
 * motor is actuated when the thump is detected.
 * 
 * Hardware
 * NODE MCU ESP32
 * Pin Out diagram:https://www.google.com/imgres?imgurl=https%3A%2F%2Fcontent.instructables.com%2FORIG%2FFOL%2FYWLI%2FJEOILQ5U%2FFOLYWLIJEOILQ5U.png%3Ffit%3Dbounds%26frame%3D1%26width%3D1024&imgrefurl=https%3A%2F%2Fwww.instructables.com%2FESP32-Internal-Details-and-Pinout%2F&tbnid=HYvxO4skyW-v-M&vet=12ahUKEwidhuvBiNrtAhWlhuYKHZJdCf4QMygAegUIARCmAQ..i&docid=5CPFwzn7BNAdyM&w=1024&h=578&q=esp32%20nodemcu%20pinout&ved=2ahUKEwidhuvBiNrtAhWlhuYKHZJdCf4QMygAegUIARCmAQ
 * ISX shield 
 * 
 * Connections:
 * 
 * Motor-> + ve->GPIO 19 , -ve->GND
 * ADXL X_OUT -> ADC0
 * ADXL Y_OUT -> ADC3
 * ADXL Z_OUT -> ADC6
 * YP4065-1A Li-Po charger +ve - >Vin (5V input regulated), -ve ->GND
 */

#include "BluetoothSerial.h"

/********** DEFINES*************/
//Selection of Axis to Send over BT
#define NUM_AXIS  3
#define NUM_FLEX  5
#define BYPASS_PRE_PROCESSING 0
/***********GLOBAL DATA*****************/

const int acc_pins[NUM_AXIS]= {A0,A3,A6};
const int motor = 19;
const int flex[NUM_FLEX] = {A15,A16,A17,A19,A18};
bool enableMotor;

BluetoothSerial SerialBT;

float ip_max[NUM_AXIS];
int   send_acc_dat[NUM_AXIS];
float ip_min[NUM_AXIS];
float ip[NUM_AXIS],op[NUM_AXIS];
bool isInit=true;
int idx; 
int jdx;

/******FUNCTION DEFINITIONS*******/

void meanNormalization(float *fIp, float *fOp, int num);  

int firstSensor = 0; // first analog sensor
int secondSensor = 0; // second analog sensor
int thirdSensor = 0; // digital sensor
int inByte = 0;   // incoming serial 

/************MAIN***************/

void setup()
{
 // start serial port at 115200 bps:
 Serial.begin(115200);
 while (!Serial) {
 ; // wait for serial port to connect. Needed for Leonardo only
 }
 pinMode(motor, OUTPUT); // motor output
 
 establishContact(); // send a byte to establish contact until receiver responds

 for(jdx=0; jdx<NUM_AXIS; jdx++)
  {
    ip_max[jdx]=0;
    ip_min[jdx]=0;
  }
  isInit=true;
  enableMotor=false;     
}
void loop()
{
 // if we get a valid byte, read analog ins:
 if (Serial.available() > 0)
 {
   // get incoming byte:
   inByte = Serial.read();
   if(inByte=='S')
   {
    enableMotor=true;
   }
   
   for(idx=0;idx<NUM_AXIS;idx++)
    {
     ip[idx]= analogRead(acc_pins[idx]); 
     delay(10);
    }
  
   if(isInit)
   {
    for(idx=0;idx<NUM_AXIS;idx++)
    {
      ip_max[idx]=ip[idx];
      ip_min[idx]=ip[idx];
    }
    isInit=false;
   }
  
   // read switch, map it to 0 or 255L
   meanNormalization(ip,op,NUM_AXIS);
   
   for(idx=0;idx<NUM_AXIS;idx++)
   {
     send_acc_dat[idx] = int(op[idx]*255);
     // send sensor values:
     Serial.write(send_acc_dat[idx]);
   }
 }

 if(enableMotor)
 {
  digitalWrite(motor,HIGH);
  delay(1000);
  digitalWrite(motor,LOW);
  enableMotor=false;
 }
}



/**************FUNCTIONS************************************/

void establishContact() {
 while (Serial.available() <= 0) {
 Serial.print('A'); // send a capital A
 delay(300);
 }
}


void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    if(inChar='S')
    {
     enableMotor=true;
    }
  }
}

void meanNormalization(float *fIp, float *fOp, int num)
{
  for(jdx=0;jdx<num;jdx++)
  {
     ip_max[jdx] = max(ip_max[jdx],fIp[jdx]);
     ip_min[jdx] = min(ip_min[jdx],fIp[jdx]);
    fOp[jdx] = (fIp[jdx]-ip_min[jdx])/(ip_max[jdx]-ip_min[jdx]);
  }
}

