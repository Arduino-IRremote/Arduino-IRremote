// Support routines for SAM processor boards

#include "IRremote.h"
#include "IRremoteInt.h"

#if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)

// "Idiot check"
#ifdef USE_DEFAULT_ENABLE_IR_IN
#error Must undef USE_DEFAULT_ENABLE_IR_IN
#endif

//+=============================================================================
// ATSAMD Timer setup & IRQ functions
//

// following based on setup from GitHub jdneo/timerInterrupt.ino

static void setTimerFrequency(int frequencyHz)
{
	int compareValue = (SYSCLOCK / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
	//Serial.println(compareValue);
	TcCount16* TC = (TcCount16*) TC3;
	// Make sure the count is in a proportional position to where it was
	// to prevent any jitter or disconnect when changing the compare value.
	TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
	TC->CC[0].reg = compareValue;
	//Serial.print("COUNT.reg ");
	//Serial.println(TC->COUNT.reg);
	//Serial.print("CC[0].reg ");
	//Serial.println(TC->CC[0].reg);
	while (TC->STATUS.bit.SYNCBUSY == 1);
}

static void startTimer()
{
	REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3);
	while (GCLK->STATUS.bit.SYNCBUSY == 1); // wait for sync

	TcCount16* TC = (TcCount16*) TC3;

	TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
	while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

	// Use the 16-bit timer
	TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
	while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

	// Use match mode so that the timer counter resets when the count matches the compare register
	TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
	while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

	// Set prescaler to 1024
	//TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024;
	TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV64;
	while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

	setTimerFrequency(1000000 / USECPERTICK);
	
	// Enable the compare interrupt
	TC->INTENSET.reg = 0;
	TC->INTENSET.bit.MC0 = 1;

	NVIC_EnableIRQ(TC3_IRQn);

	TC->CTRLA.reg |= TC_CTRLA_ENABLE;
	while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
}

//+=============================================================================
// initialization
//

void IRrecv::enableIRIn()
{
	// Interrupt Service Routine - Fires every 50uS
	//Serial.println("Starting timer");
	startTimer();
	//Serial.println("Started timer");

	// Initialize state machine variables
	irparams.rcvstate = STATE_IDLE;
	irparams.rawlen = 0;

	// Set pin modes
	pinMode(irparams.recvpin, INPUT);
}

void irs(); // Defined in IRRemote as ISR(TIMER_INTR_NAME)

void TC3_Handler(void)
{
	TcCount16* TC = (TcCount16*) TC3;
	// If this interrupt is due to the compare register matching the timer count
	// we toggle the LED.
	if (TC->INTFLAG.bit.MC0 == 1) {
		TC->INTFLAG.bit.MC0 = 1;
		irs();
	}
}

#endif // defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)