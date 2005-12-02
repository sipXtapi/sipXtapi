//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
//////////////////////////////////////////////////////////////////////////////
// APPLICATION INCLUDES>
#include <sipXtapiDriver/CommandProcessor.h>
#include <tapi/sipXtapi.h>
#include <sipXtapiDriver/EnableStunCommand.h>

//Constructor
EnableStunCommand::EnableStunCommand(const SIPX_INST hInst)
{
	hInstance = hInst;
}

int EnableStunCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;

	if(argc == 3) 
	{
		if(sipxConfigEnableStun(hInstance, argv[1], atoi(argv[2])) == SIPX_RESULT_SUCCESS)
		{
			printf("Stun enabled.\n");
		}
		else
		{
			printf("Failed to enable Stun.");
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

void EnableStunCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" <Stun server> <how often to refresh the stun binding in secs>\n");
}
