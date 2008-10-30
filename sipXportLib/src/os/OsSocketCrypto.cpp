//  
// Copyright (C) 2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2008 Mutualink, Inc. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifdef HAVE_SSL
// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsIntTypes.h"
#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "os/OsSocketCrypto.h"
#include "utl/UtlCryptoKeySym.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// DEFINES

#define SDP_CODEC_PCMU          0
//#define SDP_CODEC_PCMU          8
#define SRTP_PAYLOADTYPE_PCMU   1

struct rtpHeader {
   uint8_t vpxcc;       ///< Version, Padding, Extension and CSRC Count bits.
   uint8_t mpt;         ///< Marker and Payload Type bits.
   uint16_t seq;        ///< Sequence Number (Big Endian!)
   uint32_t timestamp;  ///< Timestamp (Big Endian!)
   uint32_t ssrc;       ///< SSRC (Big Endian!)
};

#define RTP_P_MASK    0x20  ///< Mask for Padding bit in RtpHeader::vpxcc

#define RTP_PT_MASK   0x7F  ///< Mask for Payload Type bits in RtpHeader::mpt
#define RTP_M_MASK    0x80  ///< Mask for Marker bit in RtpHeader::mpt

// MACROS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */

OsSocketCryptoProxy::OsSocketCryptoProxy(OsSocket* pureSocket,
                                         int (OsSocket::*read)(char*, int, UtlString*, int*),
                                         int (OsSocket::*write)(const char*, int),
                                         const char* pEncBinData,
                                         int encBinLength,
                                         const char* pDecBinData,
                                         int decBinLength)
: mDoPadding(FALSE)
, mpRtpEncryptKey(NULL)
, mpRtpDecryptKey(NULL)
, mPureSocket(pureSocket)
, mRead(read)
, mWrite(write)
{
   setSymKeys(pEncBinData, encBinLength, pDecBinData, decBinLength);
}

OsSocketCryptoProxy::~OsSocketCryptoProxy()
{
   delete mpRtpEncryptKey;
   delete mpRtpDecryptKey;
}

/* ============================= MANIPULATORS ============================= */


UtlBoolean OsSocketCryptoProxy::setSymKeys(const char* pEncBinData,
                                           int encBinLength,
                                           const char* pDecBinData,
                                           int decBinLength)
{
   if (mpRtpEncryptKey != NULL)
      delete mpRtpEncryptKey;

   mpRtpEncryptKey = new UtlCryptoKeySym();
   if (mpRtpEncryptKey->loadBinaryKey((const uint8_t*)pEncBinData, encBinLength) != 0)
      return FALSE;


   if (mpRtpDecryptKey != NULL)
      delete mpRtpDecryptKey;

   mpRtpDecryptKey = new UtlCryptoKeySym();
   if (mpRtpDecryptKey->loadBinaryKey((const uint8_t*)pDecBinData, decBinLength) != 0)
      return FALSE;

   return TRUE;
}

