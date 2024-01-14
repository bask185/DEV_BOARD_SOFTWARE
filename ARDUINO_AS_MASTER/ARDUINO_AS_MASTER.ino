#include "src/macros.h"
#include "src/TSCbus.h"
#include "src/BOARDS.h"
#include <Servo.h>

ARDUINO_BOARD me ;
TSCbus bus ;



void setup()
{
    Serial.begin( 115200 ) ;
}

void loop()
{
    bus.transceiveMessage() ;
}
