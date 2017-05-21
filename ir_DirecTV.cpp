/*
	
May 21, 2017 
Created by: Danilo Queiroz Barbosa
member of electronicdrops.com

This is the protocol used on DirecTV and Sky HDTV infrared Controllers.
*/


#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//
//
//                              DIRECTV   SKY
//
//
//==============================================================================


#define BITS          16  // The number of bits in the command

#define HDR_MARK    6000  // The length of the Header:Mark
#define HDR_SPACE   1200  // The lenght of the Header:Space

#define BIT_MARK    580  // The length of a Bit:Mark
#define ONE_SPACE   1200  // The length of a Bit:Space for 1's
#define ZERO_SPACE  580  // The length of a Bit:Space for 0's
#define TOPBIT 0x8000

//+=============================================================================
//
#if SEND_DIRECTV
void  IRsend::sendDirecTV (unsigned short data,  int nbits)
{

        int toggle;
	// Set IR carrier frequency
	enableIROut(36);

	// Header
	mark(HDR_MARK);
	space(HDR_SPACE);

        toggle = 0;


	// Data
	for (int i = 0; i < nbits; i++) {
           toggle = !toggle;
           if (data & TOPBIT) {
   
            if(toggle){
              mark(ONE_SPACE);
            }
            else{
              space(ONE_SPACE);
            }

           }
           else{ 
             
            if(toggle){
              mark(ZERO_SPACE);
            }
            else{
              space(ZERO_SPACE);
            }
             
               }
      data <<= 1;


        }
	// Footer
	mark(BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif



#if DECODE_DIRECTV
bool  IRrecv::decodeDirecTV (decode_results *results)
{
	unsigned long  data   = 0;  // Somewhere to build our code
	int            offset = 1;  // Skip the Gap reading

	// Check we have the right amount of data

	if (irparams.rawlen != 1 + 1 + 2 + BITS)  return false ;

	// Check initial Mark+Space match
	if (!MATCH_MARK (results->rawbuf[offset++], HDR_MARK ))  return false ;
	if (!MATCH_SPACE(results->rawbuf[offset++], HDR_SPACE))  return false ;


        int toggle = 0;

	for(int i = 0; i < BITS; i++){
		toggle = ! toggle;

		if (toggle){
			if (MATCH_MARK(results->rawbuf[offset], ONE_SPACE)){
				data = (data << 1) | 1 ;
                                offset++;
			}
			else if(MATCH_MARK(results->rawbuf[offset], ZERO_SPACE)){
				data = (data << 1) | 0 ;
                                offset++;
			}
			else return false;
		}	
		else{
			if (MATCH_SPACE(results->rawbuf[offset], ONE_SPACE)){
				data = (data << 1) | 1 ;
                                offset++;
			}
			else if(MATCH_SPACE(results->rawbuf[offset], ZERO_SPACE)){
				data = (data << 1) | 0 ;
                                offset++;
			}
			else return false;

		}

}

	// Success
	results->bits        = BITS;
	results->value       = data;
	results->decode_type = DIRECTV;
	return true;
}
#endif










