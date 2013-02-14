//  
// Copyright (C) 2008-2013 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprDecodeSelectCodecsMsg_h_
#define _MprDecodeSelectCodecsMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "mp/MpResourceMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message object used to communicate with the media processing task
class MprDecodeSelectCodecsMsg : public MpResourceMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprDecodeSelectCodecsMsg(const UtlString& targetResourceName,
                            SdpCodec* codecs[],
                            int numCodecs)
      : MpResourceMsg(MPRM_DECODE_SELECT_CODECS, targetResourceName)
      , mpCodecs(NULL)
      , mNumCodecs(0)
   {
      //UtlString codecMediaType;

      // Allocate new array
      mpCodecs = new SdpCodec*[numCodecs];

      // Copy all audio codecs to new array
      for (int i=0; i<numCodecs; i++)
      {
         if (codecs[i] != NULL)
         {
            mpCodecs[mNumCodecs] = new SdpCodec(*codecs[i]);
            mNumCodecs++;
         }
         else
         {
            mpCodecs[mNumCodecs] = NULL;
            mNumCodecs++;
         }
      }
   };

     /// Destructor
   ~MprDecodeSelectCodecsMsg() 
   {
      clearCodecs();
   };

   /// Copy constructor
   MprDecodeSelectCodecsMsg(const MprDecodeSelectCodecsMsg& resourceMsg)
      : MpResourceMsg(resourceMsg)
      , mpCodecs(NULL)
      , mNumCodecs(0)
   {
      copyCodecs(resourceMsg.mpCodecs, resourceMsg.mNumCodecs);
   };

   /// Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const 
   {
      return new MprDecodeSelectCodecsMsg(*this); 
   }

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   MprDecodeSelectCodecsMsg& operator=(const MprDecodeSelectCodecsMsg& rhs)
   {
        if(&rhs == this)
        {
            return(*this);
        }

        MpResourceMsg::operator=(rhs);
        clearCodecs();
        copyCodecs(rhs.mpCodecs, rhs.mNumCodecs);

        return *this;
   }

//@}
/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   int getNumCodecs() const
   {
      return mNumCodecs;
   }

   SdpCodec** getCodecs() const
   {
      return mpCodecs;
   }

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   SdpCodec** mpCodecs;
   int        mNumCodecs;

   void clearCodecs()
   {
      for (int i=0; i<mNumCodecs; i++)
      {
         delete mpCodecs[i];
      }
      delete[] mpCodecs;
   }

   void copyCodecs(SdpCodec* pCodecs[],
                   int numCodecs)
   {
      // Allocate new array
      mpCodecs = new SdpCodec*[numCodecs];

      // Copy all audio codecs to new array
      for (int i=0; i<numCodecs; i++) {
         if (pCodecs[i] != NULL)
         {
            mpCodecs[mNumCodecs] = new SdpCodec(*pCodecs[i]);
         }
         else
         {
            mpCodecs[mNumCodecs] = NULL;
         }
         mNumCodecs++;
      }
   }
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprDecodeSelectCodecsMsg_h_
