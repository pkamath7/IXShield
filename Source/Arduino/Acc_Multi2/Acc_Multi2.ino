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
#define MEAN_K  3
#define BYPASS_PRE_PROCESSING 0
/***********GLOBAL DATA*****************/
const int acc_pins[NUM_AXIS]= {A0,A3,A6};
const int motor = 19;
const int flex[NUM_FLEX] = {A15,A16,A17,A19,A18};

BluetoothSerial SerialBT;


float ip_z1;
float dc_z1;
float mean_z1[NUM_AXIS][MEAN_K];
int meanPtr;
bool startTimer;
float meanVal[NUM_AXIS];
float ip_max[NUM_AXIS];
float ip_min[NUM_AXIS];
float ip[NUM_AXIS],op[NUM_AXIS];
bool isInit=true;
int idx; 
int jdx;
float flex1;

/******FUNCTION DEFINITIONS*******/

void meanNormalization(float *fIp, float *fOp, int num);  

/************MAIN***************/

// the setup function runs once when you press reset or power the board
void setup() 
{
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(motor,OUTPUT);
  Serial.begin(115200);
  //clear all states
  meanPtr=0;

 for(jdx=0; jdx<NUM_AXIS; jdx++)
  {
    ip_max[jdx]=0;
    ip_min[jdx]=0;
    for(idx=0; idx<MEAN_K; idx++)
    {
      mean_z1[jdx][idx]=0;
    }
  }
  isInit=true;
}
  
// the loop function runs over and over again forever
void loop() 
{
for(idx=0;idx<NUM_AXIS;idx++)
{
 ip[idx]= analogRead(acc_pins[idx]); 
 delay(2);
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

 
#if(BYPASS_PRE_PROCESSING==1)
  Serial.println("start");
  for(idx=0;idx<NUM_AXIS;idx++)
  {
      op[idx]=ip[idx];
      Serial.println(String(op[idx]));
  }
  Serial.println("stop");
  delay(900);
#else
  meanNormalization(ip,op,NUM_AXIS);
  Serial.println("start");
  for(idx=0;idx<NUM_AXIS;idx++)
  {
      Serial.println(String(op[idx]));
  }
  Serial.println("stop");
  delay(900);
#endif
  delay(10);
}

/**************FUNCTIONS************************************/


void meanNormalization(float *fIp, float *fOp, int num)
{
  for(jdx=0;jdx<num;jdx++)
  {
     ip_max[jdx] = max(ip_max[jdx],fIp[jdx]);
     ip_min[jdx] = min(ip_min[jdx],fIp[jdx]);
    fOp[jdx] = (fIp[jdx]-ip_min[jdx])/(ip_max[jdx]-ip_min[jdx]);
  }
}





