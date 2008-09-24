//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _EnableRportCommand_h_
#define _EnableRportCommand_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>

class EnableRportCommand : public Command 
{
public:
	//constructor
	EnableRportCommand(SIPX_INST hInst);
	/*-------------------------------Manipulators----------------------------------*/
	virtual int execute(int argc, char* argv[]);
	/* ============================ ACCESSORS ================================= */
	virtual void getUsage(const char* commandName, UtlString* usage) const;

private:
	SIPX_INST hInst2;
};
#endif //_EnableRportCommand_h_
