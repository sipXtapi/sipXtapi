//
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#if defined(_WIN32)
#   include <winsock2.h>
#endif
#include "os/OsSysLog.h"
#include "resiprocateLib/sipXexternalLogger.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATICS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

bool SipXExternalLogger::operator()(resip::Log::Level level,
                                    const resip::Subsystem& subsystem, 
                                    const resip::Data& appName,
                                    const char* file,
                                    int line,
                                    const resip::Data& message,
                                    const resip::Data& messageWithHeaders)
{
   OsSysLog::add(FAC_CONFERENCE, toPriority(level), "%s", messageWithHeaders.c_str());
   return false;
}
      
OsSysLogPriority SipXExternalLogger::toPriority(resip::Log::Level level)
{
   switch (level)
   {
   case resip::Log::Crit:
      return PRI_CRIT;
   case resip::Log::Err:
      return PRI_ERR;
   case resip::Log::Warning:
      return PRI_WARNING;
   case resip::Log::Info:
      return PRI_INFO;
   case resip::Log::Debug:
   case resip::Log::Stack:
   default:
      return PRI_DEBUG;
   }
}
