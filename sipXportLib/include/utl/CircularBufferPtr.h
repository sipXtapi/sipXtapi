//
// Copyright (C) 2015 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef _CircularBufferPtr_h_
#define _CircularBufferPtr_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include <os/OsMutex.h>
#include "CircularBuffer.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Reference counted pointer for Circular array of buffer data
// TBD
class CircularBufferPtr : public CircularBuffer
{
public:
    CircularBufferPtr(unsigned long capacity = 0);

    void addRef();
    void release();

protected:
    ~CircularBufferPtr();

private:
    int     mRefCount;
    OsMutex mMutex;
};

/* ============================ INLINE METHODS ============================ */
#endif  // _CircularBufferPtr_h_

