#include "src/macros.h"
#include "src/TSCbus.h"
#include "src/BOARDS.h"
#include <Servo.h>

ARDUINO_BOARD me ;
TSCbus bus ;

const int nServo = 12 ;

Servo   servo[nServo];
uint8   servoPin[nServo] =
{
    255,255,255,255,
    255,255,255,255,
    255,255,255,255 
} ;

uint8  isAnalog ; // mark pins of A0 <> A7 as analog input pins.
uint8  inputArray[3] ;
uint16 analogValues[8] ;

void readDigitalInputs() //  D0-D7, D8-D13, A0-A5
{
    REPEAT_MS( 8 )
    {   static uint8 idx = 0 ; if( ++ idx == 3 ) idx = 0 ;

        if( idx == 0 ) { startPin =  0 ; endPin =  7 ; }
        if( idx == 1 ) { startPin =  8 ; endPin = 13 ; }
        if( idx == 2 ) { startPin = A0 ; endPin = A5 ; }
        for( int pin = startPin ; pin < endPin ; pin ++ ) inputArray[idx] |= digitalRead( pin ) ;
    }
    END_REPEAT
}

uint8 isAnalogPin( uint8 pin )
{
    pin -= A0 ;

    return ((isAnalog >> pin) & 1) ;
}

void readAnalogInputs() //  TODO: figure out if pin is configured as analog pin and perform readout
{
    REPEAT_MS( 5 ) // N.B. should not do 8 analog readouts in sequence. one at the time -> better
    {   static uint8 idx = 0 ; if( ++ idx == 8 ) idx = 0 ;

        if( isAnalogPin( A0 + idx ) ) analogValues[idx] = analogRead( A0 + idx ) ;
    }
    END_REPEAT
}

void setup()
{
    Serial.begin( 115200 ) ;
}

void loop()
{
    bus.transceiveMessage() ;

    readDigitalInputs() ;

    readAnalogInputs() ;
}

// CALL BACK FUNCTIONS

// get inputs
void notifyGetPayload( uint8 OPCODE, uint8 index )
{
    if( OPCODE == OPC_GET_INPUT )
    {       	                  // idx: 0      1       2
        return inputArray[index] ;  //  D0-D7, D8-D13, A0-A5
    }

    if( OPCODE == OPC_GET_ANALOG )
    {
        uint8 val ;
        if( index % 2 == 0 ) val = highByte( analogValues[index/2] ) ;
        else                 val =  lowByte( analogValues[index/2] ) ;

        return val ;
    }
}

// set outputs
void notifyConfigPin( Message* message )
{
    uint8 len = message->length ;

    for( int i = 0 ; i < len ; i += 2 )
    {
        uint8   pin = message->payload[  i  ] ;
        uint8 iodir = message->payload[ i+1 ] ;
    
        if( pin >= A0 && pin <= A7 )
        {
            isAnalog |= (iodir << pin) ; // set these bits to mark analog inputs
        }

        pinMode( pin, iodir ) ;     // dig
        printNumber("pin #", pin ) ;
        printNumberln(", iodir = ", iodir) ;
    }
}

void notifySetOutput( uint8 pin, uint8 val )
{
    pinMode( pin, OUTPUT ) ;
    digitalWrite( pin, val ) ;
}

void notifySetPwm( uint8 pin, uint8 val )
{
    analogWrite( pin, val ) ;
}

void notifySetServo( uint8 pin, uint8 val )
{
    for( int i = 0 ; i < nServo ; i ++ )
    {
        if( servoPin[i] == 255 ) // free servo object found!
        {
            servoPin[i] = pin ;      // link pin to object
            servo[i].attach( pin ) ; // attach pin to object
        }
        if( servoPin[i] == pin )
        {   
            servo[i].write( val ) ;
        }
    }
}
