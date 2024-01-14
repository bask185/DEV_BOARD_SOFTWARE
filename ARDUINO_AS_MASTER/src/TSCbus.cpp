#include "TSCbus.h"

/*     HEADER          |        SLAVE 1 MESSAGE         |        SLAVE X MESSAGE         |
     ID | SLAVE COUNT  |  <OPCODE> <DATAx>  <CHECKSUM>  |  <OPCODE> <DATAx>  <CHECKSUM>  |

A complete packet exists out of a two-byte header and several messages, one for every slave.

// LATEST STATUS AND THOUGHTS
  1). I am not sure if I want to retain checksums. When reading inputs, the payload is changed on the fly. So unless you buffer the original data, 
      it is a bit pointless to check the checksum.
  2). THERE IS A BUG. follow up messages tend to screw up the received payloads
  3). I may want to start by sending preambles of lets say 6 bytes with 0xFF. In whatever event that a slave does not get a byte it expects it will not loop or crash indefinately
      Code should always check on preamples and reset the lot


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

SCANNING IO bus. If the master performs an IO scan. It transmitts a module count of 0.
every slave should add a byte to this message, increasing it's length.
When the master receives the message back. The slave ID tells how many slaves are present followed by what which slave is.


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
  //   OPC    P0   P1    P2    P3    P4    P5    P6    P7   XOR |  OPC   13    12   XOR 
enum states
{
    wait4ID,
    getMessageCount,
    IOscan,
    getOPCODE,
    getLength,
    processData,
    notMyBytes,
    getChecksum,
} ;

enum transmittStates
{
    init,
    sendPreamble,
    sendSlaveID,
    sendModuleCount,
    loadMessage,
    sendMessage,
    pickNextMessage,
    finished,
} ;

TSCbus::TSCbus()
{
}


uint8 TSCbus::checkChecksum()
{
    return 1 ;
}


uint8 TSCbus::drive()
{
    if( Serial.availableForWrite() == 0 ) return ; // transmitt buffer Full? return to avoid blocking write.
    
    switch( transmittState )
    {
    case init:
        slaveCounter   = 0 ;   
        if( 1 /*transmissionAllowed*/ ) state = sendSlaveID ; // may become preamble
        break ;

    case sendPreamble: // NOT YET IN USE
        Serial.write( 0xFF ) ;
        Serial.write( 0xFF ) ;
        Serial.write( 0xFF ) ;
        Serial.write( 0xFF ) ;
        Serial.write( 0xFF ) ;
        Serial.write( 0xFF ) ;
        break ;

    case sendSlaveID:
        Serial.write( 1 ) ; // first slave has ID 1
        state = sendModuleCount ;
        break ;

    case sendModuleCount:
        if( scanningBus )
        {
            Serial.write( 0 ) ; // a count of 0 means this is a bus scan. There will be no follow up bytes and slaves must add their types to the chain
            state = finished ;
        }
        else
        {
            Serial.write( slaveCount ) ; // we either get this number from predefined message or a bus readout
            state = loadMessage ;
        }
        break ;
    
    case loadMessage:
        if(notifyloadMessage) notifyloadMessage( &message, slaveCounter ) ; // load the message
        length = message.OPCODE & 0x0F ;
        byteCounter = 0 ;
        state  = sendMessage ;
        break ;

    case sendMessage:
        Serial.write( message.payload[ byteCounter ] ) ; // send message for a slave.
        if( ++ byteCounter == length ) state = pickNextMessage ;
        break ;

    case pickNextMessage:
        if( ++ slaveCounter == slaveCount ) // all messages sent
        { 
            state = finished ;
        }
        else
        {
            byteCounter = 0 ;
            if(notifyloadMessage) notifyloadMessage( &message, slaveCounter ) ;    
        }           
        break ;

    case finished:
        // do something or not?
        state = init ;
        break ;
    }
}

/**
 * @brief receives, process and relays incomming bytes
 * The master does not need to relay bytes but merely process input variables
 */

