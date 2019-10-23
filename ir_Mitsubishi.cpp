#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//    MMMMM  IIIII TTTTT   SSSS  U   U  BBBB   IIIII   SSSS  H   H  IIIII
//    M M M    I     T    S      U   U  B   B    I    S      H   H    I
//    M M M    I     T     SSS   U   U  BBBB     I     SSS   HHHHH    I
//    M   M    I     T        S  U   U  B   B    I        S  H   H    I
//    M   M  IIIII   T    SSSS    UUU   BBBBB  IIIII  SSSS   H   H  IIIII
//==============================================================================

// IR Mitsubishi protocol (https://sbprojects.net/knowledge/ir/xsat.php)

#define MITSUBISHI_BITS 16

#define MITSUBISHI_ONE_MARK	1950 // 41*50-100
#define MITSUBISHI_ZERO_MARK  750 // 17*50-100

#define MITSUBISHI_HDR_MARK   8000U
#define MITSUBISHI_HDR_SPACE  4000U
#define MITSUBISHI_BIT_MARK   526U
#define MITSUBISHI_ZERO_SPACE 474U
#define MITSUBISHI_ONE_SPACE  1474U

//+=============================================================================
#if SEND_MITSUBISHI
void  IRsend::sendMitsubishi (unsigned long data, int nbits)
{
  // Set IR carrier frequency
  enableIROut(38); // kHz

  uint8_t address = data >> (nbits / 2);
  uint8_t command = data & ((1 << (nbits / 2)) - 1);
  unsigned int dataLength = 0;

  // Header
  mark(MITSUBISHI_HDR_MARK);
  space(MITSUBISHI_HDR_SPACE);

  // ------- first part of data: address ------- //
  for (uint64_t mask = 1ULL << 7; mask; mask >>= 1)
    if (address & mask) {
      // Send 1
      mark(MITSUBISHI_BIT_MARK);
      space(MITSUBISHI_ONE_SPACE);
      dataLength +=2;
    } else {
      // Send 0
      mark(MITSUBISHI_BIT_MARK);
      space(MITSUBISHI_ZERO_SPACE);
      dataLength +=1;
    }

  // Gap between the address and the command
  mark(MITSUBISHI_BIT_MARK);
  space(MITSUBISHI_HDR_SPACE);

  // ------- second part of data: command ------- //
  for (uint64_t mask = 1ULL << 7; mask; mask >>= 1)
    if (command & mask) {
      // Send 1
      mark(MITSUBISHI_BIT_MARK);
      space(MITSUBISHI_ONE_SPACE);
      dataLength +=2;
    } else {
      // Send 0
      mark(MITSUBISHI_BIT_MARK);
      space(MITSUBISHI_ZERO_SPACE);
      dataLength +=1;
    }

  // Footer
  mark(MITSUBISHI_BIT_MARK);
  unsigned int gap = 60000 - (dataLength * 1000) - MITSUBISHI_HDR_MARK - MITSUBISHI_HDR_SPACE * 2 - MITSUBISHI_BIT_MARK * 2;
  space(gap);
};
#endif

#if DECODE_MITSUBISHI
bool  IRrecv::decodeMitsubishi (decode_results *results)
{
  unsigned int HDR_SPACE = 350;
  // Serial.print("?!? decoding Mitsubishi:");Serial.print(irparams.rawlen); Serial.print(" want "); Serial.println( 2 * MITSUBISHI_BITS + 2);
  long data = 0;
  if (irparams.rawlen < 2 * MITSUBISHI_BITS + 2)  return false ;
  int offset = 0; // Skip first space
  // Initial space

#if 0
  // Put this back in for debugging - note can't use #DEBUG as if Debug on we don't see the repeat cos of the delay
  Serial.print("IR Gap: ");
  Serial.println( results->rawbuf[offset]);
  Serial.println( "test against:");
  Serial.println(results->rawbuf[offset]);
#endif

#if 0
  // Not seeing double keys from Mitsubishi
  if (results->rawbuf[offset] < MITSUBISHI_DOUBLE_SPACE_USECS) {
    // Serial.print("IR Gap found: ");
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = MITSUBISHI;
    return true;
  }
#endif

  offset++;

  // Typical
  // 14200 7 41 7 42 7 42 7 17 7 17 7 18 7 41 7 18 7 17 7 17 7 18 7 41 8 17 7 17 7 18 7 17 7

  // Initial Space
  if (!MATCH_MARK(results->rawbuf[offset], HDR_SPACE))  return false ;
  offset++;

  while (offset + 1 < irparams.rawlen) {
    if      (MATCH_MARK(results->rawbuf[offset], MITSUBISHI_ONE_MARK))   data = (data << 1) | 1 ;
    else if (MATCH_MARK(results->rawbuf[offset], MITSUBISHI_ZERO_MARK))  data <<= 1 ;
    else                                                                 return false ;
    offset++;

    if (!MATCH_SPACE(results->rawbuf[offset], HDR_SPACE))  break ;
    offset++;
  }

  // Success
  results->bits = (offset - 1) / 2;
  if (results->bits < MITSUBISHI_BITS) {
    results->bits = 0;
    return false;
  }

  results->value       = data;
  results->decode_type = MITSUBISHI;
  return true;
}
#endif

