//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
//////////////////////////////////////////////////////////////////////////////
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