void TSCbus::transceiveMessage() 
{
    if( Serial.available() == 0 ) return ;

    uint8 b = Serial.read() ; // remove '0'
    uint8 pinNumber ;
    uint8 pinState ;

    switch (state)
    {
    case wait4ID:                    // wait for first byte
        myID = b ;                         // the first byte of the message carries my ID
        index = 0 ;
        messageCounter = 1 ;               // reset the byte counter

        state = getMessageCount ;
        b ++ ;                          // increment the slave ID for the next slave before relaying
        goto relayByte ;


    case getMessageCount:                // get the amount of messages inside this package
        messageCount = b ;

        if( messageCount > 0 )
        { 
            state = getOPCODE ;
            goto relayByte ;
        }
        state = IOscan ; 
    // fall through 

    case IOscan:         // pass on board type bytes from previous slaves NOTE TEST ME
        if( ++index == myID ) 
        {
            if( notifyGetBoardType ) b = notifyGetBoardType() ; // every board should add it's own type to the message.
            state = wait4ID ;
        }
        goto relayByte ;
        

    case getOPCODE:
        index =  0 ;                        // reset index
        length = b & 0x0F ;                 // get the length for this OPCODE message

        if( messageCounter != myID )        // if not my message..
        {
            if( length == 0x0F ) state = getLength ;  // if length is 16, the following byte will contain a new message length.
            else                 state = notMyBytes ;
            goto relayByte ;
        }
        // message is for me
        message.OPCODE = b & 0xF0 ;
        
        if( length == 0x0F ) state = getLength ;   // if length is 16, the following byte will contain a new message length.
        else
        {
            message.length = length ;
            state = processData ;
        }
        goto relayByte ;

    case getLength:
        message.length = length = b ;
        if( messageCounter != myID ) state = notMyBytes ;
        else                         state = processData ;
        goto relayByte ;

    case notMyBytes:
        if( ++ index == length ) state = getChecksum ; 
        goto relayByte ;

    case processData:
        if( notifyGetPayload )  // for inputs OR input states on the byte
        {
            b |= notifyGetPayload( message.OPCODE, index ) ; // if applicable get payload data from application such as inputs. NOTE. We may want to send OPC as well
        }
        message.payload[index] = b ;

        if( ++ index == length ) state = getChecksum ;
        goto relayByte ;

    case getChecksum:               // NOTE. we could do without checksum.
        message.payload[index] = b ;
        if( !checkChecksum() ) ; // do something with an error or so.

        if( messageCounter ++ == messageCount ) 
        {
            processOutputs() ;
            state = wait4ID ; // This was the last opcode we had to process, we are finished
        }
        else 
        {
            state = getOPCODE ;        // there are more messages in this packet
        }
        // fallthrough

    relayByte:
        // if master do not relay bytes
        Serial.write( b ) ;
        // Serial.println( b ) ;
        break ;
    }
}

/**
 * @brief procces a message after a packet is processed. 
 * Relevant payloads from message must be processed for outputs 
 * New payloads must be prepared for inputs.
 */
void TSCbus::processOutputs()
{
    uint8 OPCODE = message.OPCODE & 0xF0 ;

    switch( OPCODE )
    {                                                       // pin                   // data
    case 0x20: if(   notifySetOutput )   notifySetOutput( message.payload[0], message.payload[1] ) ; break ; 
    case 0x30: if(      notifySetPwm )      notifySetPwm( message.payload[0], message.payload[1] ) ; break ; 
    case 0x70: if( notifyServoConfig ) notifyServoConfig( message.payload[0], message.payload[1] ) ; break ;
    case 0x80: if(    notifySetServo )    notifySetServo( message.payload[0], message.payload[1] ) ; break ; 
    case 0x90: if(     notifySetData )     notifySetData( &message ) ; break ; // we send a pointer to the message object. The function can spoon out the raw data and do application stuff withit
    case 0x60: if(   notifyConfigPin )   notifyConfigPin( &message ) ; break ; 
    }
}


