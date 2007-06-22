//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprBridgeSetGainsMsg_h_
#define _MprBridgeSetGainsMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpResourceMsg.h"
#include "mp/MprBridge.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message used to set gains in bridge mix matrix.
class MprBridgeSetGainsMsg : public MpResourceMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// Set row or column in bridge mix matrix.
   enum SetGainMsgType {
      GAINS_ROW,    ///< Set row in bridge mix matrix.
      GAINS_COLUMN  ///< Set column in bridge mix matrix.
   };

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprBridgeSetGainsMsg(const UtlString& targetResourceName,
                        int port,
                        int numGains,
                        const MpBridgeGain pGains[],
                        SetGainMsgType type)
      : MpResourceMsg(MPRM_BRIDGE_SET_GAINS, targetResourceName)
      , mPort(port)
      , mGainsNum(numGains)
      , mpGains(NULL)
      , mType(type)
   {
      mpGains = new MpBridgeGain[mGainsNum];
      memcpy(mpGains, pGains, mGainsNum*sizeof(MpBridgeGain));
   };

     /// Copy constructor
   MprBridgeSetGainsMsg(const MprBridgeSetGainsMsg& resourceMsg)
      : MpResourceMsg(resourceMsg)
      , mPort(resourceMsg.mPort)
      , mGainsNum(resourceMsg.mGainsNum)
      , mpGains(NULL)
      , mType(resourceMsg.mType)
   {
      mpGains = new MpBridgeGain[mGainsNum];
      memcpy(mpGains, resourceMsg.mpGains, mGainsNum*sizeof(MpBridgeGain));
   };

     /// Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const 
   {
      return new MprBridgeSetGainsMsg(*this); 
   }

     /// Destructor
   ~MprBridgeSetGainsMsg() 
   {
      delete[] mpGains;
   };

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   MprBridgeSetGainsMsg& operator=(const MprBridgeSetGainsMsg& rhs)
   {
      if(&rhs == this)
      {
         return(*this);
      }

      mPort = rhs.mPort;
      mType = rhs.mType;

      if (rhs.mGainsNum != mGainsNum)
      {
         delete[] mpGains;

         mGainsNum = rhs.mGainsNum;
         mpGains = new MpBridgeGain[mGainsNum];
      }
      memcpy(mpGains, rhs.mpGains, mGainsNum*sizeof(MpBridgeGain));

      return *this;
   }

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   int getPort() const {return mPort;}

     /// Return number of values in gains array.
   int getGainsNum() const {return mGainsNum;}

     /// Return pointer to gains array.
   const MpBridgeGain *getGains() const {return mpGains;}

     /// Should row or column be set?
   SetGainMsgType getType() const {return mType;}

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int             mPort;      ///< Input or output port (row or column number)
                               ///< for which gains are set.
   int             mGainsNum;  ///< Number of elements in mpGains array.
   MpBridgeGain   *mpGains;    ///< Array of gains to set.
   SetGainMsgType  mType;      ///< Should row or column be set?
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprBridgeSetGainsMsg_h_
