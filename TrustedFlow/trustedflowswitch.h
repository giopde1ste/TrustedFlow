#ifndef TRUSTEDFLOWSWITCH_H
#define TRUSTEDFLOWSWITCH_H
#include "arduino.h"

#define INLED1 A5 // LEFT
#define INLED2 A4 // TURNING
#define INLED3 A3 // RIGHT

#define LEFT 0
#define RIGHT 1

#define COUNTER 0
#define PREVCOUNTER 1
#define CHECKTIME 1000

#define SSLOW 60
#define SNORMAL 70
#define SFAST 200
#define SMAX 255

class TrustedFlowSwitch
{
public:
   //ini
   TrustedFlowSwitch(int motor1, int motor2, int enable, int positie);
   int setupHall();

   //connect to external ISR
   void hallCounter();

   //turn
   void turn(int direction, int pwm);

   //turn_variation
   bool turnHall(int direction);

   //Middle turns
   bool SwitchFromMiddle(int direction);
   bool SwitchToMiddle(int direction);

   //stop
   void stopPWM();

   //get
   int getHall();

private:
   int _motor1=0;
   int _motor2=0;
   int _enable=0;
   int _hall1=0;
   int _hall2=0;
   int _positie=0;
   unsigned long oldTime=0;
   unsigned long oldTime1=0;

   bool _print[5] = {true,true,true,true,true};
   int _hallstate[2] = {0,0};
   int _pwm = 0;
   int _distance = 0;
   int _pos = 0;
   int _zero = 0;
   int _State = 0;
   int _OldDirection;
};

#endif // TRUSTEDFLOWSWITCH_H
