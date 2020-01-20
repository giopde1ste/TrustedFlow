// This code is made for the arduino micro with the puprose of simulating different situation
// This code does not make use of the acual HALL sensor and DC motor.
//
//  Bag   Bag
//  |     |
//  S1    S2
//   \    /
//     SW
//     |
//     S3
//     |
//     out

// Lib for physical motor/switch
#include "trustedflowswitch.h"
#define MOTOR true // set this to false if the motor/switch is not connected

// Pins for hallsensor and motor
#define MOTORP 8
#define MOTORM 7
#define MOTORPWM 6
#define MOTORHALL 3

// Pins for simulated flow sensors
#define SENSOR1 9
#define SENSOR2 10
#define SENSOR3 11

// Pins for simulated hallmotor (indicator LED's)
#define INLED1 A5 // LEFT
#define INLED2 A4 // TURNING
#define INLED3 A3 // RIGHT

// Pins for simulated Buffer (indicator LED's)
#define BLED1 A2  
#define BLED2 A1
#define BLED3 A0

// Constants for buffer
#define EMPTY 0
#define FULL 100
#define BUFFERSPEED 10

// Pin for buzzer
#define BUZZER 2

// LEFT and RIGHT definition for the switch
#define LEFT 0
#define RIGHT 1
#define MIDDLE 2

// Amount of times to switch for tryfind()
#define TRYTURN 1

unsigned long oldTime = 0, oldTime2 = 0, oldTime3 = 0;
int Position, Buffer = 0;
bool OutputTone = false, OutputLED = false;

TrustedFlowSwitch Motor(MOTORP,MOTORM,MOTORPWM,MOTORHALL);

// ---------------- Everything above this comment line should normally be in a header file ----------------

void setup(){
  if(MOTOR){ attachInterrupt(digitalPinToInterrupt(MOTORHALL), hallCounter, FALLING); }
  pinMode(SENSOR1, INPUT);
  pinMode(SENSOR2, INPUT);
  pinMode(SENSOR3, INPUT);
  pinMode(INLED1, OUTPUT);
  pinMode(INLED2, OUTPUT);
  pinMode(INLED3, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BLED1, OUTPUT);
  pinMode(BLED2, OUTPUT);
  pinMode(BLED3, OUTPUT);
  digitalWrite(INLED2, LOW);
  Serial.begin(9600); // Serial connection to pc is only used for debugging
  if(MOTOR){
    Motor.setupHall();
  }else{
    Position = RIGHT; // Assume after setup switch is set to RIGHT;
  }
}

void loop(){
  Inled(); // Update Indicator LED's to currunt Position
  Bled();
  delay(5); // Implemented for consistency of the if statment
  if(RequestFlow() && !ValidFlow()){ // Only execute code if there is a flow request AND if there is no incoming Flow
    Serial.println("No valid flow");
    if(MOTOR){
      MotorChange();
    }else{
      SwitchChange();
    }
    Inled();
    Bled();
    delay(750); // wait for physical changes
    bool Success = true;
    if(!ValidFlow()){
      //Success = tryFind(); // Use this line for tryFind();
      Success = false; // Use this line for waitFind();
      if(MOTOR){
        Serial.println("Reached: waitFind() MOTOR");
        if(Position == RIGHT){
          Motor.SwitchToMiddle(LEFT);
        }else{
          Motor.SwitchToMiddle(RIGHT);
        }
        Position = MIDDLE;
      }
    }
    
    bool printReady = true;
    while(!Success){
      ErrorTone();
      waitFind();
      Bled();
      BufferEmpty();
      if(printReady){
        Serial.println("ERROR: no flow");
        printReady = false;
      }
      if(digitalRead(SENSOR1) || digitalRead(SENSOR2)){ // check for incoming flow
        noTone(BUZZER);
        digitalWrite(INLED2, LOW);
        do{
        if(digitalRead(SENSOR1)){
          if(MOTOR){
            Position = LEFT; // Target position
          }else{
            Position = RIGHT; // Position is set to oppisite of taget so SwitchChange() can be used
          }
        }
        if(digitalRead(SENSOR2)){
          if(MOTOR){
            Position = RIGHT; // Target position
          }else{
            Position = LEFT; // Position is set to oppisite of taget so SwitchChange() can be used
          }
        }
        Serial.println("Check");
        }while(Position == MIDDLE);
        if(MOTOR){
          Motor.SwitchFromMiddle(Position); // TODO: implement SwitchFromMiddle(int Position) for acual switch
        }else{
          SwitchChange();
        }
        if(Position != MIDDLE){
          Success = !Success; // Escape the while() loop
        }else{
          Serial.println("ERROR: position == MIDDLE, expected LEFT or RIGHT");
        }
      }
    }
  }
  
  if(ValidFlow()){
    BufferFill();
  }
}

