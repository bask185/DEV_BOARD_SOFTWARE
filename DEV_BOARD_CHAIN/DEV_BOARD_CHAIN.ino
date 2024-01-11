#include "src/macros.h"
#include "src/TSCbus.h"
#include "src/io.h"
#include "src/ServoSweep.h"
#include "src/debounceClass.h"
#include <Wire.h>



TSCbus bus ;

JACK_OF_ALL_TRADES me ; // yes, I am serious about 'me'

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

// const int nInputs = 4 ;
// Debounce input[nInputs] =
// {
//     Debounce( SW1 ),
//     Debounce( SW2 ),
//     Debounce( SW3 ),
//     Debounce( SW4 ),
// } ;

// void sweepServos()
// {
//     for( int i = 0 ; i < nServos ; i ++ )
//     {
//         servo[i].sweep() ;
//     }
// }

// void debounceSwitches()
// {
//     REPEAT_MS( 20 )
//     {
//         for( int i = 0 ; i < nInputs ; i ++ )
//         {
//             input[i].debounce() ;
//         }
//     }
//     END_REPEAT
// }

// void processSwitches()
// {
//     for( int i = 0 ; i < nInputs ; i ++ )
//     {
//         uint8 state = input[i].getState() ;
//         if( state == FALLING ) sendInput( input[i].getPin(), 0 ) ;
//         if( state == FALLING ) sendInput( input[i].getPin(), 1 ) ;
//     }
// }




void setup()
{
    Serial.begin( 115200 ) ;
    
    // initIO() ;

    // Wire.begin() ;

    // setup232( Serial, 115200ul ) ;

    // for( int i = 0 ; i < nServos ; i ++ )
    // {
    //     servo[i].useEEPROM() ; 
    //     // servo[i].reset() ;  // use this once if EEPROM is FUBAR
    //     servo[i].begin() ;
    // }

    // nPCF = 0 ;
    // uint8 state ;

    // for( int i = 0 ; i < 8 ; i ++ ) // max amount PCF
    // {
    //     Wire.beginTransmission(PCFaddress);
    //     state = Wire.endTransmission() ;    // check which PCF are connected

    //     if( state == 0 ) nPCF ++ ;          // PCF found, increment counter.
    //     else break ;
    // }
    pinMode(13, OUTPUT ) ;
    digitalWrite(13,LOW) ;
}

void loop()
{
    bus.transceiveMessage() ;

    // receiveMessage() ;
    // debounceSwitches() ;
    // processSwitches() ;
    // processPCF() ;
    // sweepServos() ;
}

/* application is to provide payload information on given index 
function should be exited ASAP */
uint8 notifyGetPayload( uint8 OPCODE, uint8 index )
{
    if( OPCODE == OPC_SET_DATA )
    {
        if( index == 4 ) return me.switches ; /* byte 4 * /    uint8 switches ;// xxxxSSSS  (sw4-Sw1) */
        if( index == 5 ) return me.pot1HB ;   /* byte 5 * /    uint8 pot1HB ;  // xxxxxxPP */
        if( index == 6 ) return me.pot1LB ;   /* byte 6 * /    uint8 pot1LB ;  // PPPPPPPP */
        if( index == 7 ) return me.pot2HB ;   /* byte 7 * /    uint8 pot2HB ;  // xxxxxxPP */
        if( index == 8 ) return me.pot2LB ;   /* byte 8 * /    uint8 pot2LB ;  // PPPPPPPP */
    }
    
    return 0x00 ;
}


void notifySetData( Message *message )
{
    if( message.OPCODE == OPC_CONF_IO )
    {
        me.mode         = message -> payload[0] ; // 1 = servos without frog
                                                  // 2 = servos with frog
        me.servoConf    = message -> payload[1] ; // 1 = decrement last set servo
                                                  // 2 = increment last set servo
    }

    if( message.OPCODE == OPC_SET_DATA )    // command to set ALL outputs
    {
        me.PWM1    = message -> payload[0] ;
        me.PWM2    = message -> payload[1] ;
        me.PWM3    = message -> payload[2] ;
        me.outputs = message -> payload[3] ;// SSSSRRRR  Servo1-4 Relay1-4

        servo[1].tglState( (me.outputs >> 6) & 1 ) ; 
        servo[0].tglState( (me.outputs >> 7) & 1 ) ; 
        servo[2].tglState( (me.outputs >> 5) & 1 ) ; 
        servo[3].tglState( (me.outputs >> 4) & 1 ) ;

        if( mode == no_frog ) // if relays are not connected to servo's, control them
        {
            digitalWrite( relay1, (me.outputs >> 3) & 1 ) ;
            digitalWrite( relay2, (me.outputs >> 2) & 1 ) ;
            digitalWrite( relay3, (me.outputs >> 1) & 1 ) ;
            digitalWrite( relay4, (me.outputs >> 0) & 1 ) ;
        } 
    }
}

/*
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
} */