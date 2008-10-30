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

#ifndef _OsSocketCrypto_h_
#define _OsSocketCrypto_h_

// SYSTEM INCLUDES
#include <assert.h>
// APPLICATION INCLUDES
#include "os/OsSocket.h"
#include "utl/UtlCryptoData.h"
#include "utl/UtlCryptoKey.h"

#include "os/OsMulticastSocket.h"
#include "os/OsDatagramSocket.h"
#include "os/OsNatDatagramSocket.h"

// DEFINES
#define MAX_CRYPTOBUFFER   3000

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Encrypted socket base.
*
*  This socket is very limited in its operation:
*  1) it blindly assume that flowing data is RTP stream(s)
*  2) it encode/decode only RTP payload (headers are not changed)
*  3) it only encode/decode packets with selected payload type
*
*  Read the code for more details.
*/
class OsSocketCryptoProxy
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Constructor
   OsSocketCryptoProxy(OsSocket* pureSocket,
                       int (OsSocket::*read)(char*, int, UtlString*, int*),
                       int (OsSocket::*write)(const char*, int),
                       const char* pEncBinData,
                       int encBinLength,
                       const char* pDecBinData,
                       int decBinLength);

     /// Destructor
   virtual ~OsSocketCryptoProxy();

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

     /// @copydoc OsSocket::write
   int write(const char* buffer, int bufferLength);

     /// @copydoc OsSocket::read
   int read(char* buffer, int bufferLength,
            UtlString* fromAddress, int* fromPort);

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{


//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{


//@}

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:


/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:

     /// Set encryption and decryption keys
   UtlBoolean setSymKeys(const char* pEncBinData,
                         int encBinLength,
                         const char* pDecBinData,
                         int decBinLength);

   UtlBoolean mDoPadding;
   uint8_t mCryptoBuffer[MAX_CRYPTOBUFFER];
   uint8_t mDecodeBuffer[MAX_CRYPTOBUFFER];

   UtlCryptoKey* mpRtpEncryptKey;  ///< Encryption key
   UtlCryptoKey* mpRtpDecryptKey;  ///< Decryption key

   OsSocket* mPureSocket;          ///< Basic socket object
   
     /// Read call being overridden in crypto class
   int (OsSocket::*mRead)(char* buffer, int bufferLength,
                          UtlString* fromAddress, int* fromPort);

     /// Write call being overridden in crypto class   
   int (OsSocket::*mWrite)(const char* buffer, int bufferLength);
};

/* ============================ INLINE METHODS ============================ */


//////////////////////////////////////////////////////////////////////////

class OsMulticastSocketCrypto : public OsMulticastSocket
{
public:
   OsMulticastSocketCrypto(int multicastPortNum,
                           const char* multicastHost,
                           int localHostPortNum,
                           const char* localHost,
                           const char* pEncBinData,
                           int encBinLength,
                           const char* pDecBinData,
                           int decBinLength)
   : OsMulticastSocket(multicastPortNum, multicastHost, localHostPortNum, localHost)
   , mCryptoProxy(this, 
                 (int (OsSocket::*)(char*, int, UtlString*, int*))&OsMulticastSocketCrypto::readProxy,
                 (int (OsSocket::*)(const char*, int))&OsMulticastSocketCrypto::writeProxy,
                 pEncBinData, encBinLength, pDecBinData, decBinLength)
   {
   };

   int write(const char* buffer, int bufferLength)
   {
      return mCryptoProxy.write(buffer, bufferLength);
   }
   
   int read(char* buffer, int bufferLength,
      UtlString* fromAddress, int* fromPort)
   {
      return mCryptoProxy.read(buffer, bufferLength, fromAddress, fromPort);
   }

protected:
   int writeProxy(const char* buffer, int bufferLength)
   {
      return OsMulticastSocket::write(buffer, bufferLength);
   }

   int readProxy(char* buffer, int bufferLength,
                 UtlString* fromAddress, int* fromPort)
   {
      return OsSocket::read(buffer, bufferLength, fromAddress, fromPort);
   }

private:
   OsSocketCryptoProxy mCryptoProxy;
};

//////////////////////////////////////////////////////////////////////////

class OsNatDatagramSocketCrypto : public OsNatDatagramSocket
{
public:
   OsNatDatagramSocketCrypto(int remoteHostPortNum,
                             const char* remoteHost, 
                             int localHostPortNum, 
                             const char* localHost,
                             OsNotification *pNotification,
                             const char* pEncBinData,
                             int encBinLength,
                             const char* pDecBinData,
                             int decBinLength)
   : OsNatDatagramSocket(remoteHostPortNum, remoteHost, localHostPortNum, localHost, pNotification)
   , mCryptoProxy(this, 
                 (int (OsSocket::*)(char*, int, UtlString*, int*))&OsNatDatagramSocketCrypto::readProxy,
                 (int (OsSocket::*)(const char*, int))&OsNatDatagramSocketCrypto::writeProxy,
                 pEncBinData, encBinLength, pDecBinData, decBinLength)
   {
   };

   int write(const char* buffer, int bufferLength)
   {
      return mCryptoProxy.write(buffer, bufferLength);
   }

   int read(char* buffer, int bufferLength,
            UtlString* fromAddress, int* fromPort)
   {
      return mCryptoProxy.read(buffer, bufferLength, fromAddress, fromPort);
   }

protected:
   int writeProxy(const char* buffer, int bufferLength)
   {
      return OsNatDatagramSocket::write(buffer, bufferLength);
   }

   int readProxy(char* buffer, int bufferLength,
                 UtlString* fromAddress, int* fromPort)
   {
      return OsNatDatagramSocket::read(buffer, bufferLength, fromAddress, fromPort);
   }

private:
   OsSocketCryptoProxy mCryptoProxy;
};

#endif  // _OsSocketCrypto_h_

#endif
