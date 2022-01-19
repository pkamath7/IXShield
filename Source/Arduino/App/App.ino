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

/******FUNCTION DEFINITIONS*******/
float meanNormalization(float fIp);
bool peakDetector(float fIp, float threshold);
void firstDifference(float fIp);
/************MAIN***************/

// Initialize
void setup() {

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
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
  
// Main loop
void loop() 
{
  if(SEND_AXIS==X)
  {
   ip= analogRead(acc_X); 
  }
  else if(SEND_AXIS==Y)
  {
    ip= analogRead(acc_Y); 
  }
  else 
  {
    ip= analogRead(acc_Z); 
  }
   delay(1);
  if(isInit)
  {
    ip_max=ip;
    ip_min=ip;
    isInit=false;
  }
  
  if(BYPASS_PRE_PROCESSING==1)
  {
      op=ip;
  }
  else
  {
    op=meanNormalization(ip);
    isTriggered= peakDetector(op,0.25);
    if(isTriggered==true)
    {
       Serial.println("Triggered");
    }
  }
  delay(9);
}

/**************FUNCTIONS************************************/


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
