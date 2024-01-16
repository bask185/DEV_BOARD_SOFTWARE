#include "BOARDS.h"


/* jack of all trade message
 OPC_SET_DATA
 0   size = 5 ;
 1   PWM1 ;
 2   PWM2 ;
 3   PWM3 ;
 4   outputs ;
CHECKSUM

OPC_GET_DATA
 0    size = 6 ;
 1    switches ;
 2    pot1HB ;
 3    pot1LB ;
 4    pot2HB ;
 5    pot2LB ;
 CHECKSUM
*/

void BOARD::relayMessage( Message* newMes )
{
    message = *newMes ; // this update the payload of an input board. It does however need to trigger a notification of some sort.
                        // I think it is unavoidable to move this method to the sub-classes so the payload can be processed to the variables per board type directly
}



JACK_OF_ALL_TRADES::JACK_OF_ALL_TRADES()
{
}

Message* JACK_OF_ALL_TRADES::getMessage() // N.B. may need to make a virtual functions in board class.
                                            // also, may want to dump the function definitions in header file so I can eliminate the cpp file.
{
    if( message.OPCODE != OPC_SET_DATA ) // if no command is loaded yet, we will do input readout.
    {
        message.OPCODE = OPC_GET_DATA ;
        message.payload[0] = 6 ;

        for( int i = 1 ; i < 6 ; i ++ )
        {
            message.payload[i] = 0 ;    // inputs need zeroes
        }
    }
    return &message ;
}

void  JACK_OF_ALL_TRADES::setServo( uint8 index, uint8 state )  // set 'outputs' variable correctly (may want to change indices to correspond with pinnumber instead?)
{
    if( index > 3 ) return ;

    message.OPCODE  = OPC_SET_DATA ;
    message.payload[0] = 5 ; // message length 

    if( state ) message.payload[4] |=  (1 << (7-index)) ;    // SSSS RRRR 
    else        message.payload[4] &= ~(1 << (7-index)) ;
}

void  JACK_OF_ALL_TRADES::setRelay( uint8 index, uint8 state )  // set 'outputs' variable (may want to change indices to correspond with pinnumber instead?)
{
    if( index > 3 ) return ;

    message.OPCODE  = OPC_SET_DATA ;
    message.payload[0] = 5 ; // message length 

    if( state ) message.payload[4] |=  (1 << (3-index)) ;    // SSSS RRRR 
    else        message.payload[4] &= ~(1 << (3-index)) ;
}

void  JACK_OF_ALL_TRADES::setPWM( uint8 index, uint8 val )    // called from package manager in order to update the value (may want to change indices to correspond with pinnumber instead?)
{
    if( index > 2 ) return ;

    message.OPCODE     = OPC_SET_DATA ;
    message.payload[0] = 5 ; // message length 
    
    message.payload[index+1] = val ; 
}

uint16 JACK_OF_ALL_TRADES::getADC( uint8 index )
{
    if( index >= 2 ) return 0 ;

    return adc[index] ;
}

uint8 JACK_OF_ALL_TRADES::getPWM( uint8 index )
{
    if( index >= 3 ) return 0 ;
    return pwm[ index ] ;
}

uint8 JACK_OF_ALL_TRADES::getRelay( uint8 index ) // ssss RRRR 
{
    if( index >= 4 ) return 0 ; // N.B. may want to remap index numbers so they correspond with actual Pins

    return (output >> index) & 1 ;
}

uint8 JACK_OF_ALL_TRADES::getServo( uint8 index )// SSSS rrrr 
{
    if( index >= 4 ) return 0 ; // N.B. may want to remap index numbers so they correspond with actual Pins

    return (output >> (index+4)) & 1 ;
}

// <OPC_SET_OUTPUT> <DATA1> <DATA2> <CHECKSUM>
DIGITAL_INPUT_BOARD::DIGITAL_INPUT_BOARD()
{

}

void DIGITAL_INPUT_BOARD::relayMessage( Message* newMes )
{
    inputs = 0 ;
    inputs |= (newMess -> payload[0]) << 8 ;
    inputs |= (newMess -> payload[1]) ;
}

Message* DIGITAL_INPUT_BOARD::getMessage()
{
    if( updateDue ) 
    {   updateDue = 0 ;

        message.OPCODE     = OPC_CONF_IO ; // general configuration OPCODE
        message.payload[0] = 3 ;           // length 3 or 4?
        message.payload[1] = (configuration >> 8) & 0xFF ;
        message.payload[2] =  configuration       & 0xFF ;
    }
    else
    {
        message.OPCODE     = OPC_GET_INPUTS ;
        message.payload[0] = (inputs >> 8) & 0xFF ;
        message.payload[1] =  inputs       & 0xFF ;
    }

    return &message ;
}

void DIGITAL_INPUT_BOARD::setConfig( uint8 index, uint8 state )
{
    updateDue = 1 ;

    if( state ) configuration |=  (1<<index) ; 
    else        configuration &= ~(1<<index) ;
}

uint8 DIGITAL_INPUT_BOARD::getInput( uint8 index )
{
    return (inputs >> index) & 1 ;
}



// <OPC_SET_OUTPUT> <DATA1> <DATA2> <CHECKSUM>
DIGITAL_OUTPUT_BOARD::DIGITAL_OUTPUT_BOARD()
{

}

Message* DIGITAL_OUTPUT_BOARD::getMessage()
{
    if( updateDue ) 
    {   updateDue = 0 ;

        message.payload[0] = (outputs >> 8) & 0xFF ;
        message.payload[1] =  outputs       & 0xFF ;
    }
    else
    {
        message.OPCODE = OPC_IDLE ;
    }

    return &message ;
}

void DIGITAL_OUTPUT_BOARD::setOuput( uint8 index, uint8 state )
{
    updateDue = 1 ;
    message.OPCODE = OPC_SET_OUTPUTS ;

    if( state ) outputs |=  (1<<index) ; 
    else        outputs &= ~(1<<index) ;
}

uint8 DIGITAL_OUTPUT_BOARD::getOutput( uint8 index )
{
    return (outputs >> index) & 1 ;
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



