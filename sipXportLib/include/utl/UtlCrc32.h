//
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _UrlCrc32_h_
#define _UrlCrc32_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "utl/UtlString.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * Utility class for generating CRC-32s
 */
class UtlCrc32 
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    /**
     * Default no-argument constructor
     */
    UtlCrc32() ;

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

    /**
     * Reset the Crc32 value
     */
    void reset() ;

    /**
     * Adds a single byte to the checksum
     */
    void calc(unsigned char ch) ;

    /**
     * Adds a sequence of bytes to the checksum
     */
    void calc(unsigned char* pData, size_t nData) ;

    /**
     * Adds a sequence of bytes contained within a UtlString to the checksum
     */
    void calc(const UtlString& data) ;

/* ============================ ACCESSORS ================================= */

    /**
     * Get the current CRC-32 value
     */
    unsigned long getValue() const ;

/* ============================ INQUIRY =================================== */    

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    unsigned long mCrc ;
   
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   UtlCrc32(const UtlCrc32& rUtlCrc32);
     //:Copy constructor (not implemented for this class)

   UtlCrc32& operator=(const UtlCrc32& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _UrlCrc32_h_
