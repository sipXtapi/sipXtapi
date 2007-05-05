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
                              const UtlString& filename,
                              const UtlBoolean& repeat,
                              OsNotification* evt = NULL)
      : MpResourceMsg(MPRM_FROMFILE_START, msgDestName)
      , mFilename(filename)
      , mRepeat(repeat)
      , mpEvt(evt)
   {};

   /// Copy constructor
   MpFromFileStartResourceMsg(const MpFromFileStartResourceMsg& rMpResourceMsg)
      : MpResourceMsg(rMpResourceMsg)
      , mFilename(rMpResourceMsg.mFilename)
      , mRepeat(rMpResourceMsg.mRepeat)
      , mpEvt(rMpResourceMsg.mpEvt)
   {};

   /// Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const 
   {
      return new MpFromFileStartResourceMsg(*this); 
   }

   /// Destructor
   ~MpFromFileStartResourceMsg() { /* No work required */ };

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
      mFilename = rhs.mFilename;
      mRepeat = rhs.mRepeat;
      mpEvt = rhs.mpEvt;
      return *this;
   }

     /// @brief Set the filename that is associated with this resource.
   inline void setFilename(const UtlString& filename) { mFilename = filename; }
     /**<
     *  Set the filename that is associated with this resource.
     *  @param filename the new filename that is to be set in this resource.
     */

     /// @brief Set whether or not this file loops when it plays.
   inline void setRepeat(const UtlBoolean& repeat) { mRepeat = repeat; }
     /**<
     *  Set whether or not this file loops when it plays.
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

     /// Get the filename that is associated with this resource.
   inline UtlString getFilename(void) const { return mFilename; };
     /**<
     *  Returns the filename that is associated with this resource.
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

     /// Return whether this filename is to repeat or not.
   inline UtlBoolean isRepeating(void) const { return mRepeat; };
     /**<
     *  Return whether this filename is to repeat or not.
     */

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlString mFilename; ///< The filename of the file to play.
   UtlBoolean mRepeat; ///< Whether or not to loop-play this.
   OsNotification* mpEvt; ///< Pointer to OsNotification that signals when play status changes.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpFromFileStartResourceMsg_h_
