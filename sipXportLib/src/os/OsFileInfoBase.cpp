//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsFileInfoBase.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsFileInfoBase::OsFileInfoBase()
{
}

// Copy constructor
OsFileInfoBase::OsFileInfoBase(const OsFileInfoBase& rOsFileInfoBase)
{
}

// Destructor
OsFileInfoBase::~OsFileInfoBase()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsFileInfoBase&
OsFileInfoBase::operator=(const OsFileInfoBase& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */
OsStatus OsFileInfoBase::getCreateTime(OsTime& time) const
{
    OsStatus stat = OS_SUCCESS;
    time = mCreateTime;
    return stat;

}

OsStatus OsFileInfoBase::getModifiedTime(OsTime& time) const
{
    OsStatus stat = OS_SUCCESS;
    time = mModifiedTime;
    return stat;

}

OsStatus OsFileInfoBase::getSize(unsigned long& size) const
{
    OsStatus stat = OS_SUCCESS;
    size = mSize;
    return stat;
}

/* ============================ INQUIRY =================================== */

UtlBoolean  OsFileInfoBase::isReadOnly() const
{

    return mbIsReadOnly;
}

UtlBoolean  OsFileInfoBase::isDir() const
{

    return mbIsDirectory;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */



