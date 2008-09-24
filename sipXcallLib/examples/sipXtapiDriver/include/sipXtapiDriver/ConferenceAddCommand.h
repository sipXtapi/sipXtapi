//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _ConferenceAddCommand_h_
#define _ConferenceAddCommand_h_

// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>

class ConferenceAddCommand : public Command
{
public:
	//Constructor
	ConferenceAddCommand(SIPX_CALL* hCall);
	/* ============================ MANIPULATORS ============================== */

	virtual int execute(int argc, char* argv[]);

	/* ============================ ACCESSORS ================================= */

	virtual void getUsage(const char* commandName, UtlString* usage) const;
private:
	SIPX_CALL* callHandle;
};
#endif //_ConferenceAddCommand_h_
