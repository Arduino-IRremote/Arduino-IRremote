#define TEST 0

#if TEST
#	define SEND_PRONTO        1
#	define PRONTO_ONCE        false
#	define PRONTO_REPEAT      true
#	define PRONTO_FALLBACK    true
#	define PRONTO_NOFALLBACK  false
#endif

#if SEND_PRONTO

//******************************************************************************
#if TEST
#	include <stdio.h>
	void  enableIROut (int freq)  { printf("\nFreq = %d KHz\n", freq); }
	void  mark        (int t)     { printf("+%d," , t); }
	void  space       (int t)     { printf("-%d, ", t); }
#else
#	include "IRremote.h"
#endif // TEST

//+=============================================================================
// Check for a valid hex digit
//
bool  ishex (char ch)
{
	return ( ((ch >= '0') && (ch <= '9')) ||
             ((ch >= 'A') && (ch <= 'F')) ||
             ((ch >= 'a') && (ch <= 'f'))   ) ? true : false ;
}

//+=============================================================================
// Check for a valid "blank" ... '\0' is a valid "blank"
//
bool  isblank (char ch)
{
	return ((ch == ' ') || (ch == '\t') || (ch == '\0')) ? true : false ;
}

//+=============================================================================
// Bypass spaces
//
bool  byp (char** pcp)
{
	while (isblank(**pcp))  (*pcp)++ ;
}

//+=============================================================================
// Hex-to-Byte : Decode a hex digit
// We assume the character has already been validated
//
uint8_t  htob (char ch)
{
	if ((ch >= '0') && (ch <= '9'))  return ch - '0' ;
	if ((ch >= 'A') && (ch <= 'F'))  return ch - 'A' + 10 ;
	if ((ch >= 'a') && (ch <= 'f'))  return ch - 'a' + 10 ;
}

//+=============================================================================
// Hex-to-Word : Decode a block of 4 hex digits
// We assume the string has already been validated
//   and the pointer being passed points at the start of a block of 4 hex digits
//
uint16_t  htow (char* cp)
{
	return ( (htob(cp[0]) << 12) | (htob(cp[1]) <<  8) |
             (htob(cp[2]) <<  4) | (htob(cp[3])      )  ) ;
}

//+=============================================================================
//
bool sendPronto (char* s,  bool repeat,  bool fallback)
{
	int       i;
	int       len;
	int       skip;
	char*     cp;
	uint16_t  freq;  // Frequency in KHz
	uint8_t   usec;  // pronto uSec/tick
	uint8_t   once;
	uint8_t   rpt;

	// Validate the string
	for (cp = s;  *cp;  cp += 4) {
		byp(&cp);
		if ( !ishex(cp[0]) || !ishex(cp[1]) ||
		     !ishex(cp[2]) || !ishex(cp[3]) || !isblank(cp[4]) )  return false ;
	}

	// We will use cp to traverse the string
	cp = s;

	// Check mode = Oscillated/Learned
	byp(&cp);
	if (htow(cp) != 0000)  return false;
	cp += 4;

	// Extract & set frequency
	byp(&cp);
	freq = (int)(1000000 / (htow(cp) * 0.241246));  // Rounding errors will occur, tolerance is +/- 10%
	usec = (int)(((1.0 / freq) * 1000000) + 0.5);  // Another rounding error, thank Cod for analogue electronics
	freq /= 1000;  // This will introduce a(nother) rounding error which we do not want in the usec calcualtion
	cp += 4;

	// Get length of "once" code
	byp(&cp);
	once = htow(cp);
	cp += 4;

	// Get length of "repeat" code
	byp(&cp);
	rpt = htow(cp);
	cp += 4;

	// Which code are we sending?
	if (fallback) { // fallback on the "other" code if "this" code is not present
		if (!repeat) { // requested 'once'
			if (once)  len = once * 2,  skip = 0 ;  // if once exists send it
			else       len = rpt  * 2,  skip = 0 ;  // else send repeat code
		} else { // requested 'repeat'
			if (rpt)   len = rpt  * 2,  skip = 0 ;  // if rpt exists send it
			else       len = once * 2,  skip = 0 ;  // else send once code
		}
	} else {  // Send what we asked for, do not fallback if the code is empty!
		if (!repeat)  len = once * 2,  skip = 0 ;     // 'once' starts at 0
		else          len = rpt  * 2,  skip = once ;  // 'repeat' starts where 'once' ends
    }

	// Skip to start of code
	for (i = 0;  i < skip;  i++, cp += 4)  byp(&cp) ;

	// Send code
	enableIROut(freq);
	for (i = 0;  i < len;  i++) {
		byp(&cp);
		if (i & 1)  space(htow(cp) * usec);
		else        mark (htow(cp) * usec);
		cp += 4;
	}
}

