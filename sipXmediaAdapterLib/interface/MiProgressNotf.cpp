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


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mi/MiProgressNotf.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// Message notification class used to communicate DTMF signaling.

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MiProgressNotf::MiProgressNotf(const UtlString& sourceId, 
                               unsigned posMS, unsigned totalMS)
: MiNotification(MI_NOTF_PROGRESS, sourceId)
, mCurPositionMS(posMS)
, mTotalMS(totalMS)
{
}

// Copy constructor
MiProgressNotf::MiProgressNotf(const MiProgressNotf& rMsg)
: MiNotification(rMsg)
, mCurPositionMS(rMsg.mCurPositionMS)
, mTotalMS(rMsg.mTotalMS)
{
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* MiProgressNotf::createCopy(void) const
{
   return new MiProgressNotf(*this);
}

// Destructor
MiProgressNotf::~MiProgressNotf()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
MiProgressNotf& 
MiProgressNotf::operator=(const MiProgressNotf& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   MiNotification::operator=(rhs);       // assign fields for parent class

   mCurPositionMS = rhs.mCurPositionMS;
   mTotalMS = rhs.mTotalMS;

   return *this;
}

// Set the position in milliseconds this message reports.
void MiProgressNotf::setPositionMS(unsigned posMS)
{
   mCurPositionMS = posMS;
}

// Set the total size of the buffer in milliseconds this message reports.
void MiProgressNotf::setTotalMS(unsigned totalMS)
{
   mTotalMS = totalMS;
}

/* ============================ ACCESSORS ================================= */

/// Get the current reported position of this progress update in MS.
unsigned MiProgressNotf::getPositionMS(void) const
{
   return mCurPositionMS;
}

/// Get the total size of the buffer in milliseconds this message reports.
unsigned MiProgressNotf::getTotalMS(void) const
{
   return mTotalMS;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
