# TrustedFlow
TrusedFLow_Simulated_setup.ino

    This is a program for the arduino micro with the custom shield.
  
    Functions:
    - Simultated flowsensors with switches
    - Simulated buffer with LED's
    - Simulated switch with LED's
    - Serial connection for debugging and seeing error's in the programm
    - Change line 16 #define MOTOR to true for symulation with physical switch, set to false for simulated switch or for when the pysical switch is not connected.
  
    This program is purely meant for develepment and to showcause how the final product will work.
  
trustedflowswitch.cpp & trustedflowswitch.h
  
    This is the library for controlling the motor and hallsensor in the physical switch with the custom shield.
