#include "TSCbus.h"

/*     HEADER          |        SLAVE 1 MESSAGE         |        SLAVE X MESSAGE         |
     ID | SLAVE COUNT  |  <OPCODE> <DATAx>  <CHECKSUM>  |  <OPCODE> <DATAx>  <CHECKSUM>  |

Opcode contains packet type and message length for eevry slave. 
First nibble is the command, 2nd nibble is the length.

The slave count is transmitted as well so all slaves now when a complete package
is sent

XOR is NOT counted in the package length. The OPCODE itself is part of package length

XOR is assembled using all previous bytes except the XOR itself

Slaves need to be told in advanced how they are configurated. 
This allow slave to keep INPUTS up to date so no time is wasted during transmissions.

Same is true for servo objects. These need configurations as well.
Servo objects need to make use of EEPROM. A max of 10 is allowed due to servo library




Messages:

--------------------------------------------------------------------------------
OPC_INIT        = 0X11

Initialize slaves and used by master to get slave amount. If no feedback is received the master assumes OUTPUT only mode 
and will refer to I2C readout to determen slave count. If I2C readout and slave count mismatch, error should be tossed

<OPC_INIT>  <CHECKSUM>


--------------------------------------------------------------------------------
OPC_IDLE        = 0XF1

Idle packets may be sent if slave does not have to do anything at all.

<OPC_IDLE>  <CHECKSUM>


CONTROL OUTPUTS
--------------------------------------------------------------------------------
OPC_SET_OUTPUT  = 0X22
OPC_SET_PWM     = 0X33

Set OPCODES may either set or toggle the outputs. Regardsless of what object it is.
It does not matter whether a servo or relay is connected to the addressed pin or not.
With the exception of PWM outputs, outputs are either '1' or '0' and must be configurated before hand

<OPC_SET_OUTPUT>  <IIIIIISS <CHECKSUM>
IIIIII  = 6 bit pin number, 0 - 63
SS      = 0 -> ouput off
SS      = 1 -> ouput on
SS      = 2 -> ouput toggle

<OPC_SET_PWM> <IIIIIIII> <PPPPPPPP> <CHECKSUM>
IIIIIIII = ID of PWM pin
PPPPPPPP = PWM value


READ INPUTS
--------------------------------------------------------------------------------
OPC_GET_INPUT   = 0X42
OPC_GET_ANALOG  = 0X53
OPC_GET_DATA    = 0X6x  

GET input may fetch the input values of either the digital or analog inputs
The latter needs an extra byte for the 10 bit VALUES
Get DATA may read in several bytes at once. This would be hardware specific.
The GPIO_UNIT for example could send just 1 byte with all 4 buttons in a single
packet which would otherwise 4 message cycles

<OPC_GET_INPUT>  <IIIIIIID> <CHECKSUM>                  // FILLED BY SLAVE
IIIIIIx  = 6 bit pin number, 0 - 63
D        = pin state 1 or 0

<OPC_GET_ANALOG>  <IIIIIxAA> <AAAAAAAA> <CHECKSUM>      // FILLED BY SLAVE
IIIIII      = analog pin number (A0, A63 (theoretically, there are only so many analog pins)
AA AAAAAAAA = 10 bit value

<OPC_GET_DATA> <DDDDDDDD> ... <DDDDDDDD> <CHECKSUM>     // FILLED BY SLAVE


--------------------------------------------------------------------------------
OPC_CONF_IO         = 0X62
OPC_CONF_SERVO      = 0X76
OPC_GET_MODULE_TYPE = 0x82

Configuration of IO.
Outputs be          OUTPUT
Digital inputs be   INPUT_PULLUP 
Analog  inputs be   INPUT

Servos  contain, pin, min, max, speed, turnoff, relay 

Reading the slave's module type can tell the master what is connected. The master
than knows in advance what it should or should not do with the slave. 
The opcode exists, but it not needed. You could use it to see if the bus
has what it should have 


<OPC_CONF_IO> <IIIIIIDD> <CHECKSUM>
IIIIII = pin number 0-63
DD = 0 -> INPUT        (analog input)
DD = 1 -> OUTPUT       (digital output)
DD = 2 -> INPUT_PULLUP (digital input)
DD = 3 -> Free for future use. (boards with DAC perhaps?)

<OPC_CONF_SERVO> <xxIIIIII> <NNNNNNNN> <XXXXXXXX> <SSSSSSSS> <TxRRRRRR> <CHECKSUM>
IIIIII   = pin number 0-63
NNNNNNNN = default mininum position
XXXXXXXX = default maximum position
SSSSSSSS = step time in ms between degrees. Higher number -> slower movement
T        = turnoff, kill signal when servo is position
RRRRRR   = relay pin number 0-62 (for frogs)
    Note  relay pin 63 means no relay present 

<OPC_GET_MODULE_TYPE> <TTTTTTTT> <CHECKSUM>
TTTTTTTT = module type
1 -> TS Jack Off All Trade DEV BOARD
2 -> future use?


*/

