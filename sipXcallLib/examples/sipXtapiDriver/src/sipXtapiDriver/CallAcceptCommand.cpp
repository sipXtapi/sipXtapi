//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
//////////////////////////////////////////////////////////////////////////////
// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>
#include <tapi/sipXtapi.h>
#include <sipXtapiDriver/CallAcceptCommand.h>

CallAcceptCommand::CallAcceptCommand()
{
}

int CallAcceptCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 2) 
	{
	    SIPX_CALL hCall = atoi(argv[1]);
		if(sipxCallAccept(hCall) == SIPX_RESULT_SUCCESS)
		{
			printf("Call with ID %d has been accepted.\n", hCall);
		}
		else
		{
			printf("Call with ID %d failed to be accepted.\n", hCall);
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

void CallAcceptCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append("PARAM 1: Call Handle\n");
}
