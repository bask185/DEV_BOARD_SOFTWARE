#include "src/macros.h"
#include "src/message.h"
#include "src/io.h"
#include "src/ServoSweep.h"
#include "src/debounceClass.h"
#include <Wire.h>

/* TODO
- detect PCF modules and transmitt inputs. 
  should use pins 16-23, 24-31 etc
- add function to configure servo positions if needed?
*/

const int PCFaddress = 0x20 ;

uint8   currentServoIndex ;
uint8   nPCF ;

const int nServos = 4 ;
ServoSweep servo[nServos] =
{
    ServoSweep( servo1, 80, 100, 20, 1 ),
    ServoSweep( servo2, 80, 100, 20, 1 ),
    ServoSweep( servo3, 80, 100, 20, 1 ),
    ServoSweep( servo4, 80, 100, 20, 1 ),
} ;

const int nInputs = 4 ;
Debounce input[nInputs] =
{
    Debounce( SW1 ),
    Debounce( SW2 ),
    Debounce( SW3 ),
    Debounce( SW4 ),
} ;

void sweepServos()
{
    for( int i = 0 ; i < nServos ; i ++ )
    {
        servo[i].sweep() ;
    }
}

void debounceSwitches()
{
    REPEAT_MS( 20 )
    {
        for( int i = 0 ; i < nInputs ; i ++ )
        {
            input[i].debounce() ;
        }
    }
    END_REPEAT
}

void processSwitches()
{
    for( int i = 0 ; i < nInputs ; i ++ )
    {
        uint8 state = input[i].getState() ;
        if( state == FALLING ) sendInput( input[i].getPin(), 0 ) ;
        if( state == FALLING ) sendInput( input[i].getPin(), 1 ) ;
    }
}

void processPCF()
{
    if( nPCF == 0 ) return ;

    REPEAT_MS( 50)
    {
        for( int PCF = 0 ; PCF < nPCF ; PCF ++ )
        {
            Wire.requestFrom( PCFaddress+PCF, 1 ) ;

            static uint8 prevInputs[8] ;
            uint8 inputs = Wire.read() ;

            for( int pin = 0 ; pin < 8 ; pin ++ )
            {
                uint8 pinNew = (    inputs      >> pin) & 1 ;
                uint8 pinOld = (prevInputs[PCF] >> pin) & 1 ;

                if( pinNew != pinOld )
                {  // 20<->27  28<->35  36<->43  44<->51  ETC  
                    sendInput( 20 + pin + nPCF*8, pinNew ) ; // TEST ME
                }
            }
            prevInputs[PCF] = inputs ;
        }
    }
    END_REPEAT
}


void setup()
{
    initIO() ;

    Wire.begin() ;

    setup232( Serial, 115200ul ) ;

    for( int i = 0 ; i < nServos ; i ++ )
    {
        servo[i].useEEPROM() ; 
        // servo[i].reset() ;  // use this once if EEPROM is FUBAR
        servo[i].begin() ;
    }

    nPCF = 0 ;
    uint8 state ;

    for( int i = 0 ; i < 8 ; i ++ ) // max amount PCF
    {
        Wire.beginTransmission(PCFaddress);
        state = Wire.endTransmission() ;    // check which PCF are connected

        if( state == 0 ) nPCF ++ ;          // PCF found, increment counter.
        else break ;
    }
}

void loop()
{
    receiveMessage() ;
    debounceSwitches() ;
    processSwitches() ;
    processPCF() ;
    sweepServos() ;
}


// callback function from received messages
void notifyOutput( uint8 pin, uint8 state )
{
    if( (pin >= relay1 && pin <= relay4 )
    ||   pin == PWM1 
    ||   pin == PWM2 
    ||   pin == PWM3 )  // perform a check if the received pin is any of these outputs
    {
        pinMode( pin, OUTPUT ) ;
        digitalWrite( pin, state ) ;
        // printNumberln("setting pin ", pin);
    }
}

void notifyServo( uint8 pin, uint8 state )
{
    for( int i = 0 ; i < nServos ; i ++ ) // perform a check if the received pin corresponds to a servo object.
    {
        if( servo[i].getPin() == pin )   // if match is found...
        {
            servo[i].setState( state ) ; // set the state
            currentServoIndex = i ;
            // printNumberln("setting servo ", currentServoIndex);
        }
    }
}

void notifyPWM( uint8 pin, uint8 dutyCycle )
{
    if( pin == PWM1 
    ||  pin == PWM2 
    ||  pin == PWM3 )   // check for PWM pins.
    {
        pinMode( pin, OUTPUT ) ;
        analogWrite( pin, dutyCycle ) ;
        // printNumberln("setting PWM ", pin);
    }
}

void notifyConfServo( uint8 state ) // with either a 0 or a 1 the last used servo's position can be set.
{
    if( state == 0 ) servo[currentServoIndex].decrement() ;
    if( state == 1 ) servo[currentServoIndex].increment() ;
    // printNumberln("confing servo ", currentServoIndex);
}

void notifySetMode(uint8 mode )
{
    for( int i = 0 ; i < nServos ; i ++ )
    {
        if( mode == 0 ) 
        {
            servo[i].setRelay( 0xFF ) ;   // disable relay pin
        }
        if( mode == 1 ) 
        {
            servo[i].setRelay(relay1+i) ; // all 4 relay pins are sequential
        }
    }
} 