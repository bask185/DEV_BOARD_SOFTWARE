#include "src/macros.h"
#include "src/TSCbus.h"
#include "src/BOARDS.h"


/*
    goal of project;
    This software must use the arduino board type object and the TSCbus to interract with an arduino slave unit.
*/
ARDUINO_BOARD slave1, slave2 ;
TSCbus bus ;

void notifyloadMessage( Message *message, uint8 slaveIndex )
{
    if( slaveIndex == 0 ) // first slave
    {

    }
    if( slaveIndex == 1 ) // second slave
    {

    }
    message->OPCODE = 
}

void setup()
{
    Serial.begin( 115200 ) ;
}

void loop()
{
    bus.drive() ;
}
