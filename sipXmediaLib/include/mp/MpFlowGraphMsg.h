//  
// Copyright (C) 2006-2013 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2009 SIPfoundry Inc.
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
      FLOWGRAPH_ADD_LINK = 0,
      FLOWGRAPH_ADD_RESOURCE,
      FLOWGRAPH_DESTROY_RESOURCES,
      FLOWGRAPH_DISABLE,
      FLOWGRAPH_ENABLE,
      FLOWGRAPH_PROCESS_FRAME,
      FLOWGRAPH_REMOVE_CONNECTION,
      FLOWGRAPH_REMOVE_LINK,
      FLOWGRAPH_REMOVE_RESOURCE,
      FLOWGRAPH_DESTROY_RESOURCE,
      FLOWGRAPH_START,
      FLOWGRAPH_STOP,
      RESOURCE_DISABLE,
      RESOURCE_ENABLE,

      FLOWGRAPH_START_PLAY,
      FLOWGRAPH_START_TONE,

      FLOWGRAPH_STOP_PLAY,
      FLOWGRAPH_STOP_TONE,
      FLOWGRAPH_STOP_RECORD,

      FLOWGRAPH_SYNCHRONIZE,

      FLOWGRAPH_GET_LATENCY_FOR_PATH,

      RESOURCE_SPECIFIC_START = 100     ///< start of resource-specific messages
   } MpFlowGraphMsgType;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpFlowGraphMsg(int msg, MpResource* pMsgDest=NULL,
                  void* pPtr1=NULL, void* pPtr2=NULL,
                  intptr_t int1=-1, intptr_t int2=-1);

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
   void setInt1(intptr_t i);

     /// Sets integer 2 of the media flow graph message
   void setInt2(intptr_t i);

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
   intptr_t getInt1(void) const;

     /// Return integer 2 of the media flow graph message
   intptr_t getInt2(void) const;

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
   intptr_t    mInt1;     ///< Integer data 1
   intptr_t    mInt2;     ///< Integer data 2

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpFlowGraphMsg_h_
