
//==============================================================================
//    L       EEEEEE   EEEE    OOOO
//    L       E       E       O    O
//    L       EEEE    E  EEE  O    O
//    L       E       E    E  O    O    LEGO Power Functions
//    LLLLLL  EEEEEE   EEEE    OOOO     Copyright (c) 2016, 2017 Philipp Henkel
//==============================================================================

//+=============================================================================
//

class LegoPfBitStreamEncoder {
 private:
  uint16_t data;
  bool repeatMessage;
  uint8_t messageBitIdx;
  uint8_t repeatCount;
  uint16_t messageLength;

 public:
  // HIGH data bit = IR mark + high pause
  // LOW data bit = IR mark + low pause
  static const uint16_t LOW_BIT_DURATION = 421;
  static const uint16_t HIGH_BIT_DURATION = 711;
  static const uint16_t START_BIT_DURATION = 1184;
  static const uint16_t STOP_BIT_DURATION = 1184;
  static const uint8_t IR_MARK_DURATION = 158;
  static const uint16_t HIGH_PAUSE_DURATION = HIGH_BIT_DURATION - IR_MARK_DURATION;
  static const uint16_t LOW_PAUSE_DURATION = LOW_BIT_DURATION - IR_MARK_DURATION;
  static const uint16_t START_PAUSE_DURATION = START_BIT_DURATION - IR_MARK_DURATION;
  static const uint16_t STOP_PAUSE_DURATION = STOP_BIT_DURATION - IR_MARK_DURATION;
  static const uint8_t MESSAGE_BITS = 18;
  static const uint16_t MAX_MESSAGE_LENGTH = 16000;

  void reset(uint16_t data, bool repeatMessage) {
    this->data = data;
    this->repeatMessage = repeatMessage;
    messageBitIdx = 0;
    repeatCount = 0;
    messageLength = getMessageLength();
  }

  int getChannelId() const { return 1 + ((data >> 12) & 0x3); }

  uint16_t getMessageLength() const {
    // Sum up all marks
    uint16_t length = MESSAGE_BITS * IR_MARK_DURATION;

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

  uint8_t getMarkDuration() const { return IR_MARK_DURATION; }

  uint32_t getPauseDuration() const {
    if (messageBitIdx == 0)
      return START_PAUSE_DURATION;
    else if (messageBitIdx < MESSAGE_BITS - 1) {
      return getDataBitPause();
    } else {
      return getStopPause();
    }
  }

 private:
  uint16_t getDataBitPause() const {
    const int pos = MESSAGE_BITS - 2 - messageBitIdx;
    const bool isHigh = data & (1 << pos);
    return isHigh ? HIGH_PAUSE_DURATION : LOW_PAUSE_DURATION;
  }

  uint32_t getStopPause() const {
    if (repeatMessage) {
      return getRepeatStopPause();
    } else {
      return STOP_PAUSE_DURATION;
    }
  }

  uint32_t getRepeatStopPause() const {
    if (repeatCount == 0 || repeatCount == 1) {
      return STOP_PAUSE_DURATION + (uint32_t)5 * MAX_MESSAGE_LENGTH - messageLength;
    } else if (repeatCount == 2 || repeatCount == 3) {
      return STOP_PAUSE_DURATION
             + (uint32_t)(6 + 2 * getChannelId()) * MAX_MESSAGE_LENGTH - messageLength;
    } else {
      return STOP_PAUSE_DURATION;
    }
  }
};
