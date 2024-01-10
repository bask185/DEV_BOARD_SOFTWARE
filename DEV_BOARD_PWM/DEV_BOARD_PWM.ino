#include "src/io.h"
#include "src/date.h"
#include "src/version.h"
#include "src/macros.h"
#include "src/debounceClass.h"
#include "src/event.h"
#include "src/NmraDcc.h"
#include "src/weistra.h"
#include "src/stateMachineClass.h"
#include "src/ServoSweep.h"
#include <Wire.h>

#define AUTO_SHUTTLE
//#define RECORDER


/* CONSTANTS */
const int Fmin =  50 ;
const int Fmax = 100;

const int PCFaddress = 0x20 ;

/* OBJECT DECLARATION */
ServoSweep servo[] =
{
    ServoSweep( servo1, 80, 100, 20, 1 ),
    ServoSweep( servo2, 80, 100, 20, 1 ),
    ServoSweep( servo3, 80, 100, 20, 1 ),
    ServoSweep( servo4, 80, 100, 20, 1 ),
} ;

Debounce button[] =         // note may need to declare another array for I2C extender.
{
    Debounce( SW1 ),
    Debounce( SW2 ),
    Debounce( SW3 ),
    Debounce( SW4 ),
} ;

Debounce pcfInput[] =
{
    Debounce( 0xFF ),
    Debounce( 0xFF ),
    Debounce( 0xFF ),
    Debounce( 0xFF ),
    Debounce( 0xFF ),
    Debounce( 0xFF ),
    Debounce( 0xFF ),
    Debounce( 0xFF ),
} ;

Weistra pwm( PWM1, Fmin, Fmax ) ;

StateMachine sm ;
enum states
{
    runMode,
    braking,
    departing,
} ;

EventHandler event( 0x0000, 0x7FFF) ;
enum events
{
    speedEvent,
    relayEvent,
    servoEvent,
} ;


/* VARIABLES */

int8    currentSpeed ;
int8    setPoint ;
int8    entrySpeed ;
uint8   oldDir ;

    // sample    0      512-50   512   512+50      1023
    // speed  -100   <->   0      0       0   <->   100  // dead zone in middle
void readThrottleKnob()
{
    REPEAT_MS( 200 ) // determens acceleration. SHOULD BE DEPENDED IF AUTO SHUTTLE IS DEFINED OR NOT. 
    {
        static int prevSetPoint ;

        int setPoint2b = analogRead( POT1 ) ; 
        const int deadRange = 65 ;
        if(      setPoint2b <= (512-deadRange)){ setPoint2b = map( setPoint2b,      0, 512-deadRange, -100, 0 ) ; }
        else if( setPoint2b >= (512+deadRange)){ setPoint2b = map( setPoint2b, 512+deadRange,   1023,  0,  100 ) ; }
        else                                   { setPoint2b = 0 ; }

        if( setPoint2b != prevSetPoint && sm.getState() != runMode )
        {     setPoint  = prevSetPoint = setPoint2b ;

        #ifdef RECORDER
            event.storeEvent( speedEvent, setPoint, 0xFF ) ;
        #endif
        }
    }
    END_REPEAT
}


void speedControl()
{
    REPEAT_MS( 50 )
    {
        if( setPoint != currentSpeed )
        {
            if( currentSpeed < setPoint ) currentSpeed ++ ;
            if( currentSpeed > setPoint ) currentSpeed -- ;

            if( currentSpeed > -3 && currentSpeed < 3 )
            {
                pwm.setSpeed( 0 ) ; 
                digitalWrite( relay3,  LOW ) ; 
                digitalWrite( relay4,  LOW ) ; 
            }
            else
            {
                pwm.setSpeed( currentSpeed ) ;
                if( currentSpeed < -3 ) { digitalWrite( relay3, HIGH ) ;    // set relays accordingly
                                          digitalWrite( relay4,  LOW ) ; }
                if( currentSpeed >  3 ) { digitalWrite( relay3,  LOW ) ;
                                          digitalWrite( relay4, HIGH ) ; }
            }
        }
    }
    END_REPEAT
}

void notifyEvent( uint8 type, uint16 data1, uint8 data2 )
{
    switch( type )
    {
    case speedEvent:
        setPoint = data1 ;
        break ;

    case relayEvent:
        digitalWrite( data1, data2 ) ;
        break ;

    case servoEvent:
        servo[data1].setState( data2 ) ;
        break ;
    }
}

// PROCESS INPUTS
void debounceSwitches()
{
    REPEAT_MS( 100 )
    {
        for( int i = 0 ; i < 4 ; i ++ )
        {
            button[i].debounce() ;   
        }
    } 
    END_REPEAT
}

void processSwitches() // N.B, 2x occupancy and 1x switch 4 recording control
{
    const int SHORT = 1 ;
    const int  LONG = 2 ;
    static uint8    discardRising ;
    static uint32   prevTime ;
    static uint8    pressTime ;

    uint8 state = button[0].getState() ;
    if( state == FALLING )
    {
        prevTime = millis() ;
        discardRising = 0 ;
    }

    if( state == LOW && discardRising == 0)
    {
        if( millis() - prevTime >= 2000 )
        {
            discardRising = 1 ;
            event.startRecording() ;
        }
    }

    if( state == RISING && discardRising == 1 )
    {
        if(      event.getState() ==    idle ) event.startPlaying() ;
        else if( event.getState() == playing ) event.stopPlaying() ;
    }
}

