/*
 * Lego Power Functions receive.
 * As per document
 * http://cache.lego.com/Media/Download/PowerfunctionsElementSpecsDownloads/otherfiles/download9FC026117C091015E81EC28101DACD4E/8884RemoteControlIRReceiver_Download.pdf

 * Receives the 16 bit protocol. It can be decoded with the  Open Powerfunctions code
 * https://bitbucket.org/tinkerer_/openpowerfunctionsrx

*/

#include "IRremote.h"
#include "IRremoteInt.h"

#define PF_STARTSTOP 1579
#define PF_LOWBIT 526
#define PF_HIBIT 947
#define PF_LOWER 315
#define BITS          16  // The number of bits in the command

//+=============================================================================
//
#if SEND_LEGO_POWERFUNCTIONS
void  IRsend::sendPowerFunctions (unsigned long data,  int nbits)
{
  // // Set IR carrier frequency
  // enableIROut(38);
  //
  // // Header
  // mark (HDR_MARK);
  // space(HDR_SPACE);
  //
  // // Data
  // for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1)
  // {
  //   if (data & mask)
  //   {
  //     mark (BIT_MARK);
  //     space(ONE_SPACE);
  //   }
  //   else
  //   {
  //     mark (BIT_MARK);
  //     space(ZERO_SPACE);
  //   }
  // }
  //
  // // Footer
  // mark(BIT_MARK);
  // space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
//



#if DECODE_LEGO_POWERFUNCTIONS
bool IRrecv::decodePowerFunc(decode_results *results)
{
  unsigned long  data   = 0;  // Somewhere to build our code

  DBG_PRINTLN(results->rawlen,DEC);
  // Check we have the right amount of data
  if (irparams.rawlen != (2 * BITS) + 4)
    return false ;

  DBG_PRINTLN("Attempting Lego Power Functions Decode");

  uint16_t desired_us = (results->rawbuf[1] + results->rawbuf[2]) * USECPERTICK;
  DBG_PRINT("PF desired_us = ");
  DBG_PRINTLN(desired_us,DEC);

  if(desired_us > PF_HIBIT && desired_us <= PF_STARTSTOP){
    DBG_PRINTLN("Found PF Start Bit");
    int offset = 3;
    for(int i = 0; i < BITS; i++){
      desired_us = (results->rawbuf[offset] + results->rawbuf[offset+1]) * USECPERTICK;

      DBG_PRINT("PF desired_us = ");
      DBG_PRINTLN(desired_us,DEC);
      if(desired_us >= PF_LOWER && desired_us <= PF_LOWBIT){
        DBG_PRINTLN("PF 0");
        data = (data << 1) | 0 ;
      }else if(desired_us > PF_LOWBIT && desired_us <= PF_HIBIT){
        DBG_PRINTLN("PF 1");
        data = (data << 1) | 1 ;
      }else{
        DBG_PRINTLN("PF Failed");
        return false;
      }
      offset += 2;
    }

    desired_us = (results->rawbuf[offset]) * USECPERTICK;

    DBG_PRINT("PF END desired_us = ");
    DBG_PRINTLN(desired_us,DEC);
    if(desired_us < PF_LOWER){
      DBG_PRINTLN("Found PF End Bit");
      DBG_PRINTLN(data, BIN);

      // Success
      results->bits        = BITS;
      results->value       = data;
      results->decode_type = POWERFUNCTIONS;
      return true;
    }

  }
  return false;
}
#endif
