//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>
#include <string.h>

// APPLICATION INCLUDES
#include <net/NetBase64Codec.h>
#include <os/OsDefs.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const char* NetBase64Codec::base64Codes =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
NetBase64Codec::NetBase64Codec()
{
}

// Copy constructor
NetBase64Codec::NetBase64Codec(const NetBase64Codec& rNetBase64Codec)
{
}

// Destructor
NetBase64Codec::~NetBase64Codec()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
NetBase64Codec&
NetBase64Codec::operator=(const NetBase64Codec& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void NetBase64Codec::encode(int dataSize, const char data[],
        int& encodedDataSize, char encodedData[])
{
  int i;
  int j = 0;
  encodedDataSize = encodedSize(dataSize);

  for (i = 0; i < (dataSize - (dataSize % 3)); i+=3)   // Encode 3 bytes at a time.
    {
      encodedData[j]   = base64Codes[ (data[i] & 0xfc) >> 2 ];
      encodedData[j+1] = base64Codes[ ((data[i] & 0x03) << 4)   | ((data[i+1] & 0xf0) >> 4) ];
      encodedData[j+2] = base64Codes[ ((data[i+1] & 0x0f) << 2) | ((data[i+2] & 0xc0) >> 6) ];
      encodedData[j+3] = base64Codes[ (data[i+2] & 0x3f) ];
      j += 4;
    }

  i = dataSize - (dataSize % 3);  // Where we left off before.
  switch (dataSize % 3)
    {
      case 2:  // One character padding needed.
        {
          encodedData[j] = base64Codes[ (data[i] & 0xfc) >> 2 ];
          encodedData[j+1] = base64Codes[ ((data[i] & 0x03) << 4) | ((data[i+1] & 0xf0) >> 4) ];
          encodedData[j+2] = base64Codes[ (data[i+1] & 0x0f) << 2 ];
          encodedData[j+3] = base64Codes[64];  // Pad
          break;
        }
      case 1:  // Two character padding needed.
        {
          encodedData[j] = base64Codes[ (data[i] & 0xfc) >> 2 ];
          encodedData[j+1] = base64Codes[ (data[i] & 0x03) << 4 ];
          encodedData[j+2] = base64Codes[64];  // Pad
          encodedData[j+3] = base64Codes[64];  // Pad
          break;
        }
    }
  // encodedData[j+4] = NULL;
}


void NetBase64Codec::encode(int dataSize, const char data[], UtlString& encodedData)
{
    int numEncodeBytes = encodedSize(dataSize);
    char* encodeBuffer = new char[numEncodeBytes];

    encode(dataSize, data, numEncodeBytes, encodeBuffer);
    encodedData.remove(0);
    encodedData.append(encodeBuffer, numEncodeBytes);

    delete encodeBuffer;
    encodeBuffer = NULL;
}

int NetBase64Codec::encodedSize(int dataSize)
{
        int size = dataSize / 3;
        if(dataSize % 3) size++;

        size *= 4;

        return(size);
}

char NetBase64Codec::decodeChar(const char encoded)
{
        return(encoded == '=' ? 0 : strchr(base64Codes, encoded) - base64Codes );
}

void NetBase64Codec::decode(int encodedDataSize, const char encodedData[],
        int& dataSize, char data[])
{
  int i;
  int j = 0;
  dataSize = decodedSize(encodedDataSize, encodedData);

  for (i = 0; i < encodedDataSize; i+=4) // Work on 4 bytes at a time.
    {                         // Twiddle bits.
      data[j]   = (decodeChar(encodedData[i]) << 2) |
                  ((decodeChar(encodedData[i+1]) & 0x30) >> 4);
          if(j + 1 < dataSize)
          {
                  data[j+1] = ((decodeChar(encodedData[i+1]) & 0x0f) << 4) |
                          ((decodeChar(encodedData[i+2]) & 0x3c) >> 2);

                  if(j + 2 < dataSize)
                  {
                  data[j+2] = ((decodeChar(encodedData[i+2]) & 0x03) << 6) |
                          (decodeChar(encodedData[i+3]) & 0x3f);
                  }
          }
      j += 3;
    }
}

int NetBase64Codec::decodedSize(int encodedDataSize, const char encodedData[])
{
        int size = encodedDataSize / 4 * 3;

        if(encodedData[encodedDataSize - 1] == base64Codes[64])
        {
                size--;
                if(encodedData[encodedDataSize - 2] == base64Codes[64])
                {
                        size--;
                }
        }

        return(size);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