void debouncePCF()
{
    REPEAT_MS( 100)
    {
        Wire.requestFrom( PCFaddress, 1 ) ;
        uint8 inputs = Wire.read() ;

        for( int i = 0 ; i < 8 ; i ++ )
        {
            pcfInput[i].debounce( (inputs >> i) & 1 ) ;
        }
    }
    END_REPEAT
}

void processPCF()
{
    uint8 state = pcfInput[5].getState() ;
    if( state == FALLING ) { digitalWrite( relay1, HIGH ) ; event.storeEvent( relayEvent, relay1, 1 ) ; }
    if( state == RISING  ) { digitalWrite( relay1,  LOW ) ; event.storeEvent( relayEvent, relay1, 0 ) ; }

    state = pcfInput[6].getState() ;
    if( state == FALLING ) { digitalWrite( relay2, HIGH ) ; event.storeEvent( relayEvent, relay2, 1 ) ; }
    if( state == RISING  ) { digitalWrite( relay2,  LOW ) ; event.storeEvent( relayEvent, relay2, 0 ) ; }

    for( int i = 0 ; i < 4 ; i ++ )         // first four buttons should control the four servos
    {
        state = pcfInput[i].getState() ;
        if( state == FALLING ) { servo[i].setState( 1 ) ; event.storeEvent( servoEvent, i, 1 ) ; }
        if( state == RISING  ) { servo[i].setState( 0 ) ; event.storeEvent( servoEvent, i, 1 ) ; }
    }
}

void sweepServos()
{
    for( int i = 0 ; i < 4 ; i ++ )
    {
        servo[i].sweep() ;
    }
}

void blinkLeds() // note PWM1 is used for throttle. Should shine nicely when throttle opens.
{
    REPEAT_MS( 500 )
    {
        digitalWrite( PWM2, !digitalRead( PWM2 )) ; // always toggle to show power. 

        if( event.getState() == recording ) { digitalWrite( PWM3, !digitalRead( PWM3 )) ; } // blink
        if( event.getState() == playing )   { digitalWrite( PWM3, HIGH ) ; }                // ON
        if( event.getState() == idle )      { digitalWrite( PWM3,  LOW ) ; }                // OFF
    }
    END_REPEAT
}

void setup()
{
    initIO() ;
    Serial.begin( 115200 ) ;
    Serial.println( version ) ;
    Serial.println( date ) ;

    Wire.begin() ;

    pwm.begin() ;
    pwm.setCurrentSense( POT2, 153 ) ; // 1.5A x 0.5R / 0.00488 = 153

    sm.setState( runMode ) ;
}

void loop()
{
    pwm.update() ;
    readThrottleKnob() ;// reads the throttle potentiometer and processes data
    pwm.update() ;
    speedControl() ;    // handles acceleration and direction relays
    pwm.update() ;
    debounceSwitches() ;
    pwm.update() ;
    processSwitches() ;
    pwm.update() ;
    debouncePCF() ;
    pwm.update() ;
    processPCF() ;
    pwm.update() ;
    sweepServos() ;
    pwm.update() ;
    blinkLeds() ;
    pwm.update() ;

#ifdef AUTO_SHUTTLE
    autoShuttleControl() ; // automatic shutling between 2 sensors.
#elif defined RECORDER
    recorder.update() ;    // uses the all mighty recorder
#endif

}

#ifdef AUTO_SHUTTLE
StateFunction( runMode )
{
    if( button[0].getState() == FALLING 
    ||  button[1].getState() == FALLING ) return 1 ;
    
    return 0 ;
}

StateFunction( braking )
{
    if( sm.entryState() )
    {
        entrySpeed = pwm.getSpeed() ; // store with which speed we started to brake.
        setPoint = 0 ;
    }
    if( sm.onState() )
    {
        if( pwm.getSpeed() == 0 ) sm.exit() ; 
    }

    return sm.endState() ;
}

StateFunction( departing )
{
    if( sm.entryState() )
    {
        setPoint = -entrySpeed ;
    }
    if( sm.onState() )
    {
        if( pwm.getSpeed() == -entrySpeed ) sm.exit() ;   
    }
    return sm.endState() ;
}


uint8_t autoShuttleControl()
{
    STATE_MACHINE_BEGIN( sm )
    {
        State( runMode ) {
            sm.nextState( braking, 0 ) ; }
    
        State( braking ) {
            sm.nextState( departing, 20000 ) ; } // 20s delay for reversing
    
        State( departing ) {
            sm.nextState( runMode , 8000 ) ; } // 8 seconds lockout for sensors
    }
    STATE_MACHINE_END( sm )
}
#endif

// call back message from slave unit.
void notifySendInput( uint8 pin, uint8 state )
{

}