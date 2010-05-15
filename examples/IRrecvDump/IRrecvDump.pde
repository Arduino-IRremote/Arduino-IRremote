/*
 * IRremote: IRrecvDump - dump details of IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>

int RECV_PIN = 11;

IRrecv irrecv(RECV_PIN);

decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}

// Dumps out the decode_results structure.
// Call this after IRrecv::decode()
// void * to work around compiler issue
//void dump(void *v) {
//  decode_results *results = (decode_results *)v
void dump(decode_results *results) {
  int count = results->rawlen;
  if (results->decode_type == UNKNOWN) {
    Serial.print("Unknown encoding: ");
  } 
  else if (results->decode_type == NEC) {
    Serial.print("Decoded NEC: ");
  } 
  else if (results->decode_type == SONY) {
    Serial.print("Decoded SONY: ");
  } 
  else if (results->decode_type == RC5) {
    Serial.print("Decoded RC5: ");
  } 
  else if (results->decode_type == RC6) {
    Serial.print("Decoded RC6: ");
  }
  else if (results->decode_type == SPACE_ENC) {
    Serial.print("Decoded SPACE_ENC: ");
  }
  Serial.print(results->value, HEX);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): ");
  printSonyData(results);

  for (int i = 0; i < count; i++) {
    if ((i % 2) == 1) {
      Serial.print(results->rawbuf[i]*USECPERTICK, DEC);
    } 
    else {
      Serial.print(-(int)results->rawbuf[i]*USECPERTICK, DEC);
    }
    Serial.print(" ");
  }
  Serial.println("");
}

// Helper function for displaying Sony information.
// Takes the bottom nbits bits from value, reverses them,
// and returns the result.
unsigned long reverseBits(unsigned long value, int nbits) {
  unsigned long result = 0;
  // The idea is that mask starts at the leftmost position and moves right.
  // We look at the rightmost bit of value and move left.
  // If the bit in value is set, then set the opposite bit in result using mask.
  unsigned long mask = 1 << (nbits-1);
  for (int i = 0; i < nbits; i++) {
    if (value & 1) {
      result |= mask;
    }
    value >>= 1;
    mask >>= 1;  
  }
  return result;
}

// If this looks like a Sony code, extract and print the various fields
// after reversing the bits.
// See http://www.hifi-remote.com/sony/ for more information on these codes.

void printSonyData(decode_results *results) {
  if (results->bits == 12) {
    // 7 command bits, 5 device bits
    Serial.print("device: ");
    Serial.print(reverseBits(results->value, 5), DEC);
    Serial.print(", command: ");
    Serial.println(reverseBits(results->value >> 5, 7), DEC);
  } else if (results->bits == 15) {
    // 7 command bits, 8 device bits
    Serial.print("device: ");
    Serial.print(reverseBits(results->value, 8), DEC);
    Serial.print(", command: ");
    Serial.println(reverseBits(results->value >> 8, 7), DEC);
  } else if (results->bits == 20) {
    // 7 command bits, 5 device bits, 8 extended device bits
    Serial.print("device: ");
    Serial.print(reverseBits(results->value >> 8, 5), DEC);
    Serial.print(".");
    Serial.print(reverseBits(results->value, 8), DEC);  
    Serial.print(", command: ");
    Serial.println(reverseBits(results->value >> 13, 7), DEC);
  }
}

void loop() {
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    dump(&results);
    irrecv.resume(); // Receive the next value
  }
}
