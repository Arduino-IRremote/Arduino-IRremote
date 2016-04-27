#include "IRremote.h"
#include "IRremoteInt.h"

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

class BitStreamEncoder {
 private:
  uint16_t data;
  bool repeatMessage;
  int messageBitIdx;
  int repeatCount;
  int messageLength;

  // HIGH data bit = IR mark + high pause
  // LOW data bit = IR mark + low pause
  static const int LOW_BIT_DURATION = 421;
  static const int HIGH_BIT_DURATION = 711;
  static const int START_BIT_DURATION = 1184;
  static const int STOP_BIT_DURATION = 1184;
  static const int IR_MARK_DURATION = 158;
  static const int HIGH_PAUSE_DURATION = HIGH_BIT_DURATION - IR_MARK_DURATION;
  static const int LOW_PAUSE_DURATION = LOW_BIT_DURATION - IR_MARK_DURATION;
  static const int START_PAUSE_DURATION = START_BIT_DURATION - IR_MARK_DURATION;
  static const int STOP_PAUSE_DURATION = STOP_BIT_DURATION - IR_MARK_DURATION;
  static const int MESSAGE_BITS = 18;
  static const int MAX_MESSAGE_LENGTH = 16000;

 public:
  void reset(uint16_t data, bool repeatMessage) {
    this->data = data;
    this->repeatMessage = repeatMessage;
    messageBitIdx = 0;
    repeatCount = 0;
    messageLength = getMessageLength();
  }

  int getChannelId() const { return 1 + ((data >> 12) & 0x3); }

  int getMessageLength() const {
    // Sum up all marks
    int length = MESSAGE_BITS * IR_MARK_DURATION;

    // Sum up all pauses
    length += START_PAUSE_DURATION;
    for (unsigned long mask = 1UL << 15; mask; mask >>= 1) {
      if (data & mask) {
        length += HIGH_PAUSE_DURATION;
      } else {
        length += LOW_PAUSE_DURATION;
      }
    }
    length += STOP_PAUSE_DURATION;
    return length;
  }

  boolean next() {
    messageBitIdx++;
    if (messageBitIdx >= MESSAGE_BITS) {
      repeatCount++;
      messageBitIdx = 0;
    }
    if (repeatCount >= 1 && !repeatMessage) {
      return false;
    } else if (repeatCount >= 5) {
      return false;
    } else {
      return true;
    }
  }

  int getMarkDuration() const { return IR_MARK_DURATION; }

  int getPauseDuration() const {
    if (messageBitIdx == 0)
      return START_PAUSE_DURATION;
    else if (messageBitIdx < MESSAGE_BITS - 1) {
      return getDataBitPause();
    } else {
      return getStopPause();
    }
  }

 private:
  int getDataBitPause() const {
    const int pos = MESSAGE_BITS - 2 - messageBitIdx;
    const bool isHigh = data & (1 << pos);
    return isHigh ? HIGH_PAUSE_DURATION : LOW_PAUSE_DURATION;
  }

  int getStopPause() const {
    if (repeatMessage) {
      return getRepeatStopPause();
    } else {
      return STOP_PAUSE_DURATION;
    }
  }

  int getRepeatStopPause() const {
    if (repeatCount == 0 || repeatCount == 1) {
      return STOP_PAUSE_DURATION + 5 * MAX_MESSAGE_LENGTH - messageLength;
    } else if (repeatCount == 2 || repeatCount == 3) {
      return STOP_PAUSE_DURATION
             + (6 + 2 * getChannelId()) * MAX_MESSAGE_LENGTH - messageLength;
    } else {
      return STOP_PAUSE_DURATION;
    }
  }
};

void IRsend::sendLegoPowerFunctions(uint16_t data, bool repeat)
{
  enableIROut(38);
  static BitStreamEncoder bitStreamEncoder;
  bitStreamEncoder.reset(data, repeat);
  do {
    mark(bitStreamEncoder.getMarkDuration());
    space(bitStreamEncoder.getPauseDuration());
  } while (bitStreamEncoder.next());
}
#endif
