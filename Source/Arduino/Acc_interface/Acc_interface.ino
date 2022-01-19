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
#define X 1
#define Y 2
#define Z 3

//Selection of Axis to Send over BT
#define SEND_AXIS X
#define NUM_AXIS  3
#define MEAN_K  10
#define PEAK_K  3
#define BYPASS_PRE_PROCESSING 0
#define CNT_DOWN_TIMER 300
/***********GLOBAL DATA*****************/
const int acc_X = A0;
const int acc_Y = A3;
const int acc_Z = A6;
const int motor = 19;

const int flex_1 = A18;
BluetoothSerial SerialBT;


float ip_z1;
float dc_z1;
float mean_z1[MEAN_K];
float peak_z1[PEAK_K];
bool isTriggered;
float prevSlope;
float currSlope;
float threshold;
int meanPtr;
bool startTimer;
float mean_x;
float ip_max;
float ip_min;
int triggerCnt;
int cntDwnTimer;
float ip,op;
bool isInit=true;
int idx; 
int jdx;
float flex1;

/******FUNCTION DEFINITIONS*******/
float meanNormalization(float fIp);
bool peakDetector(float fIp, float threshold);
void firstDifference(float fIp);
/************MAIN***************/

// the setup function runs once when you press reset or power the board
void setup() {

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(motor,OUTPUT);
  //SerialBT.begin("SmartGlove");
  Serial.begin(115200);
  //clear all states
    ip_z1=0;
    dc_z1=0;
    meanPtr=0;
    for(idx=0; idx<MEAN_K; idx++)
    {
      mean_z1[idx]=0;
    }
  isInit=true;
  ip_max=0;
  ip_min=0;
  triggerCnt=0;
  cntDwnTimer = CNT_DOWN_TIMER;
  startTimer =false;
}
  
// the loop function runs over and over again forever
void loop() 
{

  ip= analogRead(acc_Y); 
  delay(1);
 if(isInit)
 {
  ip_max=ip;
  ip_min=ip;
  isInit=false;
 }
#if(BYPASS_PRE_PROCESSING==1)
    op=ip;
    Serial.println(op);
    delay(90);
#else
  op=meanNormalization(ip);
  isTriggered= peakDetector(op,0.25);
  flex1=analogRead(flex_1);
  Serial.println(flex1);
  if(isTriggered==true)
  {
     Serial.println("Triggered");
     digitalWrite(motor,HIGH);
     delay(1000);
     digitalWrite(motor,LOW);
     delay(1000);
  }
#endif
  delay(10);
}

/**************FYNCTIONS************************************/


float meanNormalization(float fIp)
{
  float fOp;
  mean_z1[meanPtr++]=fIp;
  meanPtr=meanPtr%(MEAN_K);
  float sum=0;
  for(idx=0; idx<MEAN_K; idx++)
  {
   sum+= mean_z1[idx];
   ip_max = max(ip_max,mean_z1[idx]);
   ip_min = min(ip_min,mean_z1[idx]);
  }
  mean_x=sum/MEAN_K;

  fOp = (ip-mean_x)/(ip_max-ip_min);

  return fOp;
}


bool peakDetector(float fIp, float threshold)
{

  int m=0;
  float avg;
  
  for(m=0;m<PEAK_K-1;m++)
  {
  peak_z1[m]=peak_z1[m+1];
  }
  peak_z1[PEAK_K-1]= fIp;

  if((peak_z1[1]>peak_z1[0]) && (peak_z1[1] > peak_z1[2]) && (peak_z1[1] > threshold))
  {
   triggerCnt++;
  }
  if(triggerCnt==1)
  {
    /*start timer*/
    startTimer=true;
  }
  if(startTimer==true)
  {
     cntDwnTimer--;
  }
  if(cntDwnTimer==1)
  {
     cntDwnTimer = CNT_DOWN_TIMER;
     triggerCnt=0;
     startTimer=false;
  }
  if(cntDwnTimer>1 && triggerCnt==3)
  {
      cntDwnTimer = CNT_DOWN_TIMER;
      triggerCnt=0;
      startTimer=false;
      return true;
  }
  else
  {
    return false;
  }

}



