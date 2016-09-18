#include "IRremote.h"
#include "IRremoteInt.h"

//             #                  # 
//             #################### 
//             #################### 
//             #################### 
//             #               ##
//                              ## 
//             #                ###
//             #################### 
//             ####################
//             ##################
//                              ##
//             #                ###
//             #################### 
//             ####################
//             ################### 
//             # 
//                    ###### 
//                 ############ 
//               ################ 
//              #####        ##### 
//             ###               ##
//             #                  # 
//             #                  # 
//             ##                ## 
//              ##              ###
//              #####        ##### 
//                ###############
//                  ########## 
//                           
//                                # 
//                                # 
//               ####################### 
//              ######################## 
//             ######################### 
//             ###                # 
//             ###
//               # 
//                    ###### 
//                 ############ 
//               ################ 
//              #####        ##### 
//             ###               ##
//             #                  # 
//             #                  # 
//             ##                ## 
//              ##              ###
//              #####        ##### 
//                ###############
//                  ########## 
// 

#define MOTO_BITS 16
#define MOTO_HDR_MARK 9036
#define MOTO_HDR_SPACE 4424
#define MOTO_BIT_MARK 556
#define MOTO_ONE_SPACE 2185
#define MOTO_ZERO_SPACE 4424
#define MOTO_TRAIL 556
#define MOTO_GAP 29000
#define MOTO_TOP_BIT 0x8000

#if SEND_MOTO
void IRsend::sendMoto(unsigned long data,int nbits)
{
	int ones = 0;
	enableIROut(38);
	mark(MOTO_HDR_MARK);
	space(MOTO_HDR_SPACE);
	for (int i=0; i < nbits; i++) {
		mark(MOTO_BIT_MARK);
		if (data & MOTO_TOP_BIT) {
			space(MOTO_ONE_SPACE);
			ones++;
		} else {
			space(MOTO_ZERO_SPACE);
		}
		data <<= 1;
	}
	mark(MOTO_TRAIL);
	if (ones&1) {
		space(30850);
	}
	else {
		space(28600);
	}
	mark(MOTO_HDR_MARK);
	space(MOTO_ONE_SPACE);
	mark(MOTO_TRAIL);
	// space(88000);
	space(30000);
	space(30000);
	space(28000);
	mark(MOTO_HDR_MARK);
	space(MOTO_ONE_SPACE);
	mark(MOTO_TRAIL);
	space(0);
}
#endif


#if DECODE_MOTO
bool  IRrecv::decodeMoto(decode_results *results) {
	
	long data = 0;
	int offset = 1;
	if (!MATCH_MARK(results->rawbuf[offset],MOTO_HDR_MARK)) {
		return false;
	}
	offset++;
	if (irparams.rawlen < 2 * MOTO_BITS+4) {
		return false;
	}
	if (!MATCH_SPACE(results->rawbuf[offset],MOTO_HDR_SPACE)) {
		return false;
	}
	offset++;
	for (int i = 0; i < MOTO_BITS; i++) {
		if (!MATCH_MARK(results->rawbuf[offset],MOTO_BIT_MARK)) {
			return false;
		}
		offset++;
		if (MATCH_SPACE(results->rawbuf[offset],MOTO_ONE_SPACE)) {
			data = (data << 1) | 1;
		}
		else if (MATCH_SPACE(results->rawbuf[offset],MOTO_ZERO_SPACE)) {
			data <<= 1;
		}
		else {
			return false;
		}
		offset++;
	}
	results->bits = MOTO_BITS;
	results->value = data;
	results->decode_type = MOTOROLA;
	return true;
}
#endif
