//  
// Copyright (C) 2023 SIP Spectrum, Inc. http://www.sipspectrum.com 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#include "os/OsDefs.h"
#include "mp/MpAudioFileDecompress.h"

/* The number of bits required by each value */
static unsigned char numBits[] = {
   0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
   6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
};

/* Mu-Law conversions */
static bool muLawDecodeTableInitialized = false;
static AudioSample muLawDecodeTable[256];

// Constructor initializes the decoding table
DecompressG711MuLaw::DecompressG711MuLaw(MpAudioAbstract &a)
      : AbstractDecompressor(a)
{
      osPrintf("Decoding: ITU G.711 mu-Law\n");
   if (!muLawDecodeTableInitialized) {
      muLawDecodeTableInitialized = true;
      for(int i=0;i<256;i++)
         muLawDecodeTable[i] = MuLawDecode(i);
   }
}

size_t DecompressG711MuLaw::getSamples(AudioSample *buffer,
                                       size_t length)
{
   AudioByte *byteBuff =
      reinterpret_cast<AudioByte *>(buffer);
   size_t read = readBytes(byteBuff,length);
   for(long i=read-1; i>=0; i--)
   {
      unsigned short pos = (unsigned char)byteBuff[i];
      buffer[i] = muLawDecodeTable[ pos ];
   }
   return read;
}

AudioByte MuLawEncode(AudioSample s)
{
   unsigned char sign = (s<0)?0:0x80; // Save the sign
   if (s<0) s=-s; // make sample positive
   signed long adjusted = static_cast<long>(s) << (16-sizeof(AudioSample)*8);
   adjusted += 128L+4L;
   if (adjusted > 32767) adjusted = 32767;
   unsigned char exponent = numBits[(adjusted>>7)&0xFF] - 1;
   unsigned char mantissa = (unsigned char)((adjusted >> (exponent + 3)) & 0xF);
   return ~(sign | (exponent << 4) | mantissa);
}

AudioSample MuLawDecode(AudioByte ulaw)
{
   ulaw = ~ulaw;
   unsigned char exponent = (ulaw >> 4) & 0x7;
   unsigned char mantissa = (ulaw & 0xF) + 16;
   unsigned long adjusted = (mantissa << (exponent + 3)) - 128 - 4;
   AudioSample  sRet = (AudioSample) adjusted;
   return (ulaw & 0x80)? sRet : -sRet;
}

static bool aLawDecodeTableInitialized = false;
static AudioSample aLawDecodeTable[256];

DecompressG711ALaw::DecompressG711ALaw(MpAudioAbstract &a)
      : AbstractDecompressor(a)
{
      osPrintf("Decoding: ITU G.711 A-Law\n");
   if (!aLawDecodeTableInitialized) {
      aLawDecodeTableInitialized = true;
      for(int i=0;i<256;i++)
         aLawDecodeTable[i] = ALawDecode(i);
   }
}

size_t DecompressG711ALaw::getSamples(AudioSample *buffer, size_t length)
{
   AudioByte *byteBuff =
      reinterpret_cast<AudioByte *>(buffer);
   size_t read = readBytes(byteBuff,length);
   for (long i = read - 1; i >= 0; i--)
   {
       unsigned short pos = (unsigned char)byteBuff[i];
       buffer[i] = aLawDecodeTable[pos];
   }
   return read;
}

AudioByte ALawEncode(AudioSample s)
{
   unsigned char sign = (s<0)?0:0x80; // Save the sign
   if (s<0) s=-s; // make sample positive
   signed long adjusted = static_cast<long>(s)+8L; // Round it
   if (adjusted > 32767) adjusted = 32767; // Clip it
   unsigned char exponent = numBits[(adjusted>>8)&0x7F];
   unsigned char mantissa = (unsigned char)((adjusted >> (exponent + 4)) & 0xF);
   return sign | (((exponent << 4) | mantissa) ^ 0x55);
}

AudioSample ALawDecode(AudioByte alaw)
{
    int i;
    int seg;

    alaw ^= 0x55;
    i = ((alaw & 0x0F) << 4);
    seg = (((int)alaw & 0x70) >> 4);
    if (seg != 0)
        i = (i + 0x108) << (seg - 1);
    else
        i += 8;
    return (AudioSample)(((alaw & 0x80) != 0) ? i : -i);

   //Note: This alaw implementation plays back with a lot of static, it was replaced with above algorithm on 01/25/2023
   //alaw ^= 0x55;
   //unsigned char exponent = (alaw >> 4) & 0x7;
   //unsigned char mantissa = (alaw & 0xF) + (exponent?16:0);
   //unsigned long adjusted = (mantissa << (exponent + 4));
   //AudioSample  sRet = (AudioSample) adjusted;
   //return (alaw & 0x80)? -sRet : sRet;
}
