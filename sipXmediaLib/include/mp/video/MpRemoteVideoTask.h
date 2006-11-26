//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 


#ifndef _MpRemoteVideoTask_h_
#define _MpRemoteVideoTask_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsTask.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MprDejitter;
class MpdH264;
class MpvoGdi;

/// This thread display video stream, coming from remote party.
class MpRemoteVideoTask : public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

   MpRemoteVideoTask(MprDejitter* pDejitter, void *hwnd=NULL);

   virtual
   ~MpRemoteVideoTask();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   void setRemoteVideoWindow(const void *hwnd);

   void step();

   OsStatus stop();

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   void *getRemoteVideoWindow() const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   
   MprDejitter *mpDejitter; ///< We will get RTP packet from this resource.
   MpdH264     *mpDecoder;  ///< Encoder for captured frames.
   MpvoGdi     *mpVideoOut; ///< Video output system.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Main loop of the task.
   virtual int run(void* pArg);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpRemoteVideoTask_h_
