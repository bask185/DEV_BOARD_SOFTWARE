#include "src/macros.h"
#include "src/TSCbus.h"
#include "src/io.h"
#include "src/ServoSweep.h"
#include "src/debounceClass.h"
#include <Wire.h>



TSCbus bus ;

struct
{
// IO CONFIG message
/* byte 0 */    uint8 mode ;       // 1 = servos without frog
                                   // 2 = servos with frog
/* byte 1 */    uint8 servoConf ;  // 1 = decrement last set servo
                                   // 2 = increment last set servo

// IO CONTROL message as follows:
// OUTPUTS
/* byte 0 *     uint8 PWM1 ;   // obsolete, values are processed to analogWrite after transceiving   */
/* byte 1 *     uint8 PWM2 ; */
/* byte 2 *     uint8 PWM3 ; */
/* byte 3 *     uint8 outputs ; // SSSSRRRR  Servo1-4 Relay */

// INPUTS  (all are used)
/* byte 4 */    uint8 switches ;// xxxxSSSS  (sw4-Sw1)
/* byte 5 */    uint8 pot1HB ;  // xxxxxxPP
/* byte 6 */    uint8 pot1LB ;  // PPPPPPPP
/* byte 7 */    uint8 pot2HB ;  // xxxxxxPP
/* byte 8 */    uint8 pot2LB ;  // PPPPPPPP
} me ;


const int PCFaddress = 0x20 ;

uint8   currentServoIndex ;

const int nRelays = 4 ;
const int nServos = 4 ;

#ifdef HAS_FROG_RELAY
ServoSweep servo[nServos] =
{
    ServoSweep( servo1, 80, 100, 20, 1, relay1 ), // with relay
    ServoSweep( servo2, 80, 100, 20, 1, relay2 ),
    ServoSweep( servo3, 80, 100, 20, 1, relay3 ),
    ServoSweep( servo4, 80, 100, 20, 1, relay4 ),
} ;
#else

ServoSweep servo[nServos] =
{
    ServoSweep( servo1, 80, 100, 20, 1 ), // without relay
    ServoSweep( servo2, 80, 100, 20, 1 ),
    ServoSweep( servo3, 80, 100, 20, 1 ),
    ServoSweep( servo4, 80, 100, 20, 1 ),
} ;
#endif

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
        me.switches = 0 ;                       // clear all 4 bits
        for( int i = 0 ; i < nInputs ; i ++ )
        {
            input[i].debounce() ;

            if( input[i].getState() == HIGH ) me.switches |= (1 << i) ; // and set them
        }
    }
    END_REPEAT
}

void readPotentiometers()
{
    REPEAT_MS( 10 ) // do one pot meter at the the time
    {
        static uint8 select ; select ^= 1 ;
        int sample ;
        if( select )
        {
            sample = analogRead( POT1 ) ;
            me.pot1HB = (sample >> 8) & 0b00000011 ;
            me.pot1LB =  sample       & 0b11111111 ;
        }
        else
        {
            sample = analogRead( POT2 ) ;
            me.pot2HB = (sample >> 8) & 0b00000011 ;
            me.pot2LB =  sample       & 0b11111111 ;
        }
    }
    END_REPEAT
}



void setup()
{
    Serial.begin( 115200 ) ;
    
    initIO() ;

    for( int i = 0 ; i < nServos ; i ++ )
    {
        servo[i].useEEPROM() ; 
        servo[i].begin() ;
    }
}

void loop()
{
    bus.transceiveMessage() ;
    debounceSwitches() ;
    readPotentiometers() ;
    sweepServos() ;
}

// CALL BACK FUNCTIONS

// called by bus transceiver, it's a request for input data.
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

// UPDATE OUTPUTS
void notifySetData( Message *message )
{
    if( message.OPCODE == OPC_CONF_IO )
    {
        me.mode         = message -> payload[0] ; // 1 = servos without frog
                                                  // 2 = servos with frog
        me.servoConf    = message -> payload[1] ; // 1 = decrement last set servo
                                                  // 2 = increment last set servo

        // if( me.servoConf == 1 ) servo[lastSetServo].decrement() ;
        // if( me.servoConf == 2 ) servo[lastSetServo].increment() ;
    }

    if( message.OPCODE == OPC_SET_DATA )    // command to set ALL outputs
    {
        analogWrite( PWM1, message -> payload[0] ) ; printNumberln("setting PWM 1: ", message -> payload[0] ) ;
        analogWrite( PWM2, message -> payload[1] ) ; printNumberln("setting PWM 2: ", message -> payload[1] ) ;
        analogWrite( PWM3, message -> payload[2] ) ; printNumberln("setting PWM 3: ", message -> payload[2] ) ;

        uint8 outputs = message -> payload[3] ;// SSSSRRRR  Servo1-4 Relay1-4

        for( int i = 0 ; i < nServos ; i ++ )
        {
            uint8 state     = (outputs >> (7-i)) & 0b1 ;
            uint8 prevState = servo[i].getState() ;
            servo[i].setState( state ) ; 
        }

    #ifndef HAS_FROG_RELAY
        for( int i = 0 ; i < nRelays ; i ++ )
        {
            digitalWrite( relay1+i, (outputs >> (3-i)) & 1 ) ; 
            printNumberln("setting relay : ", i ) ;
        }

    #endif
    }
}