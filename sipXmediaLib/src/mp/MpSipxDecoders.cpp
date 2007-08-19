//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#include "assert.h"

#include "mp/MpSipxDecoders.h"

#define LOCAL static
//#undef LOCAL
//#define LOCAL

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

#define SIGN_BIT        (0x80)          /* Sign bit for a A-law byte. */
#define QUANT_MASK      (0xf)           /* Quantization field mask. */
#define SEG_SHIFT       (4)             /* Left shift for segment number. */
#define SEG_MASK        (0x70)          /* Segment field mask. */

#define BIAS            (0x84)          /* Bias for linear code. */

LOCAL MpAudioSample hzm_ULaw2linear(uint8_t u)
{
   int L;
   int seg;

   u = ~u;
   seg = (u & 0x70) >> 4;
   L = ((0x0f & u) << 3) + BIAS;
   L = (L << seg);
   if (0x80 & u)
   {
      L = BIAS - L;
   }
   else
   {
      L = L - BIAS;
   }
   return L;
}

LOCAL int ULawToLinear(MpAudioSample *Dest, const uint8_t *Source, int samples)
{
   int i;

   for (i=0; i<samples; i++)
   {
      *Dest = hzm_ULaw2linear(*Source);
      Dest++; Source++;
   }
   return samples;
}


/*
 * ALaw2Linear() - Convert an A-law value to 16-bit linear PCM
 *
 */
LOCAL MpAudioSample ALaw2Linear(uint8_t a_val)
{
   int t;
   int seg;

   a_val ^= 0x55;

   t = (a_val & QUANT_MASK) << 4;
   seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
   switch (seg)
   {
   case 0:
      t += 8;
      break;
   case 1:
      t += 0x108;
      break;
   default:
      t += 0x108;
      t <<= seg - 1;
   }
   return ((a_val & SIGN_BIT) ? t : -t);
}

LOCAL int ALawToLinear(MpAudioSample *Dest, const uint8_t *src, int samples)
{
   int i;

   for (i=0; i<samples; i++)
   {
      *Dest = ALaw2Linear(*src);
      Dest++; src++;
   }
   return samples;
}

LOCAL int16_t seg_end[8] = {0x00FF, 0x01FF, 0x03FF, 0x07FF,
                            0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF};

LOCAL int search(MpAudioSample val, int16_t *table, int size)
{
   int             i;

   for (i = 0; i < size; i++)
   {
      if (val <= *table++)
      {
         return i;
      }
   }
   return size;
}

int G711A_Decoder(int numSamples,
                  const uint8_t* codBuff,
                  MpAudioSample* outBuff)
{
   ALawToLinear(outBuff, codBuff, numSamples);
   return 0;
}

int G711U_Decoder(int numSamples,
                  const uint8_t* codBuff,
                  MpAudioSample* outBuff)
{
   ULawToLinear(outBuff, codBuff, numSamples);
   return 0;
}

/*
 * Linear2ALaw() - Convert a 16-bit linear PCM value to 8-bit A-law
 *
 * Linear2ALaw() accepts an 16-bit integer and encodes it as A-law data.
 *
 *              Linear Input Code       Compressed Code
 *      ------------------------        ---------------
 *      0000000wxyza                    000wxyz
 *      0000001wxyza                    001wxyz
 *      000001wxyzab                    010wxyz
 *      00001wxyzabc                    011wxyz
 *      0001wxyzabcd                    100wxyz
 *      001wxyzabcde                    101wxyz
 *      01wxyzabcdef                    110wxyz
 *      1wxyzabcdefg                    111wxyz
 *
 * For further information see John C. Bellamy's Digital Telephony, 1982,
 * John Wiley & Sons, pps 98-111 and 472-476.
 */
LOCAL uint8_t Linear2ALaw(MpAudioSample pcm_val ///< 2's complement (16-bit range)
                          )
{
   int      mask;
   int      seg;
   uint8_t  aval;

   if (pcm_val >= 0)
   {
      mask = 0xD5;            /* sign (7th) bit = 1 */
   }
   else
   {
      mask = 0x55;            /* sign bit = 0 */
      pcm_val = -pcm_val - 8;
   }

   /* Convert the scaled magnitude to segment number. */
   seg = search(pcm_val, seg_end, 8);

   /* Combine the sign, segment, and quantization bits. */

   if (seg >= 8)           /* out of range, return maximum value. */
   {
      return (0x7F ^ mask);
   }
   else
   {
      aval = seg << SEG_SHIFT;
      if (seg < 2)
         aval |= (pcm_val >> 4) & QUANT_MASK;
      else
         aval |= (pcm_val >> (seg + 3)) & QUANT_MASK;
      return (aval ^ mask);
   }
}

LOCAL int LinearToALaw(uint8_t *Dest,const MpAudioSample *src, int samples)
{
   int i;

   for (i=0; i<samples; i++)
   {
      *Dest = Linear2ALaw(*src);
      Dest++; src++;
   }
   return samples;
}

int G711A_Encoder(int numSamples,
                  const MpAudioSample* inBuff,
                  uint8_t* outBuf)
{
   LinearToALaw(outBuf, inBuff, numSamples);
   return 0;
}

LOCAL uint8_t hzm_Linear2ULaw(MpAudioSample L)
{
   int seg;
   uint8_t signmask;

   if (0 > L) {
      L = BIAS - L;
      signmask = 0x7f;
   } else {
      signmask = 0xff;
      L = BIAS + L;
   }
   if (L > 32767) L = 32767;
   if (0x7800 & L) {
      seg = (4<<4);
   } else {
      seg = 0;
      L = L << 4;
   }
   if (0x6000 & L) {
      seg += (2<<4);
   } else {
      L = L << 2;
   }
   if (0x4000 & L) {
      seg += (1<<4);
   } else {
      L = L << 1;
   }
   return ((seg | ((0x3C00 & L) >> 10)) ^ signmask);
}

LOCAL int LinearToULaw(uint8_t *Dest, const MpAudioSample *src, int samples)
{
   int i;

   for (i=0; i<samples; i++) {
      *Dest = hzm_Linear2ULaw(*src);
      Dest++; src++;
   }
   return samples;
}

int G711U_Encoder(int numSamples,
                  const MpAudioSample* inBuff,
                  uint8_t* outBuf)
{
   LinearToULaw(outBuf, inBuff, numSamples);
   return 0;
}
