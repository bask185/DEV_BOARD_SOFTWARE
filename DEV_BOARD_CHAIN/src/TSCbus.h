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
const int OPC_GET_DATA          = 0x60 ; // length is variable
const int OPC_CONF_IO           = 0x60 ; // length is variable
const int OPC_CONF_SERVO        = 0x76 ;
const int OPC_GET_MODULE_TYPE   = 0x82 ;
const int OPC_SET_DATA          = 0x90 ; // length is variable

extern uint8 receiveMessage() ;

typedef struct Mess
{
    uint8 payload[16] ; // maximum length is to be determened
    uint8 length ;
    uint8 OPCODE ;
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

/* 
    wrapper around GPIO to create GPIO objects with 
    every GPIO object has:
    - a pin number on a slave                       : 7 bit
    - a slave ID number to whom it is attached      : 8 bit // 6 slaves suffice? 
    - a 1 bit state (though not for a e.g. stepper) : 1 bit
    - a type such as                                ; x bits (undefined, so we go with 8)'
      * servo obj output
      * digital output
      * digital input
      * analog input
      * PWM output
      * stepper motor ouput
      * JACK OF ALL TRADE, Developer board.
      
    - perhaps also an index for payload byte and bit. This may be dynamic 

NOTE BENE:
    currently I have the idea to make an object per GPIO thingy. I do not know if this is smart to do.
    I will first create a JACK OF ALL TRADE boards object, to see if I can easily control all GPIO.

    I am still als not even sure whether I should use the same library for slave and for master?

    Also. I should make not a class for GPIO but a class for a board such as the JACK OF AL TRADES
    that I could inherrit

*/

class GPIO
{
public:
    //   slave ID  pin   iodir
    GPIO( uint8, uint8, uint8 ) ;
    void    init() ;
    void    set( uint8 ) ;      // set 1 or 0
    void    get() ;             // issues an input read command, not yet sure how to process. Update state in background or callback function?

private:
    uint8   slaveID     : 8 ;
    uint8   pinNumber   : 7 ;
    uint8   state       : 1 ;
    uint8   state_prev  : 1 ; // if state != state_prev an update should be sent. 
    uint8   state       : 1 ;
    uint8   type        : 8 ;
} ;

class DIGITAL_OUT : public GPIO
{

} ;

class DIGITAL_IN : public GPIO
{

} ;

class SERVO : public GPIO
{

} ;

class STEPPER : public GPIO
{

} ;

class ANALOG_IN : public GPIO
{

} ;

class PWM : public GPIO
{

} ;

// ME own custom board 
/*
tasks:
   process output messages
   read inputs and be ready to provide payloads
*/
class JACK_OF_ALL_TRADES
{
public:
    JACK_OF_ALL_TRADES() ; // constructor   any tasks?

// IO CONFIG message
/* byte 0 */    uint8 mode ;       // 1 = servos without frog
                                   // 2 = servos with frog
/* byte 1 */    uint8 servoConf ;  // 1 = decrement last set servo
                                   // 2 = increment last set servo

// IO CONTROL message as follows:
// OUTPUTS
/* byte 0 */    uint8 PWM1 ;                                
/* byte 1 */    uint8 PWM2 ;
/* byte 2 */    uint8 PWM3 ;
/* byte 3 */    uint8 outputs ; // SSSSRRRR  Servo1-4 Relay

// INPUTS
/* byte 4 */    uint8 switches ;// xxxxSSSS  (sw4-Sw1)
/* byte 5 */    uint8 pot1HB ;  // xxxxxxPP
/* byte 6 */    uint8 pot1LB ;  // PPPPPPPP
/* byte 7 */    uint8 pot2HB ;  // xxxxxxPP
/* byte 8 */    uint8 pot2LB ;  // PPPPPPPP
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


extern uint8    notifyGetPayload(  uint8, uint8 ) __attribute__ ((weak)) ;
extern void     notifySetMode(     uint8        ) __attribute__ ((weak)) ;
extern void     notifySetOutput(   uint8, uint8 ) __attribute__ ((weak)) ;
extern void     notifySetPwm(      uint8, uint8 ) __attribute__ ((weak)) ;
extern void     notifySetServo(    uint8, uint8 ) __attribute__ ((weak)) ;
extern void     notifyServoConfig( uint8, uint8 ) __attribute__ ((weak)) ;
extern void     notifySetData(     Message*     ) __attribute__ ((weak)) ; // ptr
// todo, add configure servo command

#endif