// Copyright (C) 2005-2015 SIPez LLC.  All rights reserved.
// 
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _TypeConverter_h_
#define _TypeConverter_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <mi/CpMediaInterface.h>
#include <mp/MprRecorder.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


class TypeConverter
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
    virtual ~TypeConverter();

/* ============================ MANIPULATORS ============================== */
	static OsStatus translateRecordingFormat(CpMediaInterface::CpAudioFileFormat cpFileFormat,
		                                     MprRecorder::RecordFileFormat & recordFormat);

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    /// No reason to instantiate.  Static methods only
    TypeConverter();
};

/* ============================ INLINE METHODS ============================ */
#endif  // _TypeConverter_h_

