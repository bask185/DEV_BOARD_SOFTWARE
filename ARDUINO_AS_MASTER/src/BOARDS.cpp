#include "BOARDS.h"


/* jack of all trade message
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

Message* BOARD::getMessage() // make pointer or so
{
    return &message ;
}

void BOARD::relayMessage( Message* newMes )
{
    message = *newMes ; // this update the payload of 
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

ARDUINO_BOARD::ARDUINO_BOARD()
{

}
void ARDUINO_BOARD::configurePins()
{
    message.OPCODE = OPC_CONF_IO ; // may be done in the constructor
    message.length = 1 ;
}

void ARDUINO_BOARD::pinMode( uint8 pin, uint8 mode )
{
    message.payload[ message.length ++ ] =  pin ;
    message.payload[ message.length ++ ] = mode ;
    message.payload[0] = message.length ; // the first byte after the OPCODE carries message size
}

uint8 ARDUINO_BOARD::getInput(  uint8 pin  ) 
{
    // NOTE: if the pin is configured as input pin
    // updates will be received automatically
    // instead call back functions ae invoked to inform application code that an input is received.
    
}

uint16 ARDUINO_BOARD::getAnalogInput(  uint8 pin ) 
{
    // NOTE: analog readouts are performed continously, 
    // if the pin is configured as analog input. Update will be received automatically
}

void ARDUINO_BOARD::setOutput( uint8 pin, uint8 val ) 
{
    //message.OPCODE      = OPC_SET_OUTPUT ;
    message.payload[0]  = pin ;
    message.payload[1]  = val ;
}

void ARDUINO_BOARD::setServo( uint8 pin, uint8 val ) 
{
    val = constrain( val, 0 , 180 ) ;

    //message.OPCODE      = OPC_SET_SERVO ;
    message.payload[0]  = pin ;
    message.payload[1]  = val ;
} 

void ARDUINO_BOARD::setPWM( uint8 pin, uint8 val ) 
{
    //message.OPCODE      = OPC_SET_PWM ;
    message.payload[0]  = pin ;
    message.payload[1]  = val ;
}



