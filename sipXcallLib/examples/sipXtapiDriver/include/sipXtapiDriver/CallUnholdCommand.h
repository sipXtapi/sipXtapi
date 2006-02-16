//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _CallUnholdCommand_h_
#define _CallUnholdCommand_h_

// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>

class CallUnholdCommand : public Command
{
public:
	//constructor
	CallUnholdCommand() {}
	/* ============================ MANIPULATORS ============================== */

	virtual int execute(int argc, char* argv[]);

	/* ============================ ACCESSORS ================================= */
	virtual void getUsage(const char* commandName, UtlString* usage) const;
};
#endif //_CallUnholdCommand_h