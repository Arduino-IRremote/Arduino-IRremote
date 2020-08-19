#if defined(ARDUINO_ARCH_SAMD)
// Support routines for SAM processor boards

#include "IRremote.h"

// "Idiot check"
#ifdef USE_DEFAULT_ENABLE_IR_IN
#error Must undef USE_DEFAULT_ENABLE_IR_IN
#endif

//+=============================================================================
// ATSAMD Timer setup & IRQ functions
//

// following based on setup from GitHub jdneo/timerInterrupt.ino

static void setTimerFrequency(int frequencyHz) {
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
    while (TC->STATUS.bit.SYNCBUSY == 1)
        ;
}

static void startTimer() {
    REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3);
    while (GCLK->STATUS.bit.SYNCBUSY == 1)
        ; // wait for sync

    TcCount16* TC = (TcCount16*) TC3;

    // The TC should be disabled before the TC is reset in order to avoid undefined behavior.
    TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
    // When write-synchronization is ongoing for a register, any subsequent write attempts to this register will be discarded, and an error will be reported.
    while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
    // Reset TCx
    TC->CTRLA.reg = TC_CTRLA_SWRST;
    // When writing a ‘1’ to the CTRLA.SWRST bit it will immediately read as ‘1’.
    // CTRL.SWRST will be cleared by hardware when the peripheral has been reset.
    while (TC->CTRLA.bit.SWRST)
        ;

    // Use the 16-bit timer
    // Use match mode so that the timer counter resets when the count matches the compare register
    // Set prescaler to 64
    TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV64 | TC_CTRLA_ENABLE;

    setTimerFrequency(1000000 / MICROS_PER_TICK);

    // Enable the compare interrupt
    TC->INTENSET.reg = 0;
    TC->INTENSET.bit.MC0 = 1;

    NVIC_EnableIRQ (TC3_IRQn);

}

//+=============================================================================
// initialization
//

void IRrecv::enableIRIn() {
    // Interrupt Service Routine - Fires every 50uS
    //Serial.println("Starting timer");
    startTimer();
    //Serial.println("Started timer");

    // Initialize state machine variables
    irparams.rcvstate = IR_REC_STATE_IDLE;
    irparams.rawlen = 0;

    // Set pin modes
    pinMode(irparams.recvpin, INPUT);
}

void IRrecv::disableIRIn() {
    TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
}

void IRTimer(); // Defined in IRRemote as ISR(TIMER_INTR_NAME)

void TC3_Handler(void) {
    TcCount16* TC = (TcCount16*) TC3;
    // If this interrupt is due to the compare register matching the timer count
    // we toggle the LED.
    if (TC->INTFLAG.bit.MC0 == 1) {
        TC->INTFLAG.bit.MC0 = 1;
        IRTimer();
    }
}

#endif // defined(ARDUINO_ARCH_SAMD)
