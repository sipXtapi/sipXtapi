//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _ConferenceJoinCommand_h_
#define _ConferenceJoinCommand_h_

// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>

class ConferenceJoinCommand : public Command
{
public:
	//Constructor
	ConferenceJoinCommand();
	/* ============================ MANIPULATORS ============================== */

	virtual int execute(int argc, char* argv[]);

	/* ============================ ACCESSORS ================================= */

	virtual void getUsage(const char* commandName, UtlString* usage) const;
private:
	SIPX_CONF hConference;
	SIPX_CALL callHandle;
};
#endif //_ConferenceJoinCommand_h_
