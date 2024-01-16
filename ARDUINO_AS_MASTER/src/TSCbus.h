#include <Arduino.h>
#include "macros.h"

#ifndef MESSAGE_H
#define MESSAGE_H

const int MASTER = 1 ;
const int SLAVE  = 2 ;

/* MESSAGE LAYOUT:
        HEADER           |                  MESSAGE SLAVE 1               |                  MESSAGE SLAVE X                | etc
<SLAVE ID> <SLAVE COUNT> | <OCPODE> <PAYLOAD_0> .. <PAYLOAD_X> <CHECKSUM> | <OCPODE> <PAYLOAD_0> .. <PAYLOAD_X> <CHECKSUM>  | etc

OPCODE LAYOUT
bit 7 = 1 -> IS INPUT, bit 7 = 0 -> IS OUTPUT
bit 6 - bit 3 -> COMMAND OPCODE
bit 2 - bit 0 -> SIZE
SIZE 0x0 000 = 1 // just OPCCODE
SIZE 0x1 001 = 2
SIZE 0x2 010 = 4
SIZE 0x3 011 = 6
SIZE 0x4 100 = 8
SIZE 0x5 101 = 16
SIZE 0x6 110 = 20
SIZE 0x7 111 = size lays in next byte, can be up to 256

IF OPCODE is ment for INPUTS. all payload bytes must be 0
otherwise for outputs, the payload must be filled.

*/
const int IS_INPUT              = 0x80 ; // mask to check if OPC is for input readout
const int OPC_MASK              = 0xF8 ; // masks the first 5 bits for the OPCODE

const int OPC_IDLE              = 0x00 ; // 0b0 0000 000

const int OPC_SET_DATA          = 0x0F ; // 0b0 0001 111  variable message length
const int OPC_GET_DATA          = 0x8F ; // 0b1 0001 111  variable message length

const int OPC_CONF_IO           = 0x17 ; // 0b0 0010 111  variable message length, may contain all kind of config stuff for steppers, input interrupts, etc.

const int OPC_SET_STEPPER       = 0x1D ; // 0b0 0011 101 length = 4x4 = 16 ;  positions -> send setpoints
const int OPC_GET_STEPPER       = 0x9E ; // 0b1 0011 110 length = 5x4 = 20 ;  positions -> fetch current positions + status

const int OPC_SET_DAC           = 0x24 ; // 0b0 0100 100 length = 4x2 = 8 bytes
const int OPC_GET_ADC           = 0xA4 ; // 0b1 0100 100 length = 4x2 = 8 bytes

const int OPC_SET_ENCODER       = 0x2D ; // 0b0 0101 101 length = 4x4 = 16 bytes  load encoder value (if ever needed, it exists...)
const int OPC_GET_ENCODER       = 0xAD ; // 0b1 0101 101 length = 4x4 = 16 bytes  read encoder values, 4 bytes per encoder.

const int OPC_GET_TYPE          = 0xB1 ; // 0b1 0110 001 length = 2 bytes       // respond with type number
const int OPC_GET_STATUS        = 0xBA ; // 0b1 0111 010 length = 4 bytes       // respond with a status like, MISSED A CHECKSUM followed by which OPCODE went bad. Or more data..

const int OPC_SET_OUTPUTS       = 0x41 ; // 0b0 1000 001 length = 2 bytes       // send 16 bits for outputs
const int OPC_GET_INPUTS        = 0xC1 ; // 0b1 1000 001 length = 2 bytes       // respond 16 bits for inputs

typedef struct Mess
{
    uint8 payload[64] ; // maximum length is to be determened
    uint8 length ;
    uint8 OPCODE ;
    uint8 checksum ;
} Message;




class TSCbus
{
public:
    TSCbus(uint8);
    void transceiveMessage() ;
    uint8 drive() ;

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
    uint8   meMode ; // master or slave
    uint8   slaveCount ;
    uint8   slaveCounter ;
    uint8   transmittBuffer[64] ;

    uint8   checkChecksum() ;
    uint8   assembleChecksum() ;
    void    processOutputs() ;
    void    relayInputs( uint8 ) ;

    Message message ;
};

extern uint8    assembleChecksum( Message *message ) ;

extern uint8    notifyGetPayload(  uint8, uint8     ) __attribute__ ((weak)) ;
extern uint8    notifyGetBoardType( )                 __attribute__ ((weak)) ;
extern void     notifySetMode(     uint8            ) __attribute__ ((weak)) ;
extern void     notifySetOutput(   uint8, uint8     ) __attribute__ ((weak)) ;
extern void     notifySetPwm(      uint8, uint8     ) __attribute__ ((weak)) ;
extern void     notifySetServo(    uint8, uint8     ) __attribute__ ((weak)) ;
extern void     notifyServoConfig( uint8, uint8     ) __attribute__ ((weak)) ;
extern void     notifySetData(     Message*         ) __attribute__ ((weak)) ;
extern void     notifyConfigPin(   Message*         ) __attribute__ ((weak)) ;
extern void     notifyLoadMessage( Message *, uint8 ) __attribute__ ((weak)) ;
extern void     notifyRelayInputs( Message *, uint8 ) __attribute__ ((weak)) ;

#endif