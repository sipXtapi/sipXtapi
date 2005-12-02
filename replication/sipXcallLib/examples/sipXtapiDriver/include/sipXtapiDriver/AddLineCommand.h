//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _AddLineCommand_h_
#define _AddLineCommand_h_

#include <sipXtapiDriver/CommandProcessor.h>

class AddLineCommand : public Command
{
public:
	//Constructor
	AddLineCommand(const SIPX_INST hInst,
                   SIPX_LINE* phLine);
	/*-------------------------------Manipulators----------------------------------*/
	virtual int execute(int argc, char* argv[]);
	/*-------------------------------Accessors-------------------------------------*/
	virtual void getUsage(const char* commandName, UtlString* usage) const;
private:
	SIPX_INST hInstance;
	SIPX_LINE* phLine2;
};
#endif //_AddLineCommand_h_
