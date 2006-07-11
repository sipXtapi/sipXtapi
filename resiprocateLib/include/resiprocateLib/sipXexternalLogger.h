// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _SIPXEXTERNALLOGGER_H_
#define _SIPXEXTERNALLOGGER_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "rutil/Log.hxx"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Adapter from resiprocate logging to sipX OsSysLog
class SipXExternalLogger : public resip::ExternalLogger
{
  public:
   /// Implement the resiprocate logger interface into sipX OsSysLog
   virtual bool operator()(resip::Log::Level level,
                           const resip::Subsystem& subsystem, 
                           const resip::Data& appName,
                           const char* file,
                           int line,
                           const resip::Data& message,
                           const resip::Data& messageWithHeaders);

   /// Map resiprocate log level to sipX OsSysLogPriority
   static OsSysLogPriority toPriority(resip::Log::Level level);
   
};

#endif // _SIPXEXTERNALLOGGER_H_
