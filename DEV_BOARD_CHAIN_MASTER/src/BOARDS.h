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
/* byte 0 *     uint8 PWM1 ;   // obsolete, values are processed to analogWrite after transceiving   */
/* byte 1 *     uint8 PWM2 ; */
/* byte 2 *     uint8 PWM3 ; */
/* byte 3 *     uint8 outputs ; // SSSSRRRR  Servo1-4 Relay */

// INPUTS  (all are used)
/* byte 4 * /    uint8 switches ;// xxxxSSSS  (sw4-Sw1)
/* byte 5 * /    uint8 pot1HB ;  // xxxxxxPP
/* byte 6 * /    uint8 pot1LB ;  // PPPPPPPP
/* byte 7 * /    uint8 pot2HB ;  // xxxxxxPP
/* byte 8 * /    uint8 pot2LB ;  // PPPPPPPP */

class BOARD
{
public:
    Message getMessage() ;

private:
    Message message ; 
}

class STEPPER_MOTOR_BOARD public BOARD
{
public:
    /* constructor */
    // add public messages to set and write to message payloads
private:
};

class DIGITAL_INPUT_BOARD public BOARD
{
public:
    /* constructor */
    // add public messages to set and write to message payloads
private:
};

class DIGITAL_OUTPUT_BOARD public BOARD
{
public:
    /* constructor */
    // add public messages to set and write to message payloads
private:
};

class ANALOG_INPUT_BOARD public BOARD
{
public:
    /* constructor */
    // add public messages to set and write to message payloads
private:
};

class ENCODER_BOARD public BOARD
{
public:
    /* constructor */
    // add public messages to set and write to message payloads
private:
};



class JACK_OF_ALL_TRADES public BOARD //
{
public:
    JACK_OF_ALL_TRADES() ;

    void    setServo( uint8, uint8 ) ; // set 'outputs' variable correctly
    void    setRelay( uint8, uint8 ) ; // set 'outputs' variable correctly
    
    void    setPWM( uint8, uint8 ) ;   // called from package manager in order to update the value
    uint8   getPWM( uint8 ) ;          // can be called from application to load the value and do something with it.

    
private:
    
} ;