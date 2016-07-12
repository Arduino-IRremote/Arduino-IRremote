/*
 * LegoPowerFunctionsTest: LEGO Power Functions Tests
 * Copyright (c) 2016 Philipp Henkel
 */

#include <ir_Lego_PF_BitStreamEncoder.h>

void setup() {
  Serial.begin(9600);
  delay(1000); // wait for reset triggered by serial connection
  runBitStreamEncoderTests();
}

void loop() {
}

void runBitStreamEncoderTests() {
  Serial.println();
  Serial.println("BitStreamEncoder Tests");
  static LegoPfBitStreamEncoder bitStreamEncoder;
  testStartBit(bitStreamEncoder);
  testLowBit(bitStreamEncoder);
  testHighBit(bitStreamEncoder);
  testMessageBitCount(bitStreamEncoder);
  testMessageBitCountRepeat(bitStreamEncoder);
  testMessage407(bitStreamEncoder);
  testMessage407Repeated(bitStreamEncoder);
  testGetChannelId1(bitStreamEncoder);
  testGetChannelId2(bitStreamEncoder);
  testGetChannelId3(bitStreamEncoder);
  testGetChannelId4(bitStreamEncoder);
  testGetMessageLengthAllHigh(bitStreamEncoder);
  testGetMessageLengthAllLow(bitStreamEncoder);
}

void logTestResult(bool testPassed) {
  if (testPassed) {
    Serial.println("OK");
  }
  else {
    Serial.println("FAIL  ############");
  }
}

void testStartBit(LegoPfBitStreamEncoder& bitStreamEncoder) {
  Serial.print("  testStartBit                    ");
  bitStreamEncoder.reset(0, false);
  int startMark = bitStreamEncoder.getMarkDuration();
  int startPause = bitStreamEncoder.getPauseDuration();
  logTestResult(startMark == 158 && startPause == 1184-158);
}

void testLowBit(LegoPfBitStreamEncoder& bitStreamEncoder) {
  Serial.print("  testLowBit                      ");
  bitStreamEncoder.reset(0, false);
  bitStreamEncoder.next();
  int lowMark = bitStreamEncoder.getMarkDuration();
  int lowPause = bitStreamEncoder.getPauseDuration();
  logTestResult(lowMark == 158 && lowPause == 421-158);
}

void testHighBit(LegoPfBitStreamEncoder& bitStreamEncoder) {
  Serial.print("  testHighBit                     ");
  bitStreamEncoder.reset(0xFFFF, false);
  bitStreamEncoder.next();
  int highMark = bitStreamEncoder.getMarkDuration();
  int highPause = bitStreamEncoder.getPauseDuration();
  logTestResult(highMark == 158 && highPause == 711-158);
}

void testMessageBitCount(LegoPfBitStreamEncoder& bitStreamEncoder) {
  Serial.print("  testMessageBitCount             ");
  bitStreamEncoder.reset(0xFFFF, false);
  int bitCount = 1;
  while (bitStreamEncoder.next()) {
    bitCount++;
  }
  logTestResult(bitCount == 18);
}

boolean check(LegoPfBitStreamEncoder& bitStreamEncoder, int markDuration, int pauseDuration) {
  bool result = true;
  result = result && bitStreamEncoder.getMarkDuration() == markDuration;
  result = result && bitStreamEncoder.getPauseDuration() == pauseDuration;
  return result;
}

boolean checkNext(LegoPfBitStreamEncoder& bitStreamEncoder, int markDuration, int pauseDuration) {
  bool result = bitStreamEncoder.next();
  result = result && check(bitStreamEncoder, markDuration, pauseDuration);
  return result;
}

boolean checkDataBitsOfMessage407(LegoPfBitStreamEncoder& bitStreamEncoder) {
  bool result = true;
  result = result && checkNext(bitStreamEncoder, 158, 263);
  result = result && checkNext(bitStreamEncoder, 158, 263);
  result = result && checkNext(bitStreamEncoder, 158, 263);
  result = result && checkNext(bitStreamEncoder, 158, 263);
  result = result && checkNext(bitStreamEncoder, 158, 263);
  result = result && checkNext(bitStreamEncoder, 158, 263);
  result = result && checkNext(bitStreamEncoder, 158, 263);
  result = result && checkNext(bitStreamEncoder, 158, 553);
  result = result && checkNext(bitStreamEncoder, 158, 553);
  result = result && checkNext(bitStreamEncoder, 158, 263);
  result = result && checkNext(bitStreamEncoder, 158, 263);
  result = result && checkNext(bitStreamEncoder, 158, 553);
  result = result && checkNext(bitStreamEncoder, 158, 263);
  result = result && checkNext(bitStreamEncoder, 158, 553);
  result = result && checkNext(bitStreamEncoder, 158, 553);
  result = result && checkNext(bitStreamEncoder, 158, 553);
  return result;
}

