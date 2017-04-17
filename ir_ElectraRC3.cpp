#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//
//
//                              E L E C T R A   R C 3
//
//
//==============================================================================
// The Electra RC3 AC Remote Control send 3 times 34 bits of same data, encoded using manchester code, separated by a header mark and a header space and a final mark
//
// Ex:
//   X is Mark
//   - is Space
//
// To send 1010, which encoded is equivalent to | -X X- -X X- | , we have the following signal:
//   | XXX --- -X X- -X X- XXX --- -X X- -X X- XXX --- -X X- -X X- XXXX |
//   | header |   data    | header|   data    | header|   data    | end |
//
// This code must be able to send and decode up to 64 bits of data, but the remote control uses only 34 bits.
//
//==============================================================================

#define UNIT         992  // The half of length of each bit
#define HDR_UNIT    2976  // The length of the Header (3 * UNIT)
#define END_UNIT    3968  // The final mark (4 * UNIT)

//This values are for 34 bits of data
#define MIN_RAW_LENGTH  109  // The small amount of raw data to be a valid code
#define MAX_RAW_LENGTH  211  // The maximum amount of raw data to be a valid code, you need to change RAWBUF in IRremoteInt.h to support this value

//+=============================================================================
//
#if SEND_ELECTRARC3
void  IRsend::sendElectraRC3 (unsigned long long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(33);

	// Repeat the signal 3 times
	for (unsigned int k = 0; k < 3; k++)
	{
		// Header
		mark (HDR_UNIT);
		space(HDR_UNIT);

		// Data
		for (unsigned long long  mask = 1ULL << (nbits - 1);  mask;  mask >>= 1) {
			if (data & mask) {
				space(UNIT); // 1 is space, then mark
				mark(UNIT);
			} else {
				mark (UNIT);
				space(UNIT);
			}
		}
	}

	// Footer
	mark(END_UNIT);
  space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
//
#if DECODE_ELECTRARC3
// void printLLNumber(unsigned long long n, uint8_t base)
// {
//   unsigned char buf[16 * sizeof(long)]; // Assumes 8-bit chars. 
//   unsigned long long i = 0;

//   if (n == 0) {
//     Serial.print('0');
//     return;
//   } 

//   while (n > 0) {
//     buf[i++] = n % base;
//     n /= base;
//   }

//   for (; i > 0; i--)
//     Serial.print((char) (buf[i - 1] < 10 ?
//       '0' + buf[i - 1] :
//       'A' + buf[i - 1] - 10));
// }
bool  matchCurrent(int actual, int desired, bool offset) {
	DBG_PRINT("Current - ");
	DBG_PRINT(actual);
	if (offset % 2 == 1)
	{
		DBG_PRINTLN(" - Mark");
		return MATCH_MARK(actual, desired);
	}
	else
	{
		DBG_PRINTLN(" - Space");
		return MATCH_SPACE(actual, desired);
	}
}
bool  matchNext(int actual, int desired, bool offset) {
	DBG_PRINT("Next - ");
	DBG_PRINT(actual);
	if (offset % 2 == 1)
	{
		DBG_PRINTLN(" - Space");
		return MATCH_SPACE(actual, desired);
	}
	else
	{
		DBG_PRINTLN(" - Mark");
		return MATCH_MARK(actual, desired);
	}
}

bool  IRrecv::decodeElectraRC3 (decode_results *results)
{
	int                 seq = -1;
	int                 nbits[3];
	unsigned long long  data[3];

	// Check if we have the right amount of data
	if (results->rawlen > MAX_RAW_LENGTH || results->rawlen < MIN_RAW_LENGTH)  return false ;

	// Check the first header
	DBG_PRINTLN("Check the first header");
	if (!MATCH_MARK(results->rawbuf[1], HDR_UNIT))  return false ;
	if (!MATCH_SPACE(results->rawbuf[2], HDR_UNIT) && !MATCH_SPACE(results->rawbuf[2], HDR_UNIT + UNIT))  return false;

	// Check the end
	DBG_PRINTLN("Check the end");
	if (!MATCH_MARK(results->rawbuf[results->rawlen - 1], END_UNIT))  return false ;

	// OK, It's Electra, let's decode!
	DBG_PRINTLN("Decode");

	for (int offset = 1;  offset < results->rawlen;  offset++) {
		unsigned int current = results->rawbuf[offset];
		unsigned int next = results->rawbuf[offset + 1];

		if (next == 0) break;

		if (
			(matchCurrent(current, HDR_UNIT, offset) || matchCurrent(current, HDR_UNIT + UNIT, offset)) && 
			(matchNext(next, HDR_UNIT, offset) || matchNext(next, HDR_UNIT + UNIT, offset))
		)
		{
			if (seq == 1 && data[0] == data[1] && nbits[0] == nbits[1]) // if the first 2 sequences are equal, we can stop the decode
			{
				DBG_PRINTLN("The first 2 sequences are equal, skiping.");
				break; 
			}

			seq++;
			data[seq] = 0;
			nbits[seq] = 0;
			if (matchNext(next, HDR_UNIT, offset))
			{
				offset++;
			}

			DBG_PRINTLN("HEADER");
			continue;
		}

		if (offset % 2 == 0)
		{
			data[seq] = (data[seq] << 1) | 1;
			DBG_PRINTLN("DATA: 1");
		}
		else
		{
			// data[seq] = (data[seq] << 1) | 0;
			data[seq] <<= 1;
			DBG_PRINTLN("DATA: 0");
		}
		nbits[seq]++;

		if (matchNext(next, UNIT, offset)) {
			offset++;
		}

		// Serial.print("S: ");
		// Serial.print(seq);
		// Serial.print(" - B: ");
		// if (nbits[seq] <= 9)
		// {
		// 	Serial.print("0");
		// }
		// Serial.print(nbits[seq]);
		// Serial.print(" - D: ");
		// printLLNumber(data[seq], BIN);
		// Serial.println("");
	}

	// Success
	if (data[0] == data[1] && nbits[0] == nbits[1])
	{
		results->bits        = nbits[0];
		results->value       = data[0];
	}
	else if (data[0] == data[2] && nbits[0] == nbits[2])
	{
		results->bits        = nbits[0];
		results->value       = data[0];
	}
	else if (data[1] == data[2] && nbits[1] == nbits[2])
	{
		results->bits        = nbits[1];
		results->value       = data[1];
	}
	else
	{
		return false;
	}
	results->decode_type = ELECTRARC3;
	return true;
}
#endif
