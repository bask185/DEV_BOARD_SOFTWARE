#include "src/macros.h"
#include "src/TSCbus.h"
#include "src/BOARDS.h"

/*
thoughts:
    Needs lots of testing
    the method 'getMessage()'
    should be relocated to the sub classes. The method
    must manage the different package types.
    - config messages
    - IO scan
    - set outputs
    - get inputs

    also.
    revisit the OPCODEs
    walk through message sizes. 4 bits for lengths seem redundant
    I may use less bits for length and pack every step per 2 bytes
    so ob01 = 2 byte and 0b10 = 4 byte

    I would also like to set a certain bit of OPCODEs when 'it' is concerning an INPUT
    this way you can read this specific bit to test whether we must read or write things
    for instance. Slaves which need to send inputs cannot do a checksum. Instead they must make one
    whether it is a digital, analog or encoder readout.

    I was thinking to be able to send more than one message to a slave per package.
    This would still allow for combined input/output transmissions without losing checksums

    On the other hand. Input readouts should be default and setting outputs is occasional.
    You don't need to contiously send output messages.

    setting a single device per message is unwise. If you would call something like
    slave1.setOutput(2, HIGH) ;
    slave1,setOutput(3, HIGH) ;
    twice in a single loop() cycle. The first message would be overwritten by the second.
    
    Therefor if an output is to be set, the message should contain all output data.
    If the OPCODE is loaded by a set output, that OPCODE should be transmitted.
    If NO OPCODE is loaded, input or idle transmissions should be sent.

    stepper motor board would need to send 3 input bytes per motor. 2 bytes (or 4)
    for position status and a single status byte.
*/

static        ARDUINO_BOARD      slave1 = ARDUINO_BOARD() ; 
static        ARDUINO_BOARD      slave2 = ARDUINO_BOARD() ;
// static        ENCODER_BOARD    encoder1 =        ENCODER_BOARD() ;
// static   ANALOG_INPUT_BOARD        ADC1 =   ANALOG_INPUT_BOARD() ;
// static  ANALOG_OUTPUT_BOARD        DAC1 =  ANALOG_OUTPUT_BOARD() ;
// static  DIGITAL_INPUT_BOARD  digitalIn1 =  DIGITAL_INPUT_BOARD() ;
// static  STEPPER_MOTOR_BOARD        SMM1 =  STEPPER_MOTOR_BOARD() ;
// static DIGITAL_OUTPUT_BOARD digitalOut1 = DIGITAL_OUTPUT_BOARD() ;

BOARD *board[] =
{
    &slave1 ,
    &slave2 , 
} ;

TSCbus bus(MASTER) ;

// copy from board object to bus handler
void notifyloadMessage( Message *message, uint8 slaveIndex )
{
    *message = *board[slaveIndex]->getMessage() ;
}

// copy from bus handler to board object
void notifyRelayInputs( Message *message, uint8 slaveIndex )
{
    board[slaveIndex]->relayMessage( message ) ;
}

void setup()
{
    Serial.begin( 115200 ) ;

    slave1.configurePins(  ) ;       // prepare OPCODE
    slave1.pinMode( 2, INPUT ) ; // fill payload with IO configuration
    slave1.pinMode( 3, INPUT ) ;
    slave1.pinMode( 4, INPUT ) ;
    slave1.pinMode( 5, INPUT ) ;

    slave1.pinMode( 6, OUTPUT ) ;
    slave1.pinMode( 7, OUTPUT ) ;
    slave1.pinMode( 8, OUTPUT ) ;
    slave1.pinMode( 9, OUTPUT ) ;

    slave2.configurePins(  ) ;
    slave2.pinMode( 2, INPUT ) ;
    slave2.pinMode( 3, INPUT ) ;
    slave2.pinMode( 4, INPUT ) ;
    slave2.pinMode( 5, INPUT ) ;

    slave2.pinMode( 6, OUTPUT ) ;
    slave2.pinMode( 7, OUTPUT ) ;
    slave2.pinMode( 8, OUTPUT ) ;
    slave2.pinMode( 9, OUTPUT ) ;

    while( !bus.drive() ) ; // wait for configuration messages are sent and received
}

void loop()
{
    bus.drive() ;

    slave2.setOutput( 6, !slave1.getInput( 2 ) ) ;
    slave2.setOutput( 7, !slave1.getInput( 3 ) ) ;
    slave2.setOutput( 8, !slave1.getInput( 4 ) ) ;
    slave2.setOutput( 9, !slave1.getInput( 5 ) ) ;

    slave1.setOutput( 6, !slave2.getInput( 2 ) ) ;
    slave1.setOutput( 7, !slave2.getInput( 3 ) ) ;
    slave1.setOutput( 8, !slave2.getInput( 4 ) ) ;
    slave1.setOutput( 9, !slave2.getInput( 5 ) ) ;
}
