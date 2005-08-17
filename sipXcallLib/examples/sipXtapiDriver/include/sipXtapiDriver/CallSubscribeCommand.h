//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _CallSubscribeCommand_h_
#define _CallSubscribeCommand_h_

// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>

class CallSubscribeCommand : public Command
{
public:
	//Constructor
	CallSubscribeCommand(SIPX_SUB* hSub);
	/* ============================ MANIPULATORS ============================== */

	virtual int execute(int argc, char* argv[]);

	/* ============================ ACCESSORS ================================= */

	virtual void getUsage(const char* commandName, UtlString* usage) const;
private:
	SIPX_SUB* subHandle;
};
#endif //_CallSubscribeCommand_h_
