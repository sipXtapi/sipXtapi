//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _AutoRedirectCommand_h_
#define _AutoRedirectCommand_h_

// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>
#include <tapi/sipXtapiEvents.h>

class AutoRedirectCommand : public Command
{
public:
	//Constructor
	AutoRedirectCommand(SIPX_INST hInst, SIPX_CALL hCall, BOOL* isDestroyed);
	/* ============================ MANIPULATORS ============================== */

	virtual int execute(int argc, char* argv[]);

	/* ============================ ACCESSORS ================================= */

	virtual void getUsage(const char* commandName, UtlString* usage) const;
	
private:
	SIPX_INST hInstance;
	SIPX_CALL callHandle;
	BOOL* destroy;
};
#endif //_AutoRedirectCommand_h_
