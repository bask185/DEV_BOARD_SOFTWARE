#include "message.h"

static HardwareSerial*   hwPort = nullptr ;
static SoftwareSerial*   swPort = nullptr ;
static Stream*           mySerial = nullptr ;

char        message[22] ;
uint16_t    COMMAND ;
uint16_t    DATA1 ;
uint16_t    DATA2 ;
uint16_t    DATA3 ;
uint16_t    DATA4 ;
uint16_t    DATA5 ;
uint16_t    DATA6 ;

void setup232( SoftwareSerial& s, uint32_t baud )
{
    swPort = &s ;
    mySerial = swPort ;
    swPort->begin( baud ) ;
}

void setup232( HardwareSerial& s, uint32_t baud )
{
    hwPort = &s ;
    mySerial = hwPort ;
    hwPort->begin( baud ) ;
}

// transmitt control functions
void sendOutput( uint8 pin, uint8 state )
{
    uint8 message[] = { OPCsetOutput, pin, state } ;
    sendMessage( message, 4 ) ;
}

void sendServo( uint8 pin, uint8 state )
{
    uint8 message[] = { OPCsetServo, pin, state } ;
    sendMessage( message, 4 ) ;
}

void sendPWM( uint8 pin, uint8 dutyCycle )
{
    uint8 message[] = { OPCsetPwm, pin, dutyCycle } ;
    sendMessage( message, 4 ) ;
}

void sendInput( uint8 pin, uint8 state )
{
    uint8 message[] = { OPCsendInput, pin, state } ;
    sendMessage( message, 4 ) ;
}


void processMessage()
{
    sscanf( message, "%d,%d,%d,%d,%d,%d,%d", 
        &COMMAND, &DATA1, &DATA2, &DATA3,
                  &DATA4, &DATA5, &DATA6 ) ;

    switch( COMMAND )
    {
        case OPCsetMode   : if( OPCsetMode )      notifySetMode(   DATA1        ) ; break ;
        case OPCsendInput : if( notifyInput )     notifyInput(     DATA1, DATA2 ) ; break ; 
        case OPCconfServo : if( notifyConfServo ) notifyConfServo( DATA1        ) ; break ;
        case OPCsetOutput : if( notifyOutput )    notifyOutput(    DATA1, DATA2 ) ; break ;
        case OPCsetServo  : if( notifyServo )     notifyServo (    DATA1, DATA2 ) ; break ;
        case OPCsetPwm    : if( notifyPWM )       notifyPWM(       DATA1, DATA2 ) ; break ;
    }
}

// reads and buffers in an incomming message.
// returns true when newline is received
uint8 receiveMessage()
{
    static int index = 0 ;

    if( mySerial -> available() > 0 )
    {   
        char c = mySerial -> read() ;
        if( c == '\n')
        {
            message[index] = 0 ;
            index = 0 ;

            processMessage() ;

            return true ;
        }
        else if( c >= ' ' && c < 0x80 )   // discard all non printable characters except newline
        {
            message[index++] = c ;
        }
    }
    return false; 
}

void sendMessage( uint8 *message, uint8 len )
{
    for( int i = 0 ; i < len ; i ++ )
    {
        mySerial->print( *message ) ;
        if( i < len-1) mySerial->write(',') ;
        message ++ ;
    }
    mySerial->println() ;
}
