#include "BOARDS.h"


// NOTE, all all board info here?
/*
 0   uint8   PWM1 ;
 1   uint8   PWM2 ;
 2   uint8   PWM3 ;
 3   uint8   outputs ;
 4   uint8   switches ;
 5   uint8   pot1HB ;
 6   uint8   pot1LB ;
 7   uint8   pot2HB ;
 8   uint8   pot2LB ;
*/

Message BOARD::getMessage() // make pointer or so
{
    return message ;
}



JACK_OF_ALL_TRADES::JACK_OF_ALL_TRADES()
{
    message.OPCODE = OPC_SET_DATA ;
    message.length = 10 ; // 9 payload bytes + OPC
}

void  JACK_OF_ALL_TRADES::setServo( uint8 index, uint8 state )  // set 'outputs' variable correctly (may want to change indices to correspond with pinnumber instead?)
{
    if( index > 3 ) return ;

    if( state ) message.payload[3] |=  (1 << (7-index)) ;    // SSSSRRRR 
    else        message.payload[3] &= ~(1 << (7-index)) ;
}

void  JACK_OF_ALL_TRADES::setRelay( uint8 index, uint8 state )  // set 'outputs' variable (may want to change indices to correspond with pinnumber instead?)
{
    if( index > 3 ) return ;

    if( state ) message.payload[3] |=  (1 << (3-index)) ;    // SSSSRRRR 
    else        message.payload[3] &= ~(1 << (3-index)) ;
}

void  JACK_OF_ALL_TRADES::setPWM( uint8 index, uint8 val )    // called from package manager in order to update the value (may want to change indices to correspond with pinnumber instead?)
{
    if( index > 2 ) return ;
    
    if( index == 0 ) message.payload[0] = val ; 
    if( index == 1 ) message.payload[1] = val ;
    if( index == 2 ) message.payload[2] = val ;
}

uint8 JACK_OF_ALL_TRADES::getPWM( uint8 index )           // can be called from application to load the value and do something with it.
{
    if( index > 2 ) return ;

    if( index == 0 ) return message.payload[0] ; 
    if( index == 1 ) return message.payload[1] ; 
    if( index == 2 ) return message.payload[2] ; 

    return 0 ;
}

uint8 ARDUINO_BOARD::ARDUINO_BOARD()
{
    void ARDUINO_BOARD::setOutput( uint8 pin, uint8 val ) 
    {
        pinMode( pin, OUTPUT ) ;
        digitalWrite( pin, val ) ;
    }

    uint8 ARDUINO_BOARD::getInput(  uint8 pin  ) 
    {
        pinMode( pin, INPUT_PULLUP ) ;
        return digitalRead( pin ) ; // N.B. may want to change this to payload exchange
    }

    uint16 ARDUINO_BOARD::getAnalogInput(  uint8 pin ) 
    {
        pinMode( pin, INPUT );
        return analogRead( pin ) ;
    }

    void ARDUINO_BOARD::getPin( uint8 pin ) 
    {
        for( int i = 0 ; i < 12 ; i ++ )
        {
            if( servoIndex[i] == 255 ) // free servo object
            {
                servoIndex[i] = pin ;
                servo[i].attach( pin ) ;
            }
            if( servoIndex[i] == pin ) return i ;
        }
        return 255 ; // no more available pins ;
    }

    void ARDUINO_BOARD::setServo( uint8 pin, uint8 val ) 
    {
        uint8 index = getPin( pin ) ;   // find which servo object belongs to this pin, otherwise assigns and attach one
        val = constrain( val, 0 , 180 ) ;
        servo[index].write( val ) ;
    } 

    void ARDUINO_BOARD::setPWM( uint8 pin, uint8 val ) 
    {
        analogWrite( pin, val ) ;
    }
}


