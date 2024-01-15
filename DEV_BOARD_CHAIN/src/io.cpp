#include <Arduino.h>
#include "io.h"
extern void initIO(void) {
	pinMode( Rx, 		INPUT );
	pinMode( Tx, 		OUTPUT );
	pinMode( DCC_PIN, 	INPUT );
	pinMode( PWM1, 		OUTPUT );
	pinMode( SW1, 		INPUT_PULLUP );
	pinMode( PWM2, 		OUTPUT );
	pinMode( PWM3, 		OUTPUT );
	pinMode( SW2, 		INPUT_PULLUP );
	pinMode( SW3, 		INPUT_PULLUP );
	pinMode( SW4, 		INPUT_PULLUP );
	pinMode( servo1, 	OUTPUT );
	pinMode( servo2, 	OUTPUT );
	pinMode( servo3, 	OUTPUT );
	pinMode( servo4, 	OUTPUT );
	pinMode( relay1, 	OUTPUT );
	pinMode( relay2, 	OUTPUT );
	pinMode( relay3, 	OUTPUT );
	pinMode( relay4, 	OUTPUT );
	pinMode( SDA, 		INPUT_PULLUP );
	pinMode( SCL, 		INPUT_PULLUP );
	pinMode( POT1, 		INPUT );
	pinMode( POT2, 		INPUT );
}