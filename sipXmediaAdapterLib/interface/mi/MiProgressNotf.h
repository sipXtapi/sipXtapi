//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

#ifndef _MiProgressNotf_h_
#define _MiProgressNotf_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "utl/UtlString.h"
#include "mi/MiNotification.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

  /// Message notification class used to communicate DTMF signaling.
class MiProgressNotf : public MiNotification
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor
   MiProgressNotf(const UtlString& sourceId, 
                  unsigned posMS, unsigned totalMS);

   /// Copy constructor
   MiProgressNotf(const MiProgressNotf& rNotf);

   /// Create a copy of this msg object (which may be of a derived type)
   virtual OsMsg* createCopy(void) const;

   /// Destructor
   virtual ~MiProgressNotf();

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /// Assignment operator
   MiProgressNotf& operator=(const MiProgressNotf& rhs);

     /// Set the position in milliseconds this message reports.
   void setPositionMS(unsigned posMS);

     /// Set the total size of the buffer in milliseconds this message reports.
   void setTotalMS(unsigned totalMS);

   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

     /// Get the current reported position of this progress update in MS.
   unsigned getPositionMS(void) const;

     /// Get the total size of the buffer in milliseconds this message reports.
   unsigned getTotalMS(void) const;

   //@}

   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   unsigned mCurPositionMS; ///< current position reported, in milliseconds.
   unsigned mTotalMS; ///< Total size in milliseconds.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MiProgressNotf_h_
