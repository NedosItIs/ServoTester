#include <RotaryEncoder.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64

//------------PINS---------------
//Rotary Encoders
int rotaryCLK1Pin = 13;
int rotaryDT1Pin = 12;
int rotaryButt1Pin = A6;

int rotaryCLK2Pin = 8;
int rotaryDT2Pin = 7;
int rotaryButt2Pin = A7;

//Menue buttons
int selectButtPin = A2;
int exitButtPin = A3;
int nextButtPin = A0;
int prevButtPin = A1;

//Togle Switches
int toggle1Pin = 2;
int toggle2Pin = 4;

//Servos
int servoPins[]={3,5,6,9,10,11};

//----------------------------Variables----------------------------
//Rotary Encoders
int counts[4]={90,90,90,90};
int pulseCount1=0;
int rotary1Push=0;
RotaryEncoder encoder1(rotaryCLK1Pin, rotaryDT1Pin, RotaryEncoder::LatchMode::TWO03);

int pulseCount2=0;
int rotary2Push=0;
RotaryEncoder encoder2(rotaryCLK2Pin, rotaryDT2Pin, RotaryEncoder::LatchMode::TWO03);

//MenueButtons
int menueStates[4]={0,0,0,0};
int menueLastPulses[4];


//ToggleSwitches
bool toggleState1 = false;
bool toggleState2 = false;
int toggleLastPulse1;
int toggleLastPulse2;

//Servos;
Servo servos[6];
int servoMin[6]={0,0,0,0,0,0};
int servoMax[6]={180,180,180,180,180,180};
int servoMidpoint[6]={90,90,90,90,90,90};
bool servoInverted[6]={false,false,false,false,false,false};
int servoOffset[6] = {0,0,0,0,0,0};
int bind[6]={1,3,0,2,4,0};

//Display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//UI
int selector=0;
int menue=0; //0->Not loaded/Error 1->Main 2->Output Set 3->Group Set
int selected=0;

//timing
int lastTime;

void setup() {
  Serial.begin(9600);
  delay(500);
  //Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);

  display.println("Starting");
  display.display();
  delay(500);

  //MenueButtons
  pinMode(selectButtPin,INPUT);
  pinMode(exitButtPin,INPUT);
  pinMode(nextButtPin,INPUT);
  pinMode(prevButtPin,INPUT);
  int currTime=millis();
  for(int i=0;i<4;i++){
    menueLastPulses[i]=millis();
  }


  //Rotary Buttons
  pinMode(rotaryButt1Pin,INPUT);
  pinMode(rotaryButt2Pin,INPUT);
  
  //Toogle States
  pinMode(toggle1Pin,INPUT);
  pinMode(toggle2Pin,INPUT);
  if(digitalRead(toggle1Pin)==1){
    toggleState1=true;
  }
  if(digitalRead(toggle2Pin)==1){
    toggleState2=true;
  }
  toggleLastPulse1 = currTime;
  toggleLastPulse2 = currTime;

  //Servos
  for(int i=0;i<6;i++){
    servos[i].attach(servoPins[i]);servos[i].write(90);
  }

  
  
  //Draw inital Menue
  drawMenueOne();
  menue=1;

  //timing
  lastTime=millis();
  Serial.println("initialized");
}



