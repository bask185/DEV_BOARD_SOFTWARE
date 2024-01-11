#include "TSCbus.h"

/*     HEADER          |        SLAVE 1 MESSAGE         |        SLAVE X MESSAGE         |
     ID | SLAVE COUNT  |  <OPCODE> <DATAx>  <CHECKSUM>  |  <OPCODE> <DATAx>  <CHECKSUM>  |

A complete packet exists out of a two-byte header and several messages, one for every slave.

The header exists out of an ID followed by the amount of connected slaves. The master dictates this number.
Every slave increments the ID byte for the next slave. The ID is used to determen which of the messages is ment for that slave.

Every slave relays every byte as fast as possible. A slave may read out his instructions to control outputs or it may set bits in his message
in order to relay INPUT status back to the master. If a slave is processing a message which is not for him, it simply relays the bytes over the serial bus

A message is split in 3 parts. The OPCODE, payload and Checksum

OPCODE contains packet type and message length for eevry slave. 
First nibble is the command, 2nd nibble is the length.

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
OPC_SET_DATA    = 0x91

Set OPCODES may either set or toggle the outputs. Regardsless of what object it is.
It does not matter whether a servo or relay is connected to the addressed pin or not.
With the exception of PWM outputs, outputs are either '1' or '0' and must be configurated before hand

OPC_SET_DATA may set more than 1 output in a single message. The length is therefor variable. This is application specific

<OPC_SET_OUTPUT>  <IIIIIISS <CHECKSUM>
IIIIII  = 6 bit pin number, 0 - 63
SS      = 0 -> ouput off
SS      = 1 -> ouput on
SS      = 2 -> ouput toggle

<OPC_SET_PWM> <IIIIIIII> <PPPPPPPP> <CHECKSUM>
IIIIIIII = ID of PWM pin
PPPPPPPP = PWM value

<OPC_SET_DATA> <DDDDDDDD> ... <DDDDDDDD> <CHECKSUM>


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
    transceiveData,
    getChecksum,
    notMyBytes,
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
        index =  1 ;                        // OPCODE has index 1

        if( messageCounter != myID )                // if not my message..
        {
            length = b & 0x0F ;                 // get the length for this OPCODE message
            state = notMyBytes ;
            goto relayByte ;
        }

        message.OPC    = b & 0xF0 ;
        message.length = b & 0x0F ;

        if( message.OPC == OPC_GET_INPUT  & 0xF0  
        ||  message.OPC == OPC_GET_ANALOG & 0xF0 
        ||  message.OPC == OPC_GET_DATA   & 0xF0 ) state = transmittData ; // NOTE transceiveData is the shit2be
        else                                       state = receiveData ;    
        goto relayByte ;

    case notMyBytes:
        if( ++ index == length ) state = getChecksum ; 
        goto relayByte ;

    // how transceiveData works: If received bytes are to be filled with inputs. These bytes are already empty and the payload is already filled
    // therfor we OR the empty bytes with the prepared payload values in order to read inputs
    // IF a received byte is ment for outputs, it contains data. The 'prepared' payload value must be empty for outputs. OR'ing 0 does nothing to the data
    // than the received data is spooned into the payload.
    // this allow you to utilize combined input output messages. It is even possible to combine inputs and outputs in a single byte. 
    // As long as the prepared payload values are OK.

    case transceiveData:
        b |= message.payload[ index ] ;
        message.payload[ index ] = b ;

        if( ++ index == message.length ) state = getChecksum ;
        goto relayByte ;

    case receiveData:
        message.payload[ index ] = b ;

        if( ++ index == message.length ) state = getChecksum ;
        goto relayByte ;

    case transmittData:
        //pinNumber = b >> 1 ;
        b = message.payload[ index ] ; // slave must send something. Fill the byte with prepared value
        if( ++ index == message.length ) state = getChecksum ;
        goto relayByte ;


    case getChecksum:
        message.checksum = b ;
        if( !checkChecksum() ) ; // do something with an error or so.

        if( messageCounter ++ == moduleCount ) 
        {
            messageReceived = 1 ;
            state = waitFirstByte ; // This was the last opcode we had to process, we are finished
        }
        else state = getOPC ;        // there are more messages in this packet
        // fallthrough

    relayByte:
        Serial.write(b) ; 
        break ;
    }
}

/**
 * @brief procces a message after a packet is processed. 
 * Relevant payloads from message must be processed for outputs 
 * New payloads must be prepared for inputs.
 */
void TSCbus::processMessage()
{
    if( messageReceived == 0 ) return ;
    messageReceived = 0 ;

    // get the OUTPUT commands fist!
    switch( message.OPC )
    {
    case 0x10:    // OPC_INIT

        break ;

    case 0xF0:    // OPC_IDLE

        break ;

    case 0x20:    // OPC_SET_OUTPUT

        break ;

    case 0x30:    // OPC_SET_PWM

        break ;

    case 0x40:    // OPC_GET_INPUT

        break ;

    case 0x50:    // OPC_GET_ANALOG

        break ;

    case 0x60:    // OPC_GET_DATA
        // application specific, We have just transmitted INPUT data to master and now we don't have to do anything
        break ;

    case 0x60:    // OPC_CONF_IO

        break ;

    case 0x70:    // OPC_CONF_SERVO

        break ;

    case 0x80:    // OPC_GET_MODULE_TYPE

        break ;

    case 0x90:    // OPC_SET_DATA
        // application specific, do a callback with a pointer to the data bytes and a length?
        break ;  
    }

    // preparePayload() // we dont know in advance what message we get. If we are going to get an input readout we need to prepare the payload with our input thingies.
    //                  // if we are going to get an OUTPUT message, we don't have to prepare payload. We can receive output message at any time.
                        // Note. Input readout should be handled in 2 messages. 

}




GPIO::GPIO( uint8, uint8, uint8 ) ;
void GPIO::init()
{
}

void GPIO::set( uint8 ) 
{
}

void GPIO::get() 
{
}

void GPIO::setPWM( uint8 ) 
{
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