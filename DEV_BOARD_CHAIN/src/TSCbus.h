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
const int OPC_SET_DATA          = 0x91 ; // length is variable

extern uint8 receiveMessage() ;

typedef struct Mess
{
    uint8 payload[16] ; // maximum length is to be determened
    uint8 length ;
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
    uint8   messageCount ;
    uint8   messageCounter ;
    uint8   length ;
    uint8   index ;

    uint8   checkChecksum() ;
    void    processOutputs() ;

    Message message ;
};

class GPIO
{
public:
    //   slave ID  pin   iodir
    GPIO( uint8, uint8, uint8 ) ;
    void    init() ;
    void    set( uint8 ) ;      // set 1 or 0
    void    get() ;             // issues an input read command, not yet sure how to process. Update state in background or callback function?
    void    setPWM( uint8 ) ;   // sets dutycycle

private:
    uint8   slaveID ;
    uint8   pinNumber ;
    uint8   iodir ;
    uint8   state ; // contains active state (or dutycycle) of GPIO pin
} ;



/*
extern void setup232( HardwareSerial&, uint32 ) ;
extern void setup232( SoftwareSerial&, uint32 ) ;

extern void sendMessage( uint8*, uint8 ) ;
extern void sendOutput(  uint8 , uint8 ) ;
extern void sendServo(   uint8 , uint8 ) ;
extern void sendPWM(     uint8 , uint8 ) ;
extern void sendInput(   uint8 , uint8 ) ;
*/


extern uint8    notifyGetPayload(  uint8, uint8 ) ;
extern void     notifySetMode(     uint8        ) __attribute__ ((weak)) ;
extern void     notifySetOutput(   uint8, uint8 ) __attribute__ ((weak)) ;
extern void     notifySetPwm(      uint8, uint8 ) __attribute__ ((weak)) ;
extern void     notifySetServo(    uint8, uint8 ) __attribute__ ((weak)) ;
extern void     notifyServoConfig( uint8, uint8 ) __attribute__ ((weak)) ;
extern void     notifySetData(     Message      ) __attribute__ ((weak)) ; // ptr
// todo, add configure servo command

#endif