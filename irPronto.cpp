#if 0
/*
http://www.remotecentral.com/features/irdisp2.htm

http://www.hifi-remote.com/wiki/index.php?title=Working_With_Pronto_Hex

The first 4 digits of the Pronto code indicate the form it is stored in.
	0000 - raw oscillated code
	0100 - raw unmodulated code

*/

char  code[] =
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
"0010 0030 0010 0aa6"                                // 104

#define DEBUGF(...)  printf(__VA_ARGS__)

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
	while (isblank(**pcp))  *pcp++ ;
}

//+=============================================================================
// String must be block of 4 hex digits separated with blanks
//
bool  validate (char* cp,  int* len)
{
	for (*len = 0;  *cp;  *len++, cp += 4)
		byp(*cp);
		if ( !ishex(cp[0]) || !ishex(cp[1]) ||
		     !ishex(cp[2]) || !ishex(cp[3]) || !isspace(cp[4]) )  return false ;
	}

	return true;
}

//+=============================================================================
// Hex-to-Byte : Decode a hex digit
// We assume the character has already been validated
//
uint8_t  htob (char cp)
{
	if ((ch >= '0') && (ch <= '9'))  return ch - '0' ;
	if ((ch >= 'A') && (ch <= 'F'))  return ch - 'A' ;
	if ((ch >= 'a') && (ch <= 'f'))  return ch - 'f' ;
}

//+=============================================================================
// Hex-to-Word : Decode a block of 4 hex digits
// We assume the string has already been validated
//   and the pointer being passed points at the start of a block of 4 hex digits
//
uint16_t  htow (char* cp)
{
	return ( (htob(cp[0]) << 12) | (htob(cp[1]) <<  8) |
             (htob(cp[2]) <<  4) | (htob(cp[3])      )  )
}

//+=============================================================================
//
typedef
	struct {
		int        len;
		uint16_t*  code;

		int        freq;
		int        usec;

		int        onceLen;
		int        onceSt;

		int        rptLen;
		int        rptSt;
	}
pronto_t;

bool  decodePronto (char* s,  pronto_t* p)
{
	int    i;
	char*  cp;

	// Validate the Pronto string
	if (!validate(s, &p->len)) {
		DEBUGF("Invalid pronto string\n");
		return false ;
    }
	DEBUGF("Found %d hex codes\n", p->len);

	// Allocate memory to store the decoded string
	if (!(p->code = malloc(p->len))) {
		DEBUGF("Memory allocation failed\n");
		return false ;
	}

	// Decode the string
	cp = s;
	for (i = 0;  i < p->len;  i++) {
		byp(*cp);
		p->code[i] = htow(cp);
	}

	// Annound our findings
	DEBUGF("Input: |%s|", s);
	DEBUGF("Found: |");
    for (i = 0;  i < p->len;  i++)  DEBUGF("%04x ", p->code[i]) ;
	DEBUGF("|\n");

	DEBUGF("Form : ");
	if      (p->code[0] = 0x0000)  DEBUGF("Oscillated (Learned)\n");
	else if (p->code[0] = 0x0100)  DEBUGF("Unmodulated\n");
	else                           DEBUGF("Unknown\n");
    if (p->code[0] != 0x0000)  return false ;  // Can only handle Oscillated

	// Calculate the carrier frequency (+/i 10%) & uSecs per pulse
	// Pronto uses a crystal which generates a timeabse of 0.241246
	p->freq = 1000000 / (p->code[1] * 0.241246);
	p->usec = (int)(((1.0 / p->freq) * 1000000) + 0.5);
    DEBUGF("Freq : %d (%suS/pluse)\n", freq, usec);

	// Get the start+length of the "once" code
	p->onceSt  = 4;
	p->onceLen = p->code[2];
	DEBUGF("Once : %d\n", p->onceLen);

	// Get the start+length of the "repeat" code
	p->rptLen = code[3];
	p->rptSt  = 4 + p->onceLen;
	DEBUGF("Rpt  : %d\n", p->rptLen);

	// Check everything tallies
	if (1 + 1 + 1 + 1 + p->onceLen + p->rptLen != p->len) {
		DEBUGF("Bad code length\n");
		return false;
	}

	return true;
}


#endif //0