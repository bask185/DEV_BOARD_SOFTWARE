#include "BOARDS.h"


BOARD::BOARD() {}

// void BOARD::relayMessage( Message* newMes )
// {
//     message = *newMes ; // this update the payload of an input board. It does however need to trigger a notification of some sort.
//                         // I think it is unavoidable to move this method to the sub-classes so the payload can be processed to the variables per board type directly
// }




// <OPC_GET_INPUTS> <DATA1> <DATA2> <CHECKSUM>
DIGITAL_INPUT_BOARD::DIGITAL_INPUT_BOARD() { }

void DIGITAL_INPUT_BOARD::relayMessage( Message* newMess ) // process incomming input message from bus
{
    inputs = ((newMess -> payload[0]) << 8) | (newMess -> payload[1])  ;
}

Message* DIGITAL_INPUT_BOARD::getMessage()
{
    if( updateDue ) 
    {   updateDue = 0 ;

        message.OPCODE     = OPC_CONF_IO ; // general configuration OPCODE, can be used to enable ISR and such
        message.payload[0] = 3 ;           // length 3 or 4?
        message.payload[1] = (configuration >> 8) & 0xFF ;
        message.payload[2] =  configuration       & 0xFF ;
    }

    else
    {
        message.OPCODE = OPC_GET_INPUTS ; // normal input request
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
DIGITAL_OUTPUT_BOARD::DIGITAL_OUTPUT_BOARD() { }

void DIGITAL_OUTPUT_BOARD::relayMessage( Message* newMes ) // digital outputs have no feedback
{
}

Message* DIGITAL_OUTPUT_BOARD::getMessage()
{
    if( updateDue ) 
    {   updateDue = 0 ;

        message.OPCODE     = OPC_SET_OUTPUTS ;
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

    if( state ) outputs |=  (1<<index) ; 
    else        outputs &= ~(1<<index) ;
}

uint8 DIGITAL_OUTPUT_BOARD::getOutput( uint8 index )
{
    return (outputs >> index) & 1 ;
}





// <OPC_GET_DAC> <ADC_1_HB> <ADC_1_LB> <ADC_2_HB> <ADC_2_LB> <ADC_3_HB> <ADC_3_LB> <ADC_4_HB> <ADC_4_LB> <CHECKSUM>
ANALOG_INPUT_BOARD::ANALOG_INPUT_BOARD() { }

void ANALOG_INPUT_BOARD::relayMessage( Message* newMess )
{  //                     HB                             LB
    inputs[0] = ((newMess -> payload[0]) << 8) | (newMess -> payload[1]) ;
    inputs[1] = ((newMess -> payload[2]) << 8) | (newMess -> payload[3]) ;
    inputs[2] = ((newMess -> payload[4]) << 8) | (newMess -> payload[5]) ;
    inputs[3] = ((newMess -> payload[6]) << 8) | (newMess -> payload[7]) ;
}

Message* ANALOG_INPUT_BOARD::getMessage()
{
    message.OPCODE     = OPC_GET_INPUTS ;
    return &message ;
}

uint16 ANALOG_INPUT_BOARD::getADC( uint8 index )
{
    if( index > 3 ) return 0 ;

    return inputs[index] ;
}



// <OPC_SET_DAC> <DAC_1_HB> <DAC_1_LB> <DAC_2_HB> <DAC_2_LB> <DAC_3_HB> <DAC_3_LB> <DAC_4_HB> <DAC_4_LB> <CHECKSUM>
ANALOG_OUTPUT_BOARD::ANALOG_OUTPUT_BOARD()
{
}

void ANALOG_OUTPUT_BOARD::relayMessage( Message* newMes ) // analog outputs have no feedback
{
}

Message* ANALOG_OUTPUT_BOARD::getMessage()
{
    if( updateDue ) 
    {   updateDue = 0 ;

        message.OPCODE     = OPC_SET_DAC ;
        for( uint8 i = 0 ; i < 8 ; i += 2 ) 
        {
            message.payload[ i ] = outputs[i] >> 8 ; // TEST ME
            message.payload[i+1] = outputs[i] ;
        }
        // message.payload[0] = ( outputs[0] >> 8 ) & 0xFF ;
        // message.payload[1] = ( outputs[0]      ) & 0xFF ;
        // message.payload[2] = ( outputs[1] >> 8 ) & 0xFF ;
        // message.payload[3] = ( outputs[1]      ) & 0xFF ;
        // message.payload[4] = ( outputs[2] >> 8 ) & 0xFF ;
        // message.payload[5] = ( outputs[2]      ) & 0xFF ;
        // message.payload[6] = ( outputs[3] >> 8 ) & 0xFF ;
        // message.payload[7] = ( outputs[3]      ) & 0xFF ;
    }
    else
    {
        message.OPCODE = OPC_IDLE ;
    }

    return &message ;
}

void ANALOG_OUTPUT_BOARD::setDAC( uint8 index, uint16 value )
{
    if( index > 3 ) return ;

    updateDue = 1 ;
    outputs[index] = value ;    
}





// <OPC_GET_ENCODER> <enc 0 byte 3> .. <enc 0 byte 0> ..... <enc 3 byte 3> .. <enc 3 byte 0> <CHECKSUM>
ENCODER_BOARD::ENCODER_BOARD()
{
}

void ENCODER_BOARD::relayMessage( Message* newMess )
{
    encoder[0] = (((newMess->payload[ 0]) << 24)|(newMess->payload[ 1] << 16)|(newMess->payload[ 2] << 8)|(newMess->payload[ 3])) ;
    encoder[1] = (((newMess->payload[ 4]) << 24)|(newMess->payload[ 5] << 16)|(newMess->payload[ 6] << 8)|(newMess->payload[ 7])) ;
    encoder[2] = (((newMess->payload[ 8]) << 24)|(newMess->payload[ 9] << 16)|(newMess->payload[10] << 8)|(newMess->payload[11])) ;
    encoder[3] = (((newMess->payload[12]) << 24)|(newMess->payload[13] << 16)|(newMess->payload[14] << 8)|(newMess->payload[15])) ;
}

Message* ENCODER_BOARD::getMessage()
{
    message.OPCODE = OPC_GET_ENCODER ;
    return &message ;
}

uint32 ENCODER_BOARD::getEncoder( uint8 index )
{
    if( index > 3 ) return 0 ;

    return encoder[index] ;
}







JACK_OF_ALL_TRADES::JACK_OF_ALL_TRADES()
{
}

Message* JACK_OF_ALL_TRADES::getMessage() // N.B. may need to make a virtual functions in board class.
                                            // also, may want to dump the function definitions in header file so I can eliminate the cpp file.
{
    if( updateDue == 1 ) // if no command is loaded yet, we will do input readout.
    {   updateDue  = 0 ;

        message.OPCODE = OPC_SET_DATA ;
    }

    else
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

    if( state ) message.payload[4] |=  (1 << (7-index)) ;    // SSSS rrrr 
    else        message.payload[4] &= ~(1 << (7-index)) ;

    updateDue = 1 ;
}

void  JACK_OF_ALL_TRADES::setRelay( uint8 index, uint8 state )  // set 'outputs' variable (may want to change indices to correspond with pinnumber instead?)
{
    if( index > 3 ) return ;

    message.OPCODE  = OPC_SET_DATA ;
    message.payload[0] = 5 ; // message length 

    if( state ) message.payload[4] |=  (1 << (3-index)) ;    // ssss RRRR 
    else        message.payload[4] &= ~(1 << (3-index)) ;

    updateDue = 1 ;
}

void  JACK_OF_ALL_TRADES::setPWM( uint8 index, uint8 val )    // called from package manager in order to update the value (may want to change indices to correspond with pinnumber instead?)
{
    if( index > 2 ) return ;

    message.OPCODE     = OPC_SET_DATA ;
    message.payload[0] = 5 ; // message length 
    
    message.payload[index+1] = val ; 

    updateDue = 1 ;
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

    return (outputs >> index) & 1 ;
}

uint8 JACK_OF_ALL_TRADES::getServo( uint8 index )// SSSS rrrr 
{
    if( index >= 4 ) return 0 ; // N.B. may want to remap index numbers so they correspond with actual Pins

    return (outputs >> (index+4)) & 1 ;
}



ARDUINO_BOARD::ARDUINO_BOARD() 
{

}

void ARDUINO_BOARD::relayMessage( Message* newMess )
{
}

Message* ARDUINO_BOARD::getMessage()
{
    //message.OPCODE = OPC_GET_ENCODER ;
    return &message ;
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



