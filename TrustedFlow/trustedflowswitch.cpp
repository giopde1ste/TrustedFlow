#include "trustedflowswitch.h"

TrustedFlowSwitch::TrustedFlowSwitch(int motor1, int motor2, int enable, int positie){
   _motor1=motor1;
   _motor2=motor2;
   _enable=enable;
   _positie=positie;
   if(_motor1!=0 && _motor2!=0 && _enable!=0 && _positie!=0){
      pinMode(_motor1,OUTPUT);
      pinMode(_motor2,OUTPUT);
      pinMode(_enable,OUTPUT);
      pinMode(_positie,INPUT);
      digitalWrite(_enable,LOW);
   }
}

int TrustedFlowSwitch::setupHall(){
   _pwm = SMAX;
   while(_zero!=2){
      switch (_zero){
         case 0:
            turn(LEFT,_pwm);
            if(_hallstate[COUNTER] > 1){
               _pwm = SNORMAL;
               Serial.print("_hallstate[COUNTER] = ");
               Serial.println(_hallstate[COUNTER]);
            }
            if(_hallstate[COUNTER] != _hallstate[PREVCOUNTER]){
               _hallstate[PREVCOUNTER] = _hallstate[COUNTER];
               oldTime = millis();
            }
            if(((millis() - oldTime) > CHECKTIME)/* && _hallstate[COUNTER] > 5*/){//timecheck
               stopPWM();
               _zero = 1;
               _pwm = SMAX;
               _hallstate[COUNTER] = 0;
               _hallstate[PREVCOUNTER] = 0;
               Serial.println("case 0: END");
            }
            break;
         case 1:
            turn(RIGHT,_pwm);
            if(_hallstate[COUNTER] != _hallstate[PREVCOUNTER]){
               _hallstate[PREVCOUNTER] = _hallstate[COUNTER];
               oldTime = millis();
               Serial.print("case 1: COUNTER = ");
               Serial.println(_hallstate[COUNTER]);
               if(_print[1]){
                  Serial.println("case 1: COUNTER != PREVCOUNTER");
                  _print[1] = false;
               }
            }
            if(_hallstate[COUNTER] > 1){
               _pwm = SNORMAL;
               if(_print[2]){
                  Serial.println("case 1: COUNTER > 1");
                  _print[2] = false;
               }
            }
            if((millis() - oldTime) > CHECKTIME && _hallstate[COUNTER] > 5){
               _zero = 2;
               _distance = _hallstate[COUNTER];
               Serial.print("case 1: _distance = ");
               Serial.println(_distance);
               Serial.println("case 1: END");
            }
            break;
         default:
            Serial.println("ERROR: unknown value for _zero");//error
            break;
      }
   }
   stopPWM();
   return RIGHT;
}

void TrustedFlowSwitch::hallCounter(){
   _hallstate[COUNTER]++;
}

void TrustedFlowSwitch::turn(int direction, int pwm){
   if(direction==LEFT){
      digitalWrite(_motor1,HIGH);
      digitalWrite(_motor2,LOW);
      analogWrite(_enable,pwm);
   }
   if(direction==RIGHT){
      digitalWrite(_motor1,LOW);
      digitalWrite(_motor2,HIGH);
      analogWrite(_enable,pwm);\
   }
}

bool TrustedFlowSwitch::turnHall(int direction){
   _hallstate[COUNTER] = 0;
   int Procentdone = 0;
   oldTime1 = 0;
   _State = 0;
   digitalWrite(INLED1, LOW);
   digitalWrite(INLED2, HIGH);
   digitalWrite(INLED3, LOW);
   while(true){
      if(_hallstate[COUNTER] <= _distance - 1){
         if((100*_hallstate[COUNTER]/_distance) >= 40 && _State == 0){
            _State = 1;
            Serial.print("_State = ");
            Serial.println(_State);
            oldTime1 = millis();
         }
         if(_pwm <= SSLOW && _State == 1){
            _State = 2;
            Serial.print("_State = ");
            Serial.println(_State);
            oldTime1 = millis();
         }
         switch(_State){
            case 0:
               _pwm = SMAX;
               break;
            case 1:
               _pwm = -0.1*(millis()-oldTime1)+SMAX;
               break;
            case 2:
               _pwm = SSLOW;
               break;
            default:
               Serial.println("ERROR: unknown _State");
               break;
         }
         turn(direction,_pwm);
         Serial.print("hall turn ");
         Procentdone = (100*_hallstate[COUNTER]/_distance);
         Serial.print(Procentdone);
         Serial.println("%");
         Serial.print(_hallstate[COUNTER]);
         Serial.print("/");
         Serial.println(_distance);
         if(Procentdone >= 90){
            if(_hallstate[COUNTER] != _hallstate[PREVCOUNTER]){
               _hallstate[PREVCOUNTER] = _hallstate[COUNTER];
               oldTime = millis();
            }

            if(((millis() - oldTime) > CHECKTIME)){//timecheck
               stopPWM();
               digitalWrite(INLED2, LOW);
               _hallstate[PREVCOUNTER] = 0;
               Serial.println("turnHall ERROR: Stopped before end");
               return false;
            }
         }
         if(Procentdone % 2){
            digitalWrite(INLED2, HIGH);
         }else{
            digitalWrite(INLED2, LOW);
         }
      }else{
         stopPWM();
         digitalWrite(INLED2, LOW);
         return true;
      }
   }
}

bool TrustedFlowSwitch::SwitchToMiddle(int direction){
   _pwm = SMAX;
   _hallstate[COUNTER] = 0;
   turn(direction,_pwm);
   _OldDirection = direction;
   Serial.print("SwitchToMiddle: turn started, _OldDirection = ");
   Serial.println(_OldDirection);
   while(true){
      //Serial.print("SwitchToMiddle: Wait for turn complete, hallcounter = ");
      //Serial.println(_hallstate[COUNTER]);
      delay(1);
      if(_distance/2 <= _hallstate[COUNTER]){
         stopPWM();
         Serial.println("SwitchToMiddle: SUCCES");
         return true;
      }
   }
}

bool TrustedFlowSwitch::SwitchFromMiddle(int direction){
   _pwm = SMAX;
   if(_OldDirection != direction){
      Serial.println("SwitchFromMiddle: Direction switch");
      _hallstate[COUNTER] = _distance - _hallstate[COUNTER];
   }
   Serial.print("Olddirection = ");
   Serial.println(_OldDirection);
   Serial.print("Newdirection = ");
   Serial.println(direction);
   turn(direction,_pwm);
   Serial.println("SwitchFromMiddle: turn started");
   while(true){
      //Serial.print("SwitchFromMiddle: Wait for turn complete, hallcounter = ");
      //Serial.println(_hallstate[COUNTER]);
      delay(1);
      if(_hallstate[COUNTER] >= _distance - _distance / 4){ // When the switch is already moving turn the speed back down to SNORMAL
         _pwm = SNORMAL;                                    // "_distance - _distance / 4" is used as a subsitude for "_distance * 0.75" because this will give an error
      }
      if(_hallstate[COUNTER] >= _distance){
         stopPWM();
         Serial.println("SwitchFromMiddle: SUCCES");
         return true;
      }
   }
}

void TrustedFlowSwitch::stopPWM(){
   analogWrite(_enable,0);
}

int TrustedFlowSwitch::getHall(){
   return digitalRead(_positie);
}
