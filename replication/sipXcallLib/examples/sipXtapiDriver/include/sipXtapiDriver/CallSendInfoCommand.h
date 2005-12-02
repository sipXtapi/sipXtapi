//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _CallSendInfoCommand_h_
#define _CallSendInfoCommand_h_

// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>

class CallSendInfoCommand : public Command
{
public:
	//Constructors
	CallSendInfoCommand(SIPX_INFO* phInfo);
	/* ============================ MANIPULATORS ============================== */

	virtual int execute(int argc, char* argv[]);

	/* ============================ ACCESSORS ================================= */

	virtual void getUsage(const char* commandName, UtlString* usage) const;
private:
	SIPX_INFO* hInfo;
};
#endif //_CallSendInfoCommand_h_
