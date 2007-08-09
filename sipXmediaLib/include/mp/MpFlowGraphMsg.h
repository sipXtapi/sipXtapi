//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpFlowGraphMsg_h_
#define _MpFlowGraphMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class MpResource;

/// Message object used to communicate with the media processing task
class MpFlowGraphMsg : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// Phone set message types
   typedef enum
   {
      FLOWGRAPH_ADD_LINK,
      FLOWGRAPH_ADD_RESOURCE,
      FLOWGRAPH_DESTROY_RESOURCES,
      FLOWGRAPH_DISABLE,
      FLOWGRAPH_ENABLE,
      FLOWGRAPH_PROCESS_FRAME,
      FLOWGRAPH_REMOVE_CONNECTION,
      FLOWGRAPH_REMOVE_LINK,
      FLOWGRAPH_REMOVE_RESOURCE,
      FLOWGRAPH_SET_SAMPLES_PER_FRAME,
      FLOWGRAPH_SET_SAMPLES_PER_SEC,
      FLOWGRAPH_START,
      FLOWGRAPH_STOP,
      RESOURCE_DISABLE,
      RESOURCE_ENABLE,
      RESOURCE_SET_SAMPLES_PER_FRAME,
      RESOURCE_SET_SAMPLES_PER_SEC,

      FLOWGRAPH_START_PLAY,
      FLOWGRAPH_START_TONE,
      FLOWGRAPH_START_RECORD,

      FLOWGRAPH_STOP_PLAY,
      FLOWGRAPH_STOP_TONE,
      FLOWGRAPH_STOP_RECORD,
      
      FLOWGRAPH_SYNCHRONIZE,

      FLOWGRAPH_SET_PREMIUM_SOUND,

      FLOWGRAPH_SET_DTMF_NOTIFY,

      // Resource notification messages using old flowgraph msg approach
      // NOTE: this is a temporary fix, and will be replaced by newer
      //       notification messaging.  (see MpResourceNotificationMsg.h and 
      //       OsMsgDispatcher)
      ON_MPRRECORDER_ENABLED,    ///< sent when recording is started
      ON_MPRRECORDER_DISABLED,   ///< sent when recording is stopped

      RESOURCE_SPECIFIC_START = 100     ///< start of resource-specific messages
   } MpFlowGraphMsgType;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpFlowGraphMsg(int msg, MpResource* pMsgDest=NULL,
                  void* pPtr1=NULL, void* pPtr2=NULL,
                  int int1=-1, int int2=-1);

     /// Copy constructor
   MpFlowGraphMsg(const MpFlowGraphMsg& rMpFlowGraphMsg);

     /// Create a copy of this msg object (which may be of a derived type)
   virtual OsMsg* createCopy(void) const;

     /// Destructor
   virtual
   ~MpFlowGraphMsg();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   MpFlowGraphMsg& operator=(const MpFlowGraphMsg& rhs);

     /// Set destination object of the message.
   void setMsgDest(MpResource* pMsgDest);
     /**<
     *  Sets the intended recipient for this message.  Setting the message 
     *  destination to NULL indicates that the message is intended for the 
     *  flow graph itself.
     */

     /// Sets pointer 1 (void*) of the media flow graph message
   void setPtr1(void* p);

     /// Sets pointer 2 (void*) of the media flow graph message
   void setPtr2(void* p);

     /// Sets integer 1 of the media flow graph message
   void setInt1(int i);

     /// Sets integer 2 of the media flow graph message
   void setInt2(int i);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Returns the type of the media flow graph message
   int getMsg(void) const;

     /// Get destination object of the message.
   MpResource* getMsgDest(void) const;
     /**<
     *  Returns the MpResource object that is the intended recipient for this 
     *  message.  A NULL return indicates that the message is intended for 
     *  the flow graph itself.
     */

     /// Return pointer 1 (void*) of the media flow graph message
   void* getPtr1(void) const;

     /// Return pointer 2 (void*) of the media flow graph message
   void* getPtr2(void) const;

     /// Return integer 1 of the media flow graph message
   int getInt1(void) const;

     /// Return integer 2 of the media flow graph message
   int getInt2(void) const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   MpResource* mpMsgDest; ///< Intended recipient for this message
   void*       mpPtr1;    ///< Pointer to data 1
   void*       mpPtr2;    ///< Pointer to data 2
   int         mInt1;     ///< Integer data 1
   int         mInt2;     ///< Integer data 2

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpFlowGraphMsg_h_
