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
                       int (OsSocket::*read1)(char*, int),
                       int (OsSocket::*read2)(char*, int, UtlString*, int*),
                       int (OsSocket::*read3)(char*, int, struct in_addr*, int*),
                       int (OsSocket::*read4)(char*, int, long),
                       int (OsSocket::*write1)(const char*, int),
                       int (OsSocket::*write2)(const char*, int, const char*, int),
                       int (OsSocket::*write3)(const char*, int, long),
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

     /// @copydoc OsSocket::write(const char*, int)
   int write(const char* buffer, int bufferLength);

     /// @copydoc OsSocket::write(const char*, int, const char*, int)
   int write(const char* buffer, int bufferLength,
             const char* ipAddress, int port);

     /// @copydoc OsSocket::write(const char*, int, long)
   int write(const char* buffer, int bufferLength, long waitMilliseconds);


     /// @copydoc OsSocket::read(char*, int)
   int read(char* buffer, int bufferLength);

     /// @copydoc OsSocket::read(char*, int, UtlString*, int*)
   int read(char* buffer, int bufferLength,
            UtlString* fromAddress, int* fromPort);

     /// @copydoc OsSocket::read(char*, int, struct in_addr*, int*)
   int read(char* buffer, int bufferLength,
            struct in_addr* ipAddress, int* port);

     /// @copydoc OsSocket::read(char*, int, long)
   int read(char* buffer, int bufferLength, long waitMilliseconds);


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
   
     /// Decode from internal buffer 
   UtlBoolean decode(char* buffer, int bufferLength, int originalLength, int& decodedLen);

     /// Encode to internal buffer
   UtlBoolean encode(const char* buffer, int bufferLength, int& encodedLen);
   
     /// Read call being overridden in crypto class
   int (OsSocket::*mRead1)(char* buffer, int bufferLength);

     /// Read call being overridden in crypto class
   int (OsSocket::*mRead2)(char* buffer, int bufferLength,
                          UtlString* fromAddress, int* fromPort);

     /// Read call being overridden in crypto class
   int (OsSocket::*mRead3)(char* buffer, int bufferLength,
                           struct in_addr* ipAddress, int* port);

     /// Read call being overridden in crypto class
   int (OsSocket::*mRead4)(char* buffer, int bufferLength, long waitMilliseconds);

     /// Write call being overridden in crypto class   
   int (OsSocket::*mWrite1)(const char* buffer, int bufferLength);

     /// Write call being overridden in crypto class   
   int (OsSocket::*mWrite2)(const char* buffer, int bufferLength,
                            const char* ipAddress, int port);

     /// Write call being overridden in crypto class   
   int (OsSocket::*mWrite3)(const char* buffer, int bufferLength, 
                            long waitMilliseconds);
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
                 (int (OsSocket::*)(char*, int))&OsMulticastSocketCrypto::readProxy1,
                 (int (OsSocket::*)(char*, int, UtlString*, int*))&OsMulticastSocketCrypto::readProxy2,
                 (int (OsSocket::*)(char*, int, struct in_addr*, int*))&OsMulticastSocketCrypto::readProxy3,
                 (int (OsSocket::*)(char*, int, long))&OsMulticastSocketCrypto::readProxy4,
                 (int (OsSocket::*)(const char*, int))&OsMulticastSocketCrypto::writeProxy1,
                 (int (OsSocket::*)(const char*, int, const char*, int))&OsMulticastSocketCrypto::writeProxy2,
                 NULL,
                 pEncBinData, encBinLength, pDecBinData, decBinLength)
   {
   };

   int write(const char* buffer, int bufferLength)
   {
      return mCryptoProxy.write(buffer, bufferLength);
   }

   int write(const char* buffer, int bufferLength,
             const char* ipAddress, int port)
   {
      return mCryptoProxy.write(buffer, bufferLength, ipAddress, port);
   }
   
   int read(char* buffer, int bufferLength)
   {
      return mCryptoProxy.read(buffer, bufferLength);
   }

   int read(char* buffer, int bufferLength,
            UtlString* fromAddress, int* fromPort)
   {
      return mCryptoProxy.read(buffer, bufferLength, fromAddress, fromPort);
   }

   int read(char* buffer, int bufferLength,
            struct in_addr* ipAddress, int* port)
   {
      return mCryptoProxy.read(buffer, bufferLength, ipAddress, port);
   }

   int read(char* buffer, int bufferLength,
            long waitMilliseconds)
   {
      return mCryptoProxy.read(buffer, bufferLength, waitMilliseconds);
   }

