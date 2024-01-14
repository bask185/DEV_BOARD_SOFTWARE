#include "src/macros.h"
#include "src/TSCbus.h"
#include "src/io.h"
#include "src/BOARDS.h"
#include "src/ServoSweep.h"
#include "src/debounceClass.h"
#include <Wire.h>

/* TASKS 
Read out I2C bus for MCP23017 readouts
run packet manager 
run the TSCbus
*/

/* create slaves*/
static STEPPER_MOTOR_BOARD  b0 = STEPPER_MOTOR_BOARD() ; 
static DIGITAL_INPUT_BOARD  b1 = DIGITAL_INPUT_BOARD() ;
static DIGITAL_OUTPUT_BOARD b2 = DIGITAL_OUTPUT_BOARD() ;
static ANALOG_INPUT_BOARD   b3 = ANALOG_INPUT_BOARD() ;
static ENCODER_BOARD        b4 = ENCODER_BOARD() ;
static JACK_OF_ALL_TRADES   b5 = JACK_OF_ALL_TRADES() ;

BOARD *board[] =
{
    &b0 ,
    &b1 ,
    &b2 ,
    &b3 ,
    &b4 ,
} ;
const int nBoards = 5 ;

TSCbus bus ;

void setup()
{
    Serial.begin( 115200 ) ;
    bus.begin() ;
}

void updateBus()
{
    // assemble and transmitt message
    packet.append( 1 ) ;        // assemble header. (preamble) ID and board count
    packet.append( nBoards ) ;

    for( int i = 0 ; i < nBoards ; i ++ )
    {
        packet.append( board[i]->getMessage() ) ;
    }

}

void loop()
{
    
}
