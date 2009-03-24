//  
// Copyright (C) 2009 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprHook_h_
#define _MprHook_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpAudioResource.h"
#include "mp/MpResourceMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief The "Hook" media processing resource.
*
*  This resource allows some external entity to read and possibly modify
*  buffers.
*/
class MprHook : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

     /// Pointer to a hook function.
   typedef void (*HookFunction)(MpBufPtr &pBuf, uint32_t time, void *pUserData);
     /**<
     * @param pBuf - audio data. May be NULL.
     * @param time - timestamp of audio data (in frames).
     */

/* ============================ CREATORS ================================== */
///@name Creators
//@{
     /// Constructor
   MprHook(const UtlString& rName);

     /// Destructor
   virtual
   ~MprHook();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Set hook function to be called for every received frame.
   static OsStatus setHook(const UtlString& namedResource,
                           OsMsgQ& fgQ,
                           HookFunction func,
                           void *pUserData);
     /**<
     * Note, that you have to manage userdata lifetime by yourself.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   typedef enum
   {
      MPRM_SET_HOOK = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START
   } AddlMsgTypes;

   HookFunction  mpHook;     ///< Hook function to be called.
   void         *mpUserData; ///< User data to pass to hook function.
   uint32_t      mFrameNum;  ///< Number of the current frame.

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond);

     /// @copydoc MpResource::handleMessage()
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// Copy constructor (not implemented for this class)
   MprHook(const MprHook& rMprHook);

     /// Assignment operator (not implemented for this class)
   MprHook& operator=(const MprHook& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprHook_h_
