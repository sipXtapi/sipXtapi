//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpFromFileStartResourceMsg_h_
#define _MpFromFileStartResourceMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "mp/MpResourceMsg.h"
#include "utl/UtlString.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message object used to communicate with the media processing task
class MpFromFileStartResourceMsg : public MpResourceMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor
   MpFromFileStartResourceMsg(const UtlString& msgDestName, 
                              UtlString* pAudioBuf,
                              const UtlBoolean& repeat,
                              OsNotification* evt = NULL)
      : MpResourceMsg(MPRM_FROMFILE_START, msgDestName)
      , mpAudioBuf(pAudioBuf)
      , mRepeat(repeat)
      , mpEvt(evt)
   {};

   /// Copy constructor
   MpFromFileStartResourceMsg(const MpFromFileStartResourceMsg& rMpResourceMsg)
      : MpResourceMsg(rMpResourceMsg)
      , mpAudioBuf(rMpResourceMsg.mpAudioBuf)
      , mRepeat(rMpResourceMsg.mRepeat)
      , mpEvt(rMpResourceMsg.mpEvt)
   {};

   /// Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const 
   {
      return new MpFromFileStartResourceMsg(*this); 
   }

   /// Destructor
   ~MpFromFileStartResourceMsg() 
   { /* No work required -- audio buf pointer ownership passes on to msg recipient */ };

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /// Assignment operator
   MpFromFileStartResourceMsg& operator=(const MpFromFileStartResourceMsg& rhs)
   {
      if (this == &rhs) 
         return *this;  // handle the assignment to self case

      MpResourceMsg::operator=(rhs);  // assign fields for parent class
      mpAudioBuf = rhs.mpAudioBuf;
      mRepeat = rhs.mRepeat;
      mpEvt = rhs.mpEvt;
      return *this;
   }

     /// @brief Set the audio buffer that is associated with this resource.
   inline void setAudioBuffer(UtlString* pAudioBuf) { mpAudioBuf = pAudioBuf; }
     /**<
     *  Set the audio buffer that is associated with this resource.
     *  Ownership transfers to the recipient of this message.
     *  @param pAudioBuf the new audio buffer that is to be set in this resource.
     */

     /// @brief Set whether or not this audio buffer loops when it plays.
   inline void setRepeat(const UtlBoolean& repeat) { mRepeat = repeat; }
     /**<
     *  Set whether or not this audio buffer loops when it plays.
     *  @param repeat the new repeat status that is to be set in this resource.
     */

     /// @brief Set the OsNotification pointer that is associated with this resource.
   inline void setOsNotification(OsNotification* evt) { mpEvt = evt; }
     /**<
     *  Set the OsNotification pointer that is associated with this resource.
     *  @param evt - the new OsNotification pointer used to signal 
     *         when play status changes.
     */

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

     /// Get the audio buffer that is associated with this resource.
   inline UtlString* getAudioBuffer(void) const { return mpAudioBuf; };
     /**<
     *  Returns the audio buffer that is associated with this resource.
     */

   /// Get the OsNotification pointer that is associated with this resource.
   inline OsNotification* getOsNotification(void) const { return mpEvt; };
   /**<
   *  Returns the OsNotification that is associated with this resource.
   */

   //@}

   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

     /// Return whether this bit of audio is to repeat or not.
   inline UtlBoolean isRepeating(void) const { return mRepeat; };
     /**<
     *  Return whether this bit of audio is to repeat or not.
     */

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlString* mpAudioBuf; ///< The audio buffer to play.
   UtlBoolean mRepeat; ///< Whether or not to loop-play this.
   OsNotification* mpEvt; ///< Pointer to OsNotification that signals when play status changes.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpFromFileStartResourceMsg_h_
