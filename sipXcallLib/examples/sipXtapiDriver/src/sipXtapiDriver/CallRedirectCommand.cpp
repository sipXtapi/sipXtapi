//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
//////////////////////////////////////////////////////////////////////////////
// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>
#include <tapi/sipXtapi.h>
#include <sipXtapiDriver/CallRedirectCommand.h>

CallRedirectCommand::CallRedirectCommand(SIPX_CALL* hCall)
{
	privCall = hCall;
}

int CallRedirectCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 2) 
	{
		sipxCallRedirect(*privCall, argv[1]);
		printf("Call with ID: %d has been redirected to %s.\n",
				*privCall, argv[1]);
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

void CallRedirectCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" <SIP url to forward/redirect the call to>\n");
}
