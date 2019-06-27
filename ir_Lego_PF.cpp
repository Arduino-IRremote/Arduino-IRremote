#include "IRremote.h"
#include "IRremoteInt.h"
#include "ir_Lego_PF_BitStreamEncoder.h"

//==============================================================================
//    L       EEEEEE   EEEE    OOOO
//    L       E       E       O    O
//    L       EEEE    E  EEE  O    O
//    L       E       E    E  O    O    LEGO Power Functions
//    LLLLLL  EEEEEE   EEEE    OOOO     Copyright (c) 2016 Philipp Henkel
//==============================================================================

// Supported Devices
// LEGO® Power Functions IR Receiver 8884

//+=============================================================================
//
#if SEND_LEGO_PF

#if DEBUG
namespace {
void logFunctionParameters(uint16_t data, bool repeat) {
  DBG_PRINT("sendLegoPowerFunctions(data=");
  DBG_PRINT(data);
  DBG_PRINT(", repeat=");
  DBG_PRINTLN(repeat?"true)" : "false)");
}
} // anonymous namespace
#endif // DEBUG

void IRsend::sendLegoPowerFunctions(uint16_t data, bool repeat)
{
#if DEBUG
  ::logFunctionParameters(data, repeat);
#endif // DEBUG

  enableIROut(38);
  static LegoPfBitStreamEncoder bitStreamEncoder;
  bitStreamEncoder.reset(data, repeat);
  do {
    mark(bitStreamEncoder.getMarkDuration());
    space(bitStreamEncoder.getPauseDuration());
  } while (bitStreamEncoder.next());
}

#endif // SEND_LEGO_PF