void loop() {
  int currTime=millis();
  lastTime=currTime;
  //Toogles
  bool newToggleState1=false;
  if(digitalRead(toggle1Pin)==1){
    newToggleState1=true;
  }
  //Toggle 1 Changed
  if(newToggleState1!=toggleState1 && (currTime-toggleLastPulse1)>100){
    toggleState1=newToggleState1;
    if(menue==1){
      drawGroup(0);
      drawGroup(2);
    }
    

    toggleLastPulse1=currTime;
  }
  
  bool newToggleState2=false;
  if(digitalRead(toggle2Pin)==1){
    newToggleState2=true;
  }
  //Toggle 2 changed
  if(newToggleState2!=toggleState2 && (currTime-toggleLastPulse2)>100){
    toggleState2=newToggleState2;
    if(menue==1){
      drawGroup(1);
      drawGroup(3);
    }
    
    
    toggleLastPulse2=currTime;
  }
  
    
  //Rotary Encoder

  //Encoder1
  encoder1.tick();
  static int pos1 = 0;
  int newPos1 = encoder1.getPosition();
  
  if (pos1 != newPos1) {
    pulseCount1++;
    if(pulseCount1==2){
      int dir = (int) encoder1.getDirection();
      if(dir==1){
        //Counterclockwise
        if(menue==1){
          //Main Menue
          if(!toggleState1){
            counts[0]--;
            drawVal(0);
            updateServoGroup(0);
          }else{
            counts[2]--;
            drawVal(2);
            updateServoGroup(0);
          }
        }
        
      }else{
        //Clockwise
        if(menue==1){
          //Main Menue
          if(!toggleState1){
            counts[0]++;
            drawVal(0);
            updateServoGroup(0);
            
          }else{
            counts[2]++;
            drawVal(2);
            updateServoGroup(2);
            
          }
        }
        
      }
      pulseCount1=0;
    }
    pos1 = newPos1;
  }
  //Encoder 2
  encoder2.tick();
  static int pos2 = 0;
  int newPos2 = encoder2.getPosition();
  
  if (pos2 != newPos2) {
    pulseCount2++;
    if(pulseCount2==2){
      int dir = (int) encoder2.getDirection();
      
      if(dir==1){
        //Counterclockwise
        if(menue==1){
          //Main Menue
          if(!toggleState2){
            counts[1]--;
            drawVal(1);
            updateServoGroup(1);
          }else{
            counts[3]--;
            drawVal(3);
            updateServoGroup(3);
          }          
        }else if(menue==2){
          switch(selector){
            case 0:
              if(servoMax[selected]>0){
                servoMax[selected]--;
              }else{servoMax[selected]=0;}
            break;
            case 1:
             if(servoMin[selected]>1){
                servoMin[selected]--;
              }else{servoMin[selected]=1;}
            break;
            case 2:
              if(servoOffset[selected]>1){
                servoOffset[selected]--;
              }else{servoOffset[selected]=1;}
            break;
            case 3:
              if(servoMidpoint[selected]>1){
                servoMidpoint[selected]--;
              }else{servoMidpoint[selected]=1;}
            break;
            case 4:
              servoInverted[selected]=!servoInverted[selected];
            break;
            case 5:
              if(bind[selected]>0){
                bind[selected]--;
              }else{bind[selected]=0;}

            break;
          }
          drawContent(selector);
        }
      }else{
        //Clockwise
        if(menue==1){
          //Main Menue
          if(!toggleState2){
            counts[1]++;
            drawVal(1);
            updateServoGroup(1);
          }else{
            counts[3]++;
            drawVal(3);
            updateServoGroup(3);
          }
        }else if(menue==2){
          switch(selector){
            case 0:
              if(servoMax[selected]<180){
                servoMax[selected]++;
              }else{servoMax[selected]=180;}
            break;
            case 1:
             if(servoMin[selected]<179){
                servoMin[selected]++;
              }else{servoMin[selected]=170;}
            break;
            case 2:
              if(servoOffset[selected]<179){
                servoOffset[selected]++;
              }else{servoOffset[selected]=179;}
            break;
            case 3:
              if(servoMidpoint[selected]<179){
                servoMidpoint[selected]++;
              }else{servoMidpoint[selected]=179;}
            break;
            case 4:
              servoInverted[selected]=!servoInverted[selected];
            break;
            case 5:
              if(bind[selected]<3){
                bind[selected]++;
              }else{bind[selected]=3;}
            break;
          }
          drawContent(selector);
        }
      }
       pulseCount2=0;
    }
    pos2 = newPos2;
  }

  //Menue Buttons
  int newMenueStates[4]={digitalRead(prevButtPin),digitalRead(nextButtPin),digitalRead(exitButtPin),digitalRead(selectButtPin)};
  
  //--------------Previous--------------
  if(menueStates[0]!=newMenueStates[0]&&(currTime-menueLastPulses[0])>100){
    if(newMenueStates[0]==1){
      if(menue==1){
        //Switch to Groups
        if(selector==0){
          selector=5;
          drawOutput(0);
          drawOutput(5);
        }else{
          selector--;
          //On Groups
          drawOutput(selector+1);
          drawOutput(selector);
        }
      }else if(menue==2){
        if(selector==0){
          selector=5;
          drawCheckBox(5,false);
          drawCheckBox(0,false);
        }else{
          selector--;
          drawCheckBox(selector+1,false);
          drawCheckBox(selector,false);
        }
      }
    }
    menueStates[0]=newMenueStates[0];
    menueLastPulses[0]=currTime;
    
  }//---------------------Next------------------------
  if(menueStates[1]!=newMenueStates[1]&&(currTime-menueLastPulses[1])>100){
    if(newMenueStates[1]==1){
      if(menue==1){
        //Switch to Outputs
        if(selector==5){
          selector=0;
          drawOutput(0);
          drawOutput(5);
        }else{
          selector++;
          drawOutput(selector-1);
          drawOutput(selector);
          
        }
        
      }else if(menue==2){
        if(selector==5){
          selector=0;
          drawCheckBox(5,false);
          drawCheckBox(0,false);
        }else{
          selector++;
          drawCheckBox(selector-1,false);
          drawCheckBox(selector,false);
        }
      }
    }
    menueStates[1]=newMenueStates[1];
    menueLastPulses[1]=currTime;
  }
  //---------------------Exit-----------------------
  if(menueStates[2]!=newMenueStates[2]&&(currTime-menueLastPulses[2])>100){
    if(newMenueStates[2]==1){
      if(menue==1){
        
      }else if(menue==2){
        selector=0;
        drawMenueOne();
        menue=1;
      }
    }

    menueStates[2]=newMenueStates[2];
    menueLastPulses[2]=currTime;
  }
  //----------------------Select----------------------
  if(menueStates[3]!=newMenueStates[3]&&(currTime-menueLastPulses[3])>100){
    if(newMenueStates[3]==1){
      //In menue 1
      if(menue==1){
        //On Outs
        selected=selector;
        selector=0;
        drawMenueTwo();
        menue=2;
      }else if(menue==2){
        if(selector==4){
          servoInverted[selected]=!servoInverted[selected];
          drawContent(4);
        }else{
          
        }
      }
      
    }
    
    menueStates[3]=newMenueStates[3];
    menueLastPulses[3]=currTime;
  }

}

