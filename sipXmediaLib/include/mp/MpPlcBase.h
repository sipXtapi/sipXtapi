//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _MpPlcBase_h_
#define _MpPlcBase_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlDefs.h>
#include <os/OsStatus.h>
#include <utl/UtlString.h>
#include "mp/MpTypes.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  Base class for all PLC algorithms.
*
*  To create concrete class you could directly instantiate it or use
*  MpPlcBase::createPlc() static method for greater flexibility.
*
*  @nosubgrouping
*/
class MpPlcBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

   static MpPlcBase *createPlc(const UtlString &plcName);

     /// Initialize PLC with given sample rate and frame size.
   virtual OsStatus init(int samplesPerSec, int samplesPerFrame) = 0;
     /**
     *  Should be called before any other class methods.
     */

     /// Virtual base destructor.
   virtual ~MpPlcBase() {};

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Process next frame - update PLC history and do PLC if needed.
   virtual OsStatus processFrame(int inFrameNum,
                                 int outFrameNum,
                                 const MpAudioSample* in,
                                 MpAudioSample* out,
                                 UtlBoolean* signalModified) = 0;

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Return maximum offset for delayed frame with respect to current
   virtual int getMaxDelayedFramesNum() const = 0;
     /**<
     *  If zero is returned, late packets are not allowed.
     */

     /// Return maximum offset for future frame with respect to current
   virtual int getMaxFutureFramesNum() const = 0;

     /// Return algorithmic delay in samples, should be called after init
   virtual int getAlgorithmicDelay() const = 0;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif //_MpPlcBase_h_