//+=============================================================================
#if TEST

int  main ( )
{
	char  prontoTest[] =
		"0000 0070 0000 0032 0080 0040 0010 0010 0010 0030 " //  10
		"0010 0010 0010 0010 0010 0010 0010 0010 0010 0010 " //  20
		"0010 0010 0010 0010 0010 0010 0010 0010 0010 0010 " //  30
		"0010 0010 0010 0030 0010 0010 0010 0010 0010 0010 " //  40
		"0010 0010 0010 0010 0010 0010 0010 0010 0010 0010 " //  50
		"0010 0010 0010 0030 0010 0010 0010 0010 0010 0010 " //  60
		"0010 0010 0010 0010 0010 0010 0010 0010 0010 0010 " //  70
		"0010 0010 0010 0030 0010 0010 0010 0030 0010 0010 " //  80
		"0010 0010 0010 0030 0010 0010 0010 0010 0010 0030 " //  90
		"0010 0010 0010 0030 0010 0010 0010 0010 0010 0030 " // 100
		"0010 0030 0010 0aa6";                               // 104

	sendPronto(prontoTest, PRONTO_ONCE,   PRONTO_FALLBACK);    // once code
	sendPronto(prontoTest, PRONTO_REPEAT, PRONTO_FALLBACK);    // repeat code
	sendPronto(prontoTest, PRONTO_ONCE,   PRONTO_NOFALLBACK);  // once code
	sendPronto(prontoTest, PRONTO_REPEAT, PRONTO_NOFALLBACK);  // repeat code

	return 0;
}

#endif // TEST

#endif // SEND_PRONTO


























































#if 0
//******************************************************************************
// Sources:
//   http://www.remotecentral.com/features/irdisp2.htm
//   http://www.hifi-remote.com/wiki/index.php?title=Working_With_Pronto_Hex
//******************************************************************************

#include <stdint.h>
#include <stdio.h>

#define IRPRONTO
#include "IRremoteInt.h"  // The Arduino IRremote library defines USECPERTICK

//------------------------------------------------------------------------------
// Source: https://www.google.co.uk/search?q=DENON+MASTER+IR+Hex+Command+Sheet
//         -> http://assets.denon.com/documentmaster/us/denon%20master%20ir%20hex.xls
//
char  prontoTest[] =
	"0000 0070 0000 0032 0080 0040 0010 0010 0010 0030 " //  10
	"0010 0010 0010 0010 0010 0010 0010 0010 0010 0010 " //  20
	"0010 0010 0010 0010 0010 0010 0010 0010 0010 0010 " //  30
	"0010 0010 0010 0030 0010 0010 0010 0010 0010 0010 " //  40
	"0010 0010 0010 0010 0010 0010 0010 0010 0010 0010 " //  50
	"0010 0010 0010 0030 0010 0010 0010 0010 0010 0010 " //  60
	"0010 0010 0010 0010 0010 0010 0010 0010 0010 0010 " //  70
	"0010 0010 0010 0030 0010 0010 0010 0030 0010 0010 " //  80
	"0010 0010 0010 0030 0010 0010 0010 0010 0010 0030 " //  90
	"0010 0010 0010 0030 0010 0010 0010 0010 0010 0030 " // 100
	"0010 0030 0010 0aa6";                               // 104

//------------------------------------------------------------------------------
// This is the longest code we can support
#define CODEMAX  200

//------------------------------------------------------------------------------
// This is the data we pull out of the pronto code
typedef
	struct {
		int        freq;           // Carrier frequency (in Hz)
		int        usec;           // uSec per tick (based on freq)

		int        codeLen;        // Length of code
		uint16_t   code[CODEMAX];  // Code in hex

		int        onceLen;        // Length of "once" transmit
		uint16_t*  once;           // Pointer to start within 'code'

		int        rptLen;         // Length of "repeat" transmit
		uint16_t*  rpt;            // Pointer to start within 'code'
	}
pronto_t;

//------------------------------------------------------------------------------
// From what I have seen, the only time we go over 8-bits is the 'space'
// on the end which creates the lead-out/inter-code gap.  Assuming I'm right,
// we can code this up as a special case and otherwise halve the size of our
// data!
// Ignoring the first four values (the config data) and the last value
// (the lead-out), if you find a protocol that uses values greater than 00fe
// we are going to have to revisit this code!
//
//
// So, the 0th byte will be the carrier frequency in Khz (NOT Hz)
//      "  1st  "    "   "   "  length of the "once" code
//      "  2nd  "    "   "   "  length of the "repeat" code
//
// Thereafter, odd  bytes will be Mark  lengths as a multiple of USECPERTICK uS
//             even   "     "  "  Space    "    "  "    "     "       "      "
//
// Any occurence of "FF" in either a Mark or a Space will indicate
//   "Use the 16-bit FF value" which will also be a multiple of USECPERTICK uS
//
//
// As a point of comparison, the test code (prontoTest[]) is 520 bytes
// (yes, more than 0.5KB of our Arduino's precious 32KB) ... after conversion
// to pronto hex that goes down to ((520/5)*2) = 208 bytes ... once converted to
// our format we are down to ((208/2) -1 -1 +2) = 104 bytes
//
// In fariness this is still very memory-hungry
// ...As a rough guide:
//   10 codes cost 1K of memory (this will vary depending on the protocol).
//
// So if you're building a complex remote control, you will probably need to
// keep the codes on an external memory device (not in the Arduino sketch) and
// load them as you need them.  Hmmm.
//
// This dictates that "Oscillated Pronto Codes" are probably NOT the way forward
//
// For example, prontoTest[] happens to be: A 48-bit IR code in Denon format
// So we know it starts with 80/40                           (Denon header)
//             and ends with 10/aa6                          (Denon leadout)
//             and all (48) bits in between are either 10/10 (Denon 0)
//                                                  or 10/30 (Denon 1)
// So we could easily store this data in 1-byte  ("Denon")
//                                     + 1-byte  (Length=48)
//                                     + 6-bytes (IR code)
// At 8-bytes per code, we can store 128 codes in 1KB or memory - that's a lot
// better than the 2 (two) we started off with!
//
// And serendipitously, by reducing the amount of data, our program will run
// a LOT faster!
//
// Again, I repeat, even after you have spent time converting the "Oscillated
// Pronto Codes" in to IRremote format, it will be a LOT more memory-hungry
// than using sendDenon() (or whichever) ...BUT these codes are easily
// available on the internet, so we'll support them!
//
typedef
	struct {
		uint16_t   FF;
		uint8_t    code[CODEMAX];
	}
irCode_t;

//------------------------------------------------------------------------------
#define DEBUGF(...)  printf(__VA_ARGS__)

//+=============================================================================
// String must be block of 4 hex digits separated with blanks
//
bool  validate (char* cp,  int* len)
{
	for (*len = 0;  *cp;  (*len)++, cp += 4) {
		byp(&cp);
		if ( !ishex(cp[0]) || !ishex(cp[1]) ||
		     !ishex(cp[2]) || !ishex(cp[3]) || !isblank(cp[4]) )  return false ;
	}

	return true;
}

//+=============================================================================
// Hex-to-Byte : Decode a hex digit
// We assume the character has already been validated
//
uint8_t  htob (char ch)
{
	if ((ch >= '0') && (ch <= '9'))  return ch - '0' ;
	if ((ch >= 'A') && (ch <= 'F'))  return ch - 'A' + 10 ;
	if ((ch >= 'a') && (ch <= 'f'))  return ch - 'a' + 10 ;
}

//+=============================================================================
// Hex-to-Word : Decode a block of 4 hex digits
// We assume the string has already been validated
//   and the pointer being passed points at the start of a block of 4 hex digits
//
uint16_t  htow (char* cp)
{
	return ( (htob(cp[0]) << 12) | (htob(cp[1]) <<  8) |
             (htob(cp[2]) <<  4) | (htob(cp[3])      )  ) ;
}

//+=============================================================================
// Convert the pronto string in to data
//
bool  decode (char* s,  pronto_t* p,  irCode_t* ir)
{
	int    i, len;
	char*  cp;

	// Validate the Pronto string
	if (!validate(s, &p->codeLen)) {
		DEBUGF("Invalid pronto string\n");
		return false ;
    }
	DEBUGF("Found %d hex codes\n", p->codeLen);

	// Allocate memory to store the decoded string
	//if (!(p->code = malloc(p->len))) {
	//	DEBUGF("Memory allocation failed\n");
	//	return false ;
	//}

	// Check in case our code is too long
	if (p->codeLen > CODEMAX) {
		DEBUGF("Code too long, edit CODEMAX and recompile\n");
		return false ;
	}

	// Decode the string
	cp = s;
	for (i = 0;  i < p->codeLen;  i++, cp += 4) {
		byp(&cp);
		p->code[i] = htow(cp);
	}

	// Announce our findings
	DEBUGF("Input: |%s|\n", s);
	DEBUGF("Found: |");
    for (i = 0;  i < p->codeLen;  i++)  DEBUGF("%04x ", p->code[i]) ;
	DEBUGF("|\n");

	DEBUGF("Form [%04X] : ", p->code[0]);
	if      (p->code[0] == 0x0000)  DEBUGF("Oscillated (Learned)\n");
	else if (p->code[0] == 0x0100)  DEBUGF("Unmodulated\n");
	else                            DEBUGF("Unknown\n");
    if (p->code[0] != 0x0000)  return false ;  // Can only handle Oscillated

	// Calculate the carrier frequency (+/- 10%) & uSecs per pulse
	// Pronto uses a crystal which generates a timeabse of 0.241246
	p->freq     = (int)(1000000 / (p->code[1] * 0.241246));
	p->usec     = (int)(((1.0 / p->freq) * 1000000) + 0.5);
	ir->code[0] = p->freq / 1000;
    DEBUGF("Freq [%04X] : %d Hz  (%d uS/pluse) -> %d KHz\n",
	       p->code[1], p->freq, p->usec, ir->code[0]);

	// Set the length & start pointer for the "once" code
	p->onceLen  = p->code[2];
	p->once     = &p->code[4];
	ir->code[1] = p->onceLen;
	DEBUGF("Once [%04X] : %d\n", p->code[2], p->onceLen);

	// Set the length & start pointer for the "repeat" code
	p->rptLen = p->code[3];
	p->rpt    = &p->code[4 + p->onceLen];
	ir->code[2] = p->rptLen;
	DEBUGF("Rpt  [%04X] : %d\n", p->code[3], p->rptLen);

	// Check everything tallies
	if (1 + 1 + 1 + 1 + (p->onceLen * 2) + (p->rptLen * 2) != p->codeLen) {
		DEBUGF("Bad code length\n");
		return false;
	}

	// Convert the IR data to our new format
	ir->FF = p->code[p->codeLen - 1];

	len = (p->onceLen * 2) + (p->rptLen * 2);
	DEBUGF("Encoded: |");
	for (i = 0;  i < len;  i++) {
		if (p->code[i+4] == ir->FF) {
			ir->code[i+3] = 0xFF;
		} else if (p->code[i+4] > 0xFE) {
			DEBUGF("\n%04X : Mark/Space overflow\n", p->code[i+4]);
			return false;
		} else {
			ir->code[i+3] = (p->code[i+4] * p->usec) / USECPERTICK;
		}
		DEBUGF("%s%d", !i ? "" : (i&1 ? "," : ", "), ir->code[i+3]);
	}
	DEBUGF("|\n");

	ir->FF = (ir->FF * p->usec) / USECPERTICK;
	DEBUGF("FF -> %d\n", ir->FF);

	return true;
}

//+=============================================================================
//
void  irDump (irCode_t* ir)
{
	int  i, len;

	printf("uint8_t  buttonName[%d] = {", len);

	printf("%d,%d, ", (ir->FF >> 8), ir->FF & 0xFF);
	printf("%d,%d,%d, ", ir->code[0], ir->code[1], ir->code[2]);

	len = (ir->code[1] * 2) + (ir->code[2] * 2);
	for (i = 0;  i < len;  i++) {
		printf("%s%d", !i ? "" : (i&1 ? "," : ", "), ir->code[i+3]);
	}

	printf("};\n");

}

//+=============================================================================
//
int  main ( )
{
	pronto_t  pCode;
	irCode_t  irCode;

	decode(prontoTest, &pCode, &irCode);

	irDump(&irCode);

	return 0;
}

#endif //0
