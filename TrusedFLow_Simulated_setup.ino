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
#define FULL 1000
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

void setup(){
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
  Serial.begin(9600);
  Position = LEFT; // Assume after setup switch is set to RIGHT;
}

void loop(){
  Inled(); // Update Indicator LED's to currunt Position
  Bled();
  delay(5); // Implemented for consistency of the if statment
  if(RequestFlow() && !ValidFlow()){ // Only execute code if there is a flow request AND if there is no incoming Flow
    Serial.println("No valid flow");
    SwitchChange();
    Inled();
    Bled();
    delay(750); // wait for physical changes
    bool Success = true;
    if(!ValidFlow()){
      //Success = tryFind(); // Use this line for tryFind();
      Success = false; // Use this line for waitFind();
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
        if(digitalRead(SENSOR1)){
          Position = RIGHT;
        }
        if(digitalRead(SENSOR2)){
          Position = LEFT;
        }
        SwitchChange();
        Success = !Success; // Escape the while() loop
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
  if(Buffer > EMPTY && Buffer <= 333){
    digitalWrite(BLED1, HIGH);
    digitalWrite(BLED2, LOW);
    digitalWrite(BLED3, LOW);
  }
  if(Buffer > 333 && Buffer <= 666){
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

void SwitchChange(){ // Function to switch to the other side
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

bool tryFind(){ // Function to use switch find flow i amount of times
  for(int i = 0; i < TRYTURN; i++){ // try to find flow i times
    SwitchChange();
    Inled();
    Bled();
    delay(750); // Wait for flow changes
    if(ValidFlow()){ // check flow
      return true;
    }
  }
  return false;
}

void waitFind(){
  digitalWrite(INLED1, LOW);
  digitalWrite(INLED3, LOW);
  //Position = MIDDLE;
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
