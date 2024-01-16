#include <Arduino.h>
#include "macros.h"
#include "TSCbus.h"
/*
    this class is ment to control IO with. 
    I think the class should interface between control functions and the packet manager.
    The class both sets outputs as well as reading in inputs of slaves.

    I am not sure if this class is needed or something. The class does relate to one full message of slave
    
    In application code I want to set outputs and read data from messages. So the appriopiate message should be relayed to the corresponding object.
    setting outputs comes from I2C extenders. But that is not relevant here.
    
    The packet manager should request this class how to fill a message.

    I would also like to make a more generic class from which I can inherit. That would allow me to create more type of boards and stuff them in arrays

message payload;
// OUTPUTS
/* OPC_SET_DATA
/* byte 0 *     uint8 PWM1 ;   // obsolete, values are processed to analogWrite after transceiving   */
/* byte 1 *     uint8 PWM2 ; */
/* byte 2 *     uint8 PWM3 ; */
/* byte 3 *     uint8 outputs ; // SSSSRRRR  Servo1-4 Relay */

// INPUTS  (all are used)
/* OPC_GET_DATA
/* byte 0 * /    uint8 switches ;// xxxxSSSS  (sw4-Sw1)
/* byte 1 * /    uint8 pot1HB ;  // xxxxxxPP
/* byte 2 * /    uint8 pot1LB ;  // PPPPPPPP
/* byte 3 * /    uint8 pot2HB ;  // xxxxxxPP
/* byte 4 * /    uint8 pot2LB ;  // PPPPPPPP */

class BOARD
{
public:
    // Message* getMessage() ; virtual needed?
    // void relayMessage( Message* ) ; virtual needed?
    Message  message ; 

private:
} ;

class STEPPER_MOTOR_BOARD: public BOARD
{
public:
    /* constructor */
    // add public messages to set and write to message payloads
private:
};

class DIGITAL_INPUT_BOARD: public BOARD
{
public:
    DIGITAL_INPUT_BOARD() ;

    void        relayMessage( Message* newMes )
    Message*    getMessage() ;
    
    void        setConfig( uint8, uint8 ) ;
    uint8       getInput( uint8 ) ;

private:
    uint16      inputs ;
    uint16      configuration ;
    uint8       updateDue = 0 ;
};

// <OPC_SET_OUTPUT> <DATA1> <DATA2> <CHECKSUM>
class DIGITAL_OUTPUT_BOARD: public BOARD
{
public:
    DIGITAL_OUTPUT_BOARD() ;

    Message*    getMessage() ;
    void        setOuput( uint8, uint8 ) ;
    uint8       getOutput( uint8 ) ;

private:
    uint16      outputs ;
    uint8       updateDue = 0 ;
};

class ANALOG_INPUT_BOARD: public BOARD
{
public:
    /* constructor */
    // add public messages to set and write to message payloads
private:
};

class ANALOG_OUTPUT_BOARD: public BOARD
{
public:
    /* constructor */
    // add public messages to set and write to message payloads
private:
};

class ENCODER_BOARD: public BOARD
{
public:
    /* constructor */
    // add public messages to set and write to message payloads
private:
};


class JACK_OF_ALL_TRADES: public BOARD //
{
public:
    JACK_OF_ALL_TRADES() ;

    void    setServo( uint8, uint8 ) ; // set 'outputs' variable correctly
    void    setRelay( uint8, uint8 ) ; // set 'outputs' variable correctly    
    void    setPWM(   uint8, uint8 ) ;   // called from package manager in order to update the value

    uint16  getADC( uint8 ) ;
    uint8   getPWM(   uint8 ) ;
    uint8   getRelay( uint8 ) ;
    uint8   getServo( uint8 ) ;

private:
    uint8   pwm[3] ;
    uint16  adc[2] ;
    uint8   outputs ; //SSSS RRRR
} ;

class ARDUINO_BOARD: public BOARD
{
public:
    ARDUINO_BOARD() ;
    // note a manager should be used in order to update input pins on a regular basis
    void    configurePins() ; // may be done in the constructor?
    void    pinMode( uint8 pin, uint8 mode ) ;
    void    setOutput( uint8 pin, uint8 val ) ;
    uint8   getInput(  uint8 pin  ) ;
    uint16  getAnalogInput(  uint8 pin ) ;
    void    setServo( uint8 pin, uint8 val ) ;
    void    setPWM(   uint8 pin, uint8 val ) ;

private:
    void getPin( uint8 pin ) ;
} ;