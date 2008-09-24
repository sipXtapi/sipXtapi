//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _CallPlayFileCommand_h_
#define _CallPlayFileCommand_h_

// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>

class CallPlayFileCommand : public Command
{
public:
	//Constructor
	CallPlayFileCommand() { };
	/* ============================ MANIPULATORS ============================== */

	virtual int execute(int argc, char* argv[]);

	/* ============================ ACCESSORS ================================= */

	virtual void getUsage(const char* commandName, UtlString* usage) const;
};
#endif //_CallPlayFileCommand_h_