protected:
   int writeProxy1(const char* buffer, int bufferLength)
   {
      return OsMulticastSocket::write(buffer, bufferLength);
   }

   int writeProxy2(const char* buffer, int bufferLength,
                  const char* ipAddress, int port)
   {
      return OsMulticastSocket::write(buffer, bufferLength, ipAddress, port);
   }


   int readProxy1(char* buffer, int bufferLength)
   {
      return OsMulticastSocket::read(buffer, bufferLength);
   }

   int readProxy2(char* buffer, int bufferLength,
                 UtlString* fromAddress, int* fromPort)
   {
      return OsSocket::read(buffer, bufferLength, fromAddress, fromPort);
   }

   int readProxy3(char* buffer, int bufferLength,
                 struct in_addr* ipAddress, int* port)
   {
      return OsSocket::read(buffer, bufferLength, ipAddress, port);
   }

   int readProxy4(char* buffer, int bufferLength,
                 long waitMilliseconds)
   {
      return OsSocket::read(buffer, bufferLength, waitMilliseconds);
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
                 (int (OsSocket::*)(char*, int))&OsNatDatagramSocketCrypto::readProxy1,
                 (int (OsSocket::*)(char*, int, UtlString*, int*))&OsNatDatagramSocketCrypto::readProxy2,
                 (int (OsSocket::*)(char*, int, struct in_addr*, int*))&OsNatDatagramSocketCrypto::readProxy3,
                 (int (OsSocket::*)(char*, int, long))&OsNatDatagramSocketCrypto::readProxy4,
                 (int (OsSocket::*)(const char*, int))&OsNatDatagramSocketCrypto::writeProxy1,
                 (int (OsSocket::*)(const char*, int, const char*, int))&OsNatDatagramSocketCrypto::writeProxy2,
                 (int (OsSocket::*)(const char*, int, long))&OsNatDatagramSocketCrypto::writeProxy3,
                 pEncBinData, encBinLength, pDecBinData, decBinLength)
   {
   };

   int write(const char* buffer, int bufferLength)
   {
      return mCryptoProxy.write(buffer, bufferLength);
   }

   int write(const char* buffer, int bufferLength,
             const char* ipAddress, int port)
   {
      return mCryptoProxy.write(buffer, bufferLength, ipAddress, port);
   }

   int write(const char* buffer, int bufferLength, long waitMilliseconds)
   {
      return mCryptoProxy.write(buffer, bufferLength, waitMilliseconds);
   }
   
   int read(char* buffer, int bufferLength)
   {
      return mCryptoProxy.read(buffer, bufferLength);
   }

   int read(char* buffer, int bufferLength,
            UtlString* fromAddress, int* fromPort)
   {
      return mCryptoProxy.read(buffer, bufferLength, fromAddress, fromPort);
   }

   int read(char* buffer, int bufferLength,
            struct in_addr* ipAddress, int* port)
   {
      return mCryptoProxy.read(buffer, bufferLength, ipAddress, port);
   }

   int read(char* buffer, int bufferLength,
            long waitMilliseconds)
   {
      return mCryptoProxy.read(buffer, bufferLength, waitMilliseconds);
   }

protected:
   int writeProxy1(const char* buffer, int bufferLength)
   {
      return OsNatDatagramSocket::write(buffer, bufferLength);
   }

   int writeProxy2(const char* buffer, int bufferLength,
                  const char* ipAddress, int port)
   {
      return OsNatDatagramSocket::write(buffer, bufferLength, ipAddress, port);
   }

   int writeProxy3(const char* buffer, int bufferLength,
                   long waitMilliseconds)
   {
      return OsNatDatagramSocket::write(buffer, bufferLength, waitMilliseconds);
   }


   int readProxy1(char* buffer, int bufferLength)
   {
      return OsNatDatagramSocket::read(buffer, bufferLength);
   }

   int readProxy2(char* buffer, int bufferLength,
                 UtlString* fromAddress, int* fromPort)
   {
      return OsNatDatagramSocket::read(buffer, bufferLength, fromAddress, fromPort);
   }

   int readProxy3(char* buffer, int bufferLength,
                 struct in_addr* ipAddress, int* port)
   {
      return OsNatDatagramSocket::read(buffer, bufferLength, ipAddress, port);
   }

   int readProxy4(char* buffer, int bufferLength,
                 long waitMilliseconds)
   {
      return OsNatDatagramSocket::read(buffer, bufferLength, waitMilliseconds);
   }
private:
   OsSocketCryptoProxy mCryptoProxy;
};

#endif  // _OsSocketCrypto_h_

#endif
