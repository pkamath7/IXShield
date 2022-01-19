import processing.serial.*;
int bgcolor;   // Background color
int fgcolor;   // Fill color
Serial myPort;      // The serial port
int[] serialInArray = new int[3]; // Where we'll put what we receive
int serialCount = 0;     // A count of how many bytes we receive
int xpos, ypos;     // Starting position of the ball
boolean firstContact = false;  // Whether we've heard from the
          // microcontroller
 int hor_pos;
 int vert_pos;
 int speed;
 boolean isCalbirationDone;
 int score;
 int calibStage;
 float freq=500;
 float amplitude;
 float cos_speed=freq*2*PI;
 PImage rocket;
 PImage asteroid;
 PImage bg;

 public class Pos
 {
  public int positionX;
  public int positionY;
  
  Pos(int x, int y)
  {
    positionX=x;
    positionY=y;
  }
 }
 
ArrayList<Pos>calibrationPositions = new ArrayList<Pos>();
 
void setup() {
 // Stage size 
 size(300, 600); 
 
 // Print a list of the serial ports, for debugging purposes:
 println(Serial.list());
 String portName = Serial.list()[0];
 myPort = new Serial(this, portName, 115200);
 
 // Set the starting position of the ball (middle of the stage)
 xpos = width/2;
 ypos = height/2;
 loadGameImages();

 initCalibrationConfig();
}
void draw() 
{

 //actuateFeedback();
 boolean hit=false;
 background(bgcolor);
 image(bg,0,0);
 DrawCalibrationWindow ();
  fill(255);
 // Draw the user shape
 image(rocket,xpos, ypos);
 
 //If calibration is complete start game
 if(isCalbirationDone)
 { //<>//
   renderBall(hor_pos, vert_pos);
   vert_pos+=speed;
   if(vert_pos>=height)
   {
      hor_pos=int(random(0,width));
   }
   vert_pos%=height;
   
   hit=detectHit(hor_pos,vert_pos,xpos,ypos);
   if(hit==true)
   {
     for(int j=0;j<2;j++)
     {
     fill(0);
     rect(0,0,width,height);
     fill(255,0,0);
     textSize(50);
     stroke(20);
     text("HIT",(width/2)-40,(height/2));
     }
     //print("HIT");
      actuateFeedback();
   }
 }
}
void serialEvent(Serial myPort) {
 // read a byte from the serial port:
 int inByte = myPort.read();
 // if this is the first byte received, and it's an A,
 // clear the serial buffer and note that you've
 // had first contact from the microcontroller.
 // Otherwise, add the incoming byte to the array:
 if (firstContact == false) {
 if (inByte == 'A') {
  myPort.clear();   // clear the serial port buffer
  firstContact = true;  // you've had first contact from the microcontroller
  myPort.write('A');  // ask for more
 }
 }
 else {
 // Add the latest byte from the serial port to array:
 serialInArray[serialCount] = inByte;
 serialCount++;
 // If we have 3 bytes:
 if (serialCount > 2 ) {
  ypos = int((float(serialInArray[0])/255.0)*height);
  xpos = int((float(serialInArray[1])/255.0)*width);
  fgcolor = serialInArray[2];
  // print the values (for debugging purposes only):
  //println(xpos + "t" + ypos + "t" + fgcolor);
  // Send a capital A to request new sensor readings:
  myPort.write('A');
  // Reset serialCount:
  serialCount = 0;
 }
 }
}

void renderBall(int pos_x, int pos_y)
{
 // Draw the shape
 image(asteroid,pos_x, pos_y); 
}

boolean detectHit(int obj_pos_x, int obj_pos_y, int user_pos_x, int user_pos_y)
{
  boolean isHit=false;
  if((obj_pos_x>(user_pos_x-20))&& (obj_pos_x<(user_pos_x+20))&& (obj_pos_y>(user_pos_y-20)) && (obj_pos_y<(user_pos_y+20)))
  {
    isHit=true;
  }
  return isHit;
}


void DrawCalibrationWindow()
{
  if(!isCalbirationDone)
  {
    fill(255);
    textSize(20);
    stroke(20);
    strokeWeight(4);
    text("CALIBRATION",(width/2)-60,(height/2));
    int target_posX=calibrationPositions.get(calibStage).positionX;
    int target_posY=calibrationPositions.get(calibStage).positionY;
    noFill();
    stroke(255);
    ellipse(target_posX,target_posY,30,30);
    if((target_posX+10>xpos)&& (target_posX-10<xpos)&& (target_posY+10>ypos) && (target_posY-10<ypos))
    {
      calibStage+=1;
    }
    if(calibStage==4)
    {
      isCalbirationDone=true;
    }
  }
}

//Configure game parameters
void initCalibrationConfig()
{ 
 hor_pos=int(random(0,width));
 vert_pos=0;
 speed=8;
 amplitude =width; 
 score=0;
 calibStage=0;
 /*calibration settings*/
 calibrationPositions.add(new Pos(15,height/2));
 calibrationPositions.add(new Pos(width-15,height/2));
 calibrationPositions.add(new Pos(width/2,15));
 calibrationPositions.add(new Pos(width/2,height-15));
}

void loadGameImages()
{
 rocket= loadImage("rocket.png");
 asteroid=loadImage("asteroid.png");
 bg = loadImage("space.jpg");
}

void actuateFeedback()
{
  myPort.clear();   // clear the serial port buffer
  myPort.write('S');  // ask for more
  delay(1500);
}