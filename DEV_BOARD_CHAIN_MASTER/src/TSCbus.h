#include <Arduino.h>
#include "macros.h"

#ifndef MESSAGE_H
#define MESSAGE_H

const int OPC_INIT              = 0x11 ; // tidy me up please. I essentially only use OPC_SET_DATA so far and a config message so..
const int OPC_IDLE              = 0xF1 ;
const int OPC_SET_OUTPUT        = 0x22 ;
const int OPC_SET_PWM           = 0x33 ;
const int OPC_GET_INPUT         = 0x42 ;
const int OPC_GET_ANALOG        = 0x53 ;
const int OPC_GET_DATA          = 0x60 ; // length is variable
const int OPC_CONF_IO           = 0x60 ; // length is variable
const int OPC_CONF_SERVO        = 0x76 ;
const int OPC_GET_MODULE_TYPE   = 0x82 ;
const int OPC_SET_DATA          = 0x90 ; // length is variable

extern uint8 receiveMessage() ;

typedef struct Mess
{
    uint8 payload[64] ; // maximum length is to be determened
    uint8 length ;
    uint8 OPCODE ;
} Message;




class TSCbus
{
public:
    TSCbus();
    void transceiveMessage() ;
    void drive() ;

private:
    uint8   state           = 0 ; // transceiving
    uint8   transmittState  = 0 ; // transmitting (master)
    uint8   scanningBus ;         // flag to initiate bus scan
    uint8   myID ;
    uint8   byteCounter ;
    uint8   messageCount ;
    uint8   messageCounter ;
    uint8   length ;
    uint8   index ;
    uint8   slaveCount ;
    uint8   slaveCounter ;
    uint8   transmittBuffer[64] ;

    uint8   checkChecksum() ;
    void    processOutputs() ;

    Message message ;
};


extern uint8    notifyGetPayload(  uint8, uint8 ) __attribute__ ((weak)) ;
extern uint8    notifyGetBoardType( )             __attribute__ ((weak)) ;
extern void     notifySetMode(     uint8        ) __attribute__ ((weak)) ;
extern void     notifySetOutput(   uint8, uint8 ) __attribute__ ((weak)) ;
extern void     notifySetPwm(      uint8, uint8 ) __attribute__ ((weak)) ;
extern void     notifySetServo(    uint8, uint8 ) __attribute__ ((weak)) ;
extern void     notifyServoConfig( uint8, uint8 ) __attribute__ ((weak)) ;
extern void     notifySetData(     Message*     ) __attribute__ ((weak)) ;
extern void     notifyConfigPin(   Message*     ) __attribute__ ((weak)) ;

#endif