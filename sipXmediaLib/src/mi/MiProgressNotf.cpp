// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
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
