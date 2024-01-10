#include <Arduino.h>
#include <SoftwareSerial.h>
#include "macros.h"

#ifndef MESSAGE_H
#define MESSAGE_H

enum Commands
{
    // 0 is to be discarded
    // configuring
    OPCconfServo = 1,      // 1
    OPCcoupleOutput,       // 2
    OPCcoupleInput,        // 3
    OPCcoupleCoilDrive,    // 4

    // setting outputs
    OPCsetOutput,          // 5
    OPCsetServo,           // 6
    OPCsetPwm,             // 7
    OPCsetCoil,            // 8

    // sending inputs
    OPCsendInput,          // 9 
} ;

extern uint8 receiveMessage() ;

extern void setup232( HardwareSerial&, uint32 ) ;
extern void setup232( SoftwareSerial&, uint32 ) ;

extern void sendMessage( uint8*, uint8 ) ;
extern void sendOutput(  uint8 , uint8 ) ;
extern void sendServo(   uint8 , uint8 ) ;
extern void sendPWM(     uint8 , uint8 ) ;
extern void sendInput(   uint8 , uint8 ) ;

extern void processMessage() ;

extern void notifySetMode(           uint8      mode ) __attribute__ ((weak)) ;
extern void notifyConfServo(         uint8     state ) __attribute__ ((weak)) ;
extern void notifyOutput( uint8 pin, uint8     state ) __attribute__ ((weak)) ;
extern void notifyServo(  uint8 obj, uint8     state ) __attribute__ ((weak)) ;
extern void notifyPWM(    uint8 pin, uint8 dutyCycle ) __attribute__ ((weak)) ;
// todo, add configure servo command

#endif