int OsSocketCryptoProxy::write(const char* buffer, int bufferLength)
{
   // We assume that buffer is RTP packet, if so and payload == pcmu
   // perform encryption before send
   struct rtpHeader* ph = (struct rtpHeader* )buffer;

   unsigned char* payloadData = (unsigned char* )ph + sizeof(struct rtpHeader);
   UtlBoolean padding = ((ph->vpxcc & RTP_P_MASK) == RTP_P_MASK);

   int payloadOctets = bufferLength - sizeof(struct rtpHeader);
   if (padding)
   {
      uint8_t padBytes = buffer[bufferLength - 1];
      if ((padBytes & (~3)) != 0) {
         // For security reason
         padBytes = 0;
      }
      payloadOctets -= padBytes;
   }

   // Encrypt the payload if necessary
   UtlCryptoKey* pKey = mpRtpEncryptKey;
   if ((pKey) && ((ph->mpt & RTP_PT_MASK) == SDP_CODEC_PCMU))
   {
      // Move the clear payload into an allocated array
      memcpy(mCryptoBuffer, buffer, sizeof(struct rtpHeader));
      int encLen = MAX_CRYPTOBUFFER - sizeof(struct rtpHeader);

      if (pKey->encrypt(payloadData, payloadOctets, 
         mCryptoBuffer + sizeof(struct rtpHeader), &encLen) == 0)
      {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
            "OsSocketCrypto RTP encrypt failed: PayloadLen=%d", payloadOctets);
         return 0;
      }
      struct rtpHeader* ph2 = (struct rtpHeader* )mCryptoBuffer;

      ph2->vpxcc &= ~RTP_P_MASK;
      ph2->mpt = (SRTP_PAYLOADTYPE_PCMU & RTP_PT_MASK) | (ph2->mpt & RTP_M_MASK);

      int pad = mDoPadding ? ((4 - (3 & encLen)) & 3) : 0;
      switch (pad)
      {
      case 3:
         payloadData[encLen+1] = 0;         /* fall through */
      case 2:
         payloadData[encLen] = 0;           /* fall through */
      case 1:
         payloadData[encLen+pad-1] = pad;
         ph2->vpxcc |= RTP_P_MASK;
      case 0:
         break;
      }


     return (mPureSocket->*mWrite)((const char*)mCryptoBuffer,
          sizeof(struct rtpHeader) + encLen + pad);
   }
   else
   {
      return (mPureSocket->*mWrite)(buffer, bufferLength);
   }
}

int OsSocketCryptoProxy::read(char* buffer, int bufferLength,
                              UtlString* fromAddress, int* fromPort)
{
   if (mpRtpDecryptKey == NULL)
   {
      return (mPureSocket->*mRead)(buffer, bufferLength, fromAddress, fromPort);
   }
 
   assert(MAX_CRYPTOBUFFER >= bufferLength);
   int res = (mPureSocket->*mRead)((char*)mDecodeBuffer, bufferLength, fromAddress, fromPort);
   if (res <= 0)
      return res;

   struct rtpHeader* ph = (struct rtpHeader* )mDecodeBuffer;

   unsigned char* payloadData = (unsigned char* )ph + sizeof(struct rtpHeader);
   UtlBoolean padding = ((ph->vpxcc & RTP_P_MASK) == RTP_P_MASK);

   int payloadOctets = res - sizeof(struct rtpHeader);
   if (padding)
   {
      uint8_t padBytes = buffer[bufferLength - 1];
      if ((padBytes & (~3)) != 0) {
         // For security reason
         padBytes = 0;
      }
      payloadOctets -= padBytes;
   }

   int payloadType = (ph->mpt & RTP_PT_MASK);
   
   if (SRTP_PAYLOADTYPE_PCMU == payloadType)
   {
      ph->vpxcc &= ~RTP_P_MASK;
      ph->mpt = (SDP_CODEC_PCMU & RTP_PT_MASK) | (ph->mpt & RTP_M_MASK);

      memcpy(buffer, mDecodeBuffer, sizeof(struct rtpHeader));

      UtlCryptoKey* pKey = mpRtpDecryptKey;
      int len = bufferLength - sizeof(struct rtpHeader);
      if (pKey->decrypt(payloadData, payloadOctets,
         (unsigned char*)buffer + sizeof(struct rtpHeader), &len) == 0)
      {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
            "OsSocketCrypto RTP decrypt failed: PayloadLen=%d", payloadOctets);
         return 0;
      }

      res = len + sizeof(struct rtpHeader);
   }
   else 
   {      
      memcpy(buffer, mDecodeBuffer, res);
   }
   return res;
}

/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */


/* ////////////////////////////// PROTECTED /////////////////////////////// */


/* /////////////////////////////// PRIVATE //////////////////////////////// */


/* ============================== FUNCTIONS =============================== */

#endif
