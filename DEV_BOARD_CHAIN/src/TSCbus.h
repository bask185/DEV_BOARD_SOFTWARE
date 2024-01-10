#include <Arduino.h>
// #include <SoftwareSerial.h>
#include "macros.h"

#ifndef MESSAGE_H
#define MESSAGE_H

const int OPC_INIT              = 0x11 ;
const int OPC_IDLE              = 0xF1 ;
const int OPC_SET_OUTPUT        = 0x22 ;
const int OPC_SET_PWM           = 0x33 ;
const int OPC_GET_INPUT         = 0x42 ;
const int OPC_GET_ANALOG        = 0x53 ;
const int OPC_GET_DATA          = 0x61 ; // length is variable
const int OPC_CONF_IO           = 0x62 ;
const int OPC_CONF_SERVO        = 0x76 ;
const int OPC_GET_MODULE_TYPE   = 0x82 ;

extern uint8 receiveMessage() ;

typedef struct Mess
{
    uint8 payload[16] ; // max possible length
    uint8 OPC ;
    uint8 length ;
    uint8 checksum ;
    uint8 index ;
} Message;


class TSCbus
{
public:
    TSCbus();
    void transceiveMessage() ;

private:
    uint8   state ;
    uint8   myID ;
    uint8   byteCounter ;
    uint8   lastOPC ;
    uint8   moduleCount ;
    uint8   messageCounter ;

    uint8   checkChecksum() ;

    Message message ;
};



/*
extern void setup232( HardwareSerial&, uint32 ) ;
extern void setup232( SoftwareSerial&, uint32 ) ;

extern void sendMessage( uint8*, uint8 ) ;
extern void sendOutput(  uint8 , uint8 ) ;
extern void sendServo(   uint8 , uint8 ) ;
extern void sendPWM(     uint8 , uint8 ) ;
extern void sendInput(   uint8 , uint8 ) ;
*/



extern void notifySetMode(           uint8      mode ) __attribute__ ((weak)) ;
extern void notifyConfServo(         uint8     state ) __attribute__ ((weak)) ;
extern void notifyInput(  uint8 pin, uint8     state ) __attribute__ ((weak)) ;
extern void notifyOutput( uint8 pin, uint8     state ) __attribute__ ((weak)) ;
extern void notifyServo(  uint8 obj, uint8     state ) __attribute__ ((weak)) ;
extern void notifyPWM(    uint8 pin, uint8 dutyCycle ) __attribute__ ((weak)) ;
// todo, add configure servo command

#endif