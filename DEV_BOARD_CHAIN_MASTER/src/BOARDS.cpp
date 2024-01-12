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

