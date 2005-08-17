//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
//////////////////////////////////////////////////////////////////////////////
// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>
#include <tapi/sipXtapi.h>
#include <sipXtapiDriver/CallConnectCommand.h>

CallConnectCommand::CallConnectCommand()
{

}

int CallConnectCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 3) 
	{
		if(sipxCallConnect(atoi(argv[1]), argv[2]) == SIPX_RESULT_SUCCESS) 
		{
			printf("Call connected with id: %s\n", argv[1]);
		}
		else
		{
			printf("Unable to connect to %s\n", argv[2]);
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

void CallConnectCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" <Call Handle> <SIP url of the target party>\n");
}
