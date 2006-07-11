//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
//////////////////////////////////////////////////////////////////////////////
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
