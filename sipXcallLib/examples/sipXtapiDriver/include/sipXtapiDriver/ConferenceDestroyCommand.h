//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _ConferenceDestroyCommand_h_
#define _ConferenceDestroyCommand_h_

// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>

class ConferenceDestroyCommand : public Command
{
public:
	//Constructor
	ConferenceDestroyCommand();
	/* ============================ MANIPULATORS ============================== */

	virtual int execute(int argc, char* argv[]);

	/* ============================ ACCESSORS ================================= */

	virtual void getUsage(const char* commandName, UtlString* usage) const;
};
#endif //_ConferenceDestroyCommand_h_
