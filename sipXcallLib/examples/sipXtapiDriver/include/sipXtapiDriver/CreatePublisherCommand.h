//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _CreatePublisherCommand_h_
#define _CreatePublisherCommand_h_

// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>

class CreatePublisherCommand : public Command
{
public:
	//Constructor
	CreatePublisherCommand(const SIPX_INST hInst,
						   SIPX_PUB* phPub);
	/* ============================ MANIPULATORS ============================== */

	virtual int execute(int argc, char* argv[]);

	/* ============================ ACCESSORS ================================= */

	virtual void getUsage(const char* commandName, UtlString* usage) const;
private:
	SIPX_INST hInstance;
	SIPX_PUB* hPub;
};
#endif //_CreatePublisherCommand_h_