void testMessage407(LegoPfBitStreamEncoder& bitStreamEncoder) {
  Serial.print("  testMessage407                  ");
  bitStreamEncoder.reset(407, false);
  bool result = true;
  result = result && check(bitStreamEncoder, 158, 1026);
  result = result && checkDataBitsOfMessage407(bitStreamEncoder);
  result = result && checkNext(bitStreamEncoder, 158, 1026);
  result = result && !bitStreamEncoder.next();
  logTestResult(result);
}

void testMessage407Repeated(LegoPfBitStreamEncoder& bitStreamEncoder) {
  Serial.print("  testMessage407Repeated          ");
  bitStreamEncoder.reset(407, true);
  bool result = true;
  result = result && check(bitStreamEncoder, 158, 1026);
  result = result && checkDataBitsOfMessage407(bitStreamEncoder);
  result = result && checkNext(bitStreamEncoder, 158, 1026 + 5 * 16000 - 10844);
  result = result && checkNext(bitStreamEncoder, 158, 1026);
  result = result && checkDataBitsOfMessage407(bitStreamEncoder);
  result = result && checkNext(bitStreamEncoder, 158, 1026 + 5 * 16000 - 10844);
  result = result && checkNext(bitStreamEncoder, 158, 1026);
  result = result && checkDataBitsOfMessage407(bitStreamEncoder);
  result = result && checkNext(bitStreamEncoder, 158, 1026 + 8 * 16000 - 10844);
  result = result && checkNext(bitStreamEncoder, 158, 1026);
  result = result && checkDataBitsOfMessage407(bitStreamEncoder);
  result = result && checkNext(bitStreamEncoder, 158, 1026 + 8 * 16000 - 10844);
  result = result && checkNext(bitStreamEncoder, 158, 1026);
  result = result && checkDataBitsOfMessage407(bitStreamEncoder);
  result = result && checkNext(bitStreamEncoder, 158, 1026);
  result = result && !bitStreamEncoder.next();
  logTestResult(result);
}

void testMessageBitCountRepeat(LegoPfBitStreamEncoder& bitStreamEncoder) {
  Serial.print("  testMessageBitCountRepeat       ");
  bitStreamEncoder.reset(0xFFFF, true);
  int bitCount = 1;
  while (bitStreamEncoder.next()) {
    bitCount++;
  }
  logTestResult(bitCount == 5*18);
}

void testGetChannelId1(LegoPfBitStreamEncoder& bitStreamEncoder) {
  Serial.print("  testGetChannelId1               ");
  bitStreamEncoder.reset(407, false);
  logTestResult(bitStreamEncoder.getChannelId() == 1);
}

void testGetChannelId2(LegoPfBitStreamEncoder& bitStreamEncoder) {
  Serial.print("  testGetChannelId2               ");
  bitStreamEncoder.reset(4502, false);
  logTestResult(bitStreamEncoder.getChannelId() == 2);
}

void testGetChannelId3(LegoPfBitStreamEncoder& bitStreamEncoder) {
  Serial.print("  testGetChannelId3               ");
  bitStreamEncoder.reset(8597, false);
  logTestResult(bitStreamEncoder.getChannelId() == 3);
}

void testGetChannelId4(LegoPfBitStreamEncoder& bitStreamEncoder) {
  Serial.print("  testGetChannelId4               ");
  bitStreamEncoder.reset(12692, false);
  logTestResult(bitStreamEncoder.getChannelId() == 4);
}

void testGetMessageLengthAllHigh(LegoPfBitStreamEncoder& bitStreamEncoder) {
  Serial.print("  testGetMessageLengthAllHigh     ");
  bitStreamEncoder.reset(0xFFFF, false);
  logTestResult(bitStreamEncoder.getMessageLength() == 13744);
}

void testGetMessageLengthAllLow(LegoPfBitStreamEncoder& bitStreamEncoder) {
  Serial.print("  testGetMessageLengthAllLow      ");
  bitStreamEncoder.reset(0x0, false);
  logTestResult(bitStreamEncoder.getMessageLength() == 9104);
}




