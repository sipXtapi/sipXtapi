//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
//////////////////////////////////////////////////////////////////////////////
// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>
#include <tapi/sipXtapi.h>
#include <sipXtapiDriver/CallHoldCommand.h>

CallHoldCommand::CallHoldCommand()
{
}

int CallHoldCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 3)
	{
	    SIPX_CALL hCall = atoi(argv[1]);
		if(atoi(argv[2]) == 1)
		{
			sipxCallHold(hCall);
			printf("Call with ID: %d is on hold.\n", hCall);
		}
		else if(atoi(argv[2]) == 0)
		{
			sipxCallUnhold(hCall);
			printf("Call with ID: %d is no longer on hold.\n", hCall);
		}
		else
		{
			printf("Invalid argument.\n");
			commandStatus = CommandProcessor::COMMAND_BAD_SYNTAX;
		}
	}
	else
	{
		UtlString usage;
        getUsage(argv[0], &usage);
        printf("%s", usage.data());
        commandStatus = CommandProcessor::COMMAND_BAD_SYNTAX;
	}
	return commandStatus;
}

void CallHoldCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append("\n PARAM 1: Call Handle\n  PARAM 2: (1 = put call on hold, 0 = unhold the call)   >\n");
}

