//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
//////////////////////////////////////////////////////////////////////////////
// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandProcessor.h>
#include <tapi/sipXtapi.h>
#include <sipXtapiDriver/CallAnswerCommand.h>

CallAnswerCommand::CallAnswerCommand()
{
}

int CallAnswerCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
	if(argc == 2)
	{
		sipxCallAnswer(atoi(argv[1]));
		printf("Call with ID: %d answered.\n", atoi(argv[1]));
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

void CallAnswerCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
    usage->append(" PARAM 1:  Call Handle\n");
}
