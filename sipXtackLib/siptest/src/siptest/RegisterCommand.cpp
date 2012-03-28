//
// Copyright (C) 2006-2012 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#ifdef TEST
#include <assert.h>
#include "utl/UtlMemCheck.h"
#endif //TEST
#include <stdio.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES
#include "siptest/RegisterCommand.h"
#include "siptest/CommandProcessor.h"
#include "net/SipLine.h"
#include "net/NetMd5Codec.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
static const
char UsageMsg[] =
"   <identity-sip-url> [start | stop]      (start or stop registering this identitiy)\n";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
RegisterCommand::RegisterCommand(SipLineMgr* lineMgr) :
  mLineMgr( *lineMgr )
{
}


// Destructor
RegisterCommand::~RegisterCommand()
{
}

/* ============================ MANIPULATORS ============================== */

int RegisterCommand::execute(int argc, char* argv[])
{
    int commandStatus = CommandProcessor::COMMAND_FAILED;
    Url      identity;

    UtlBoolean startRegistering = TRUE;
    identity = argv[1];

    if(argc == 2)
    {
        startRegistering = TRUE;
        commandStatus = CommandProcessor::COMMAND_SUCCESS;
    }
    if(argc == 3)
    {
        UtlString stopStart(argv[2]);
        if(stopStart.compareTo("stop", UtlString::ignoreCase) == 0)
        {
            startRegistering = FALSE;
            commandStatus = CommandProcessor::COMMAND_SUCCESS;
        }
        else if(stopStart.compareTo("start", UtlString::ignoreCase) == 0)
        {
            startRegistering = TRUE;
            commandStatus = CommandProcessor::COMMAND_SUCCESS;
        }
        else
        {
            printf( "Illegal token: \"%s\" used as second argument\n", argv[2]);
        }
    }

    if(commandStatus == CommandProcessor::COMMAND_SUCCESS)
    {
        printf( "%s identity '%s'\n", 
            startRegistering ? "Registering" : "Unregistering", argv[1] );

        if(mLineMgr.enableLine(identity))
        {
        }
        else
        {
            printf("Line: \"%s\" not found, add line via auth command\n", argv[1]);
            commandStatus = CommandProcessor::COMMAND_FAILED;
        }
    }

  return(commandStatus);
}

/* ============================ ACCESSORS ================================= */

void RegisterCommand::getUsage(const char* commandName, UtlString* usage) const
{
  Command::getUsage(commandName, usage);
  usage->append( UsageMsg );
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

