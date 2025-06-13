/*
  Copyright (c) 2014-2015 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "variant.h"
#include "Arduino.h"
/*
 * Pins descriptions
 */
const PinDescription g_APinDescription[]=
{
  // 0..1 are unused by default (pins in use by 32.768KHz crystal, which is used by the Arduino core)
	{ PORTA,  0, PIO_DIGITAL, (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },
	{ PORTA,  1, PIO_DIGITAL, (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },

	// 2..3 - Analogue inputs - VIN FB and CT
	{ PORTA,  2, PIO_ANALOG,  (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG		           ), ADC_Channel0,   NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },
	{ PORTA,  3, PIO_ANALOG,  (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG 		       ), ADC_Channel1,   NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },
	
	// 4..8 - Digital inputs - Enable and Interlocks
	{ PORTA,  4, PIO_ANALOG,  (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG 		       ), ADC_Channel4,   NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_4 },
	{ PORTA,  5, PIO_ANALOG,  (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG 		       ), ADC_Channel5,   NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_5 },
	{ PORTA,  6, PIO_ANALOG,  (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG 		       ), ADC_Channel6,   NOT_ON_PWM, NOT_ON_TIMER,	EXTERNAL_INT_6 },
	{ PORTA,  7, PIO_ANALOG,  (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG 		       ), ADC_Channel7,   NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_7 },
	{ PORTA,  8, PIO_SERCOM_ALT, (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG 		       ), ADC_Channel16,  NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NMI },
	
	// 9..11 - Digital outputs - Contactor, Status and Fault
	{ PORTA,  9, PIO_SERCOM_ALT, (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG 		       ), ADC_Channel17,  NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_9 },
	{ PORTA, 10, PIO_ANALOG,  (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG 		       ), ADC_Channel18,  NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },
	{ PORTA, 11, PIO_ANALOG,  (PIN_ATTR_DIGITAL|PIN_ATTR_ANALOG 		       ), ADC_Channel19,  NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },

	// 12..13 - pins don't exist
	{ NOT_A_PORT,  0, PIO_NOT_A_PIN, PIN_ATTR_NONE								, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // Unused
	{ NOT_A_PORT,  0, PIO_NOT_A_PIN, PIN_ATTR_NONE								, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // Unused

	// 14 - Spare 1
	{ PORTA, 14, PIO_DIGITAL, (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_14 },
	
	// 15..17 - Status LED Red, Green and Blue
	{ PORTA, 15, PIO_DIGITAL, (PIN_ATTR_DIGITAL								   ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // Red
	{ PORTA, 16, PIO_SERCOM,  (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER	   ), No_ADC_Channel, PWM2_CH0,   TCC2_CH0, 	EXTERNAL_INT_NONE }, // Green
	{ PORTA, 17, PIO_SERCOM,  (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER	   ), No_ADC_Channel, PWM2_CH1,   TCC2_CH1, 	EXTERNAL_INT_NONE }, // Blue
	
	// 18..19 - Enable LED Red and Green
	{ PORTA, 18, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER	   ), No_ADC_Channel, PWM3_CH0,   TC3_CH0, 		EXTERNAL_INT_NONE }, // Red
	{ PORTA, 19, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER	   ), No_ADC_Channel, PWM3_CH1,   TC3_CH1, 		EXTERNAL_INT_NONE }, // Green
	
	// 20..21 pins don't exist
	{ NOT_A_PORT,  0, PIO_NOT_A_PIN, PIN_ATTR_NONE								, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // Unused
	{ NOT_A_PORT,  0, PIO_NOT_A_PIN, PIN_ATTR_NONE								, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // Unused
	
	// 22..23 - Interlock LED Red and Green
	{ PORTA, 22, PIO_SERCOM, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER	   ), No_ADC_Channel, PWM4_CH0,   TC4_CH0, 		EXTERNAL_INT_NONE }, // Red
	{ PORTA, 23, PIO_SERCOM, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER	   ), No_ADC_Channel, PWM4_CH1,   TC4_CH1, 		EXTERNAL_INT_NONE }, // Green

	// 24..26 - USB_NEGATIVE and USB_POSITIVE, pin 26 does not exist
	{ PORTA, 24, PIO_COM,     (PIN_ATTR_NONE                                   ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // USB/DM
	{ PORTA, 25, PIO_COM,     (PIN_ATTR_NONE                                   ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // USB/DP
	{ NOT_A_PORT,  0, PIO_NOT_A_PIN, PIN_ATTR_NONE								, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // Unused

	// 27..29 Spare 2 and 3 / A/CS (pin 29 does not exist)
	{ PORTA, 27, PIO_DIGITAL, (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_15 },
	{ PORTA, 28, PIO_DIGITAL, (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_8 }, 
	{ NOT_A_PORT,  0, PIO_NOT_A_PIN, PIN_ATTR_NONE								, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // Unused

	// 30..31 Digital functions / Debug interface (SWD CLK and SWD IO)
	{ PORTA, 30, PIO_DIGITAL, (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_10 },  // SWD CLK
	{ PORTA, 31, PIO_DIGITAL, (PIN_ATTR_DIGITAL                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_11 },  // SWD IO  
} ;

const void* g_apTCInstances[TCC_INST_NUM+TC_INST_NUM]={ TCC0, TCC1, TCC2, TC3, TC4, TC5 } ;

// Multi-serial objects instantiation
SERCOM sercom0( SERCOM0 ) ;
SERCOM sercom1( SERCOM1 ) ;
SERCOM sercom2( SERCOM2 ) ;
SERCOM sercom3( SERCOM3 ) ;

Uart Serial1( &sercom3, PIN_SERIAL1_RX, PIN_SERIAL1_TX, PAD_SERIAL1_RX, PAD_SERIAL1_TX ) ;

void SERCOM0_Handler()
{
  Serial1.IrqHandler();
}

void initVariant(void) {
  // special initialization code just for us
}