enum states
{
    waitFirstByte,
    getModuleCount,
    getOPC,
    receiveData,
    transmittData,
    getChecksum,
} ;

TSCbus::TSCbus()
{
}

uint8 TSCbus::checkChecksum()
{
    return 1 ;
}


void TSCbus::transceiveMessage() // <-- slave unit only
{
    if( Serial.available() == 0 ) return ;

    uint8 b = Serial.read() ; // remove '0'
    uint8 pinNumber ;
    uint8 pinState ;

    switch (state)
    {
    case waitFirstByte:                    // wait for first byte
        myID = b ;                         // the first byte of the message carries my ID

        messageCounter = 1 ;               // reset the byte counter
        lastOPC        = 0 ;               // reset flag NOTY IN USE ATM

        state = getModuleCount ;
        b ++ ;                          // increment the slave ID for the next slave before relaying
        goto relayByte ;


    case getModuleCount:                // get the amount of modules from master. This is used to count the opcodes so we know when the message is finished
        moduleCount = b ;
        if( moduleCount == 0 ) state = waitFirstByte ; // if the module count is 0, the master is inspecting the bus and can count slaves.
        else                   state = getOPC ;        // otherwise, get the opcode of the first message

        goto relayByte ;


// SLAVE NODE
    case getOPC:
        message.OPC    = b ;
        message.length = b & 0x0F ;                 // get the length for this OPCODE message
        message.index =  1 ;                        // OPCODE has index 1 so..

        if( message.OPC == OPC_GET_INPUT 
        ||  message.OPC == OPC_GET_ANALOG 
        ||  message.OPC == OPC_GET_DATA )   

              state = transmittData ;                 // Slave must  SEND information
        else  state = receiveData ;                   // slave must RECEIVE information

        goto relayByte ;


    case receiveData:
        if( messageCounter == myID )            // if this is my OPCODE
        {
            message.payload[ message.index ] = b ;
        }

        if( ++ message.index == message.length ) state = getChecksum ;
        goto relayByte ;


    case transmittData:
        if( messageCounter == myID )            // NOTE this need alteration. 
        {                                       // only now we receive our pinnumber to read from. If slave has 20 inputs, it cannot know
            pinNumber = b >> 1 ;

            message.payload[ message.index ] = digitalRead( pinNumber ) ;


            b = message.payload[ message.index ] ; // slave must send something. Fill the byte
        }
        else
        {
            // printNumberln( "not my ID ", messageCounter ) ;
        }
        if( ++ message.index == message.length ) state = getChecksum ;
        goto relayByte ;


    case getChecksum:
        message.checksum = b ;
        // printNumberln( "comparing checksum: ", b ) ;
        if( !checkChecksum() ) // Serial.print("wrong checksum bruh.. fock off :-P");

        if( messageCounter ++ == moduleCount ) // This was the last opcode we had to process, we are finished
        {
            // Serial.println("message relayed") ;
            state = waitFirstByte ;
        }
        else
        {
            // Serial.println("getting next OPCODE") ;
            state = getOPC ;
        }// fallthru

    relayByte:
        Serial.write(b) ; 
        // // Serial.println(b) ;
        break ;
    }
}

/*
static HardwareSerial*   hwPort = nullptr ;
static SoftwareSerial*   swPort = nullptr ;
static Stream*           mySerial = nullptr ;

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
    sendMessage( message ) ;
}
*/