void drawOutput(int num){
    int xCord=1+num*16;
    if(num>2){
      xCord+=32;
    }
    //Clear Display Area
    display.fillRect(xCord-1, 0, 16, 16,BLACK);
    
    display.drawRect(xCord, 1, 14, 14,WHITE);
    int bindedTo=bind[num];
    if(num==selector){
      //display.drawRect(xCord+1,2,12,12,WHITE);
      display.drawRect(xCord-1,0,16,16,WHITE);
    }
    if(bindedTo==1){
      display.fillRect(xCord+2, 3, 5, 5,WHITE);
    }else if(bindedTo==2){
      display.fillRect(xCord+7, 3, 5, 5,WHITE);
    }else if(bindedTo==3){
      display.fillRect(xCord+2, 8, 5, 5,WHITE);
    }else if(bindedTo==4){
      display.fillRect(xCord+7, 8, 5, 5,WHITE);
    }
    
    display.display();
    
  }

 void drawGroup(int num){
  
   int yCord = 16;
   int xCord = 56;
   if(num>1){
    yCord=40;
   }
   if(num==1||num==3){
    xCord=65;
   }
   //Clear Display Area
    display.fillRect(xCord-1, yCord-1, 9, 25,BLACK);

    display.drawRect(xCord, yCord, 7, 23,WHITE);
    
    if(selector-6==num){
      display.drawRect(xCord-1, yCord-1, 9, 25,WHITE);
    }
    bool active = !toggleState1;
    if(num==1){
      active=!toggleState2;
    }else if(num==2){
      active=toggleState1;
    }else if(num==3){
      active=toggleState2;
    }
    if(active){
      display.fillRect(xCord+2, yCord+2, 3, 19,WHITE);
    }
    display.display();
 }
  void drawVal(int num){
    int xCord=12;
    int yCord=20;
    if(num>1){
      yCord=45;
    }
    if(num==1||num==3){
      xCord = 92;
    }
    //clear area of screen
    int clearX = 40;
    if(counts[num]>99){
      xCord-=5;
      clearX+=5;
    }else if(counts[num]<10){
      xCord+=5;
      clearX-=5;
    }
    display.fillRect(xCord-10,yCord,clearX,15,BLACK);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(xCord, yCord);
    display.println(counts[num]);
    display.display();
 }

 void drawMenueOne(){
  display.clearDisplay();
  for(int i=0;i<6;i++){
    drawOutput(i);
  }
  for(int i=0;i<4;i++){
    drawGroup(i);
    drawVal(i);
  }
 }

 void drawCheckBox(int num,bool checked){
   int xCord=1;
   int yCord=18+num*12;
   if(num>3){
      xCord=64;
      yCord=18+(num-4)*12;
   }
   //clear Area of Screen;
   display.fillRect(xCord-1,yCord-1,12,12,BLACK);

   display.drawRect(xCord,yCord,10,10,WHITE);
   if(selector==num){
    display.drawRect(xCord-1,yCord-1,12,12,WHITE);
   }
   if(checked){
    display.drawRect(xCord+2,yCord+2,8,8,WHITE);
   }
   display.display();
   
 }
 void drawContent(int num){
   int xCord=14;
   int yCord=18+num*12;
   if(num>3){
      xCord=77;
      yCord=18+(num-4)*12;
   }
   display.fillRect(xCord-1,yCord-1,45,12,BLACK);
   switch(num){
    //max
    case 0:
      //Icon
      display.drawLine(xCord,yCord,xCord+10,yCord,WHITE);
      display.drawLine(xCord+4,yCord+1,xCord+4,yCord+10,WHITE);
      display.drawLine(xCord+5,yCord+1,xCord+5,yCord+10,WHITE);

      display.setTextSize(1);
      display.setCursor(xCord+14,yCord+1);
      display.println(servoMax[selected]);
    break;
    //min
    case 1:
    //Icon
      display.drawLine(xCord,yCord+10,xCord+10,yCord+10,WHITE);
      display.drawLine(xCord+4,yCord,xCord+4,yCord+9,WHITE);
      display.drawLine(xCord+5,yCord,xCord+5,yCord+9,WHITE);
      
      display.setTextSize(1);
      display.setCursor(xCord+14,yCord+1);
      display.println(servoMin[selected]);
    break;
    //offset
    case 2:
       //Icon
      display.drawLine(xCord,yCord,xCord,yCord+10,WHITE);
      display.drawLine(xCord+1,yCord+4,xCord+10,yCord+4,WHITE);
      display.drawLine(xCord+1,yCord+5,xCord+10,yCord+5,WHITE);

      display.setTextSize(1);
      display.setCursor(xCord+14,yCord+1);
      display.println(servoOffset[selected]);
    break;
    //midpoint
    case 3:
      //Icon
      display.drawLine(xCord+4,yCord,xCord+4,yCord+10,WHITE);
      display.drawLine(xCord,yCord+4,xCord+10,yCord+4,WHITE);
      display.drawLine(xCord,yCord+5,xCord+10,yCord+5,WHITE);

      display.setTextSize(1);
      display.setCursor(xCord+14,yCord+1);
      display.println(servoMidpoint[selected]);
    break;
    //inverted
    case 4:
      //Icon
      display.drawRect(xCord,yCord,10,10,WHITE);
      display.drawRect(xCord+4,yCord+4,2,2,WHITE);
      
      display.setTextSize(1);
      display.setCursor(xCord+14,yCord+1);
      if(servoInverted[selected]){
        display.println("True");
      }else{
        display.println("False");
      }
      
    break;
    //Group
    case 5:
      display.setTextSize(1);
      display.setCursor(xCord+2,yCord+1);
      display.print("Group ");display.print(bind[selected]+1);
    break;
   }
   display.display();
 }

 void drawMenueTwo(){
    display.clearDisplay();
    display.setCursor(34,0);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.print("Out ");
    display.print(selected+1);
    display.display();

    for(int i=0;i<6;i++){
      drawCheckBox(i,false);
      drawContent(i);
    }
 }


 void updateServoGroup(int num){
    for(int i=0;i<6;i++){
      if(bind[i]==num+1){
        servos[i].write(counts[num]);
        Serial.print("Updated Servo ");Serial.println(i+1);
      }
    }
    Serial.print("Updated Servo Group");Serial.println(num+1);
 }