void Inled(){ // Check for Position and set the LED's accordingly
  if(Position == RIGHT){ // RIGHT
    digitalWrite(INLED3, HIGH);
    digitalWrite(INLED1, LOW);
  }
  if(Position == LEFT){ // LEFT
    digitalWrite(INLED3, LOW);
    digitalWrite(INLED1, HIGH);
  }
  if(Position == MIDDLE){
    digitalWrite(INLED3, LOW);
    digitalWrite(INLED1, LOW);
  }
}

void Bled(){ // Check for simulated Buffer and set the lED's accordingly
  if(Buffer <= EMPTY){
    digitalWrite(BLED1, LOW);
    digitalWrite(BLED2, LOW);
    digitalWrite(BLED3, LOW);
  }
  if(Buffer > EMPTY && Buffer <= 33){
    digitalWrite(BLED1, HIGH);
    digitalWrite(BLED2, LOW);
    digitalWrite(BLED3, LOW);
  }
  if(Buffer > 33 && Buffer <= 66){
    digitalWrite(BLED1, HIGH);
    digitalWrite(BLED2, HIGH);
    digitalWrite(BLED3, LOW);
  }
  if(Buffer > (FULL-1)){
    digitalWrite(BLED1, HIGH);
    digitalWrite(BLED2, HIGH);
    digitalWrite(BLED3, HIGH);
  }
}

void BufferFill(){ // Fill the Buffer with 1
  if(Buffer < FULL && (millis() - oldTime3) > BUFFERSPEED){
    Buffer += 1;
    oldTime3 = millis();
  }
}

void BufferEmpty(){ // Empty the Buffer with 1
  if(Buffer > EMPTY && (millis() - oldTime3) > BUFFERSPEED){
    Buffer -= 1;
    oldTime3 = millis();
  }
}

void SwitchChange(){ // Function to switch to the other side, Normally the lib for the switch would handle this command
  digitalWrite(INLED3, LOW);
  digitalWrite(INLED1, LOW);

  for(int i = 0; i < 10; i++){ // Blink 10 times
    delay(100);
    digitalWrite(INLED2, HIGH);
    delay(100);
    digitalWrite(INLED2, LOW);
    Serial.println("BLINK");
  }
  if(Position == RIGHT){
    Position = LEFT;
  }else{
    Position = RIGHT;
  }
}

void MotorChange(){ // Function to switch to the other side, this fuction used the motor lib
  digitalWrite(INLED3, LOW);
  digitalWrite(INLED1, LOW);
  if(Position == RIGHT){
    Motor.turnHall(LEFT);
    Position = LEFT;
  }else{
    Motor.turnHall(RIGHT);
    Position = RIGHT;
  }
}

bool tryFind(){ // Function to use switch find flow i amount of times
  for(int i = 0; i < TRYTURN; i++){ // try to find flow i times
    if(MOTOR){
      MotorChange();
    }else{
      SwitchChange();
    }
    Inled();
    Bled();
    delay(750); // Wait for flow changes
    if(ValidFlow()){ // check flow
      return true;
    }
  }
  return false;
}

void waitFind(){ // Blink INLED2 when waiting for flow
  digitalWrite(INLED1, LOW);
  digitalWrite(INLED3, LOW);
  if(OutputLED){
    if((millis() - oldTime2) >= 100){
      oldTime2 = millis();
      digitalWrite(INLED2, LOW);
      OutputLED = false;
    }
  }else{
    if((millis() - oldTime2) >= 200){
      oldTime2 = millis();
      digitalWrite(INLED2, HIGH);
      OutputLED = true;
    }
  }
}

bool ValidFlow(){ // Function to determine if there is flow with the position of the switch
  bool isFlow;
  
  if((digitalRead(SENSOR1) && Position == LEFT) || digitalRead(SENSOR2) && Position == RIGHT){
    isFlow = true;
  }else{
    isFlow = false;
  }
  
  return isFlow;
}

bool RequestFlow(){ // Function to determine if the liquid is being drained
  bool isFlow;

  if(digitalRead(SENSOR3)){
    isFlow = true;
  }else{
    isFlow = false;
  }

  return isFlow;
}

void ErrorTone(){ // Function for an error sound
  if(OutputTone){
    if((millis() - oldTime) >= 250){
      oldTime = millis();
      noTone(BUZZER);
      OutputTone = false;
    }
  }else{
    if((millis() - oldTime) >= 500){
      oldTime = millis();
      tone(BUZZER, 440);
      OutputTone = true;
    }
  }
}

void hallCounter(){ // Used with ISR to count the amout of times the hallsensor has a falling edge
  Motor.hallCounter();
}
