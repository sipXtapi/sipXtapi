//  
// Copyright (C) 2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2008 Mutualink, Inc. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#include <os/OsIntTypes.h>
#include <iostream>
#include <string>
#include <vector>
#include <signal.h>
#include <conio.h>
using namespace std;

#include <rtcp/RtcpConfig.h>
#include <mp/MpMediaTask.h>
#include <mp/MpFlowGraphBase.h>
#include <CpTopologyGraphInterface.h>
#include <mi/CpMediaInterface.h>
#include <mi/CpMediaInterfaceFactoryFactory.h>
#include <net/QoS.h>
#include <sdp/SdpCodec.h>
#include <rtcp/SourceDescription.h>
#include <rtcp/BaseClass.h>

#undef Release

#include <os/OsSysLog.h>
#include <os/OsConfigDb.h>
#include <os/OsTask.h>
#include <os/OsProcess.h>
#include <utl/UtlSList.h>
#include <utl/UtlVoidPtr.h>
#include <utl/UtlInit.h>

#define BUFFER_SIZE 256
#define ARG_NUM 10
namespace
{
    /// Possible action types
    typedef enum
    {
        ACTION_RX, ACTION_TX, ACTION_RXTX,
        ACTION_RXSTOP, ACTION_TXSTOP, ACTION_RXSTART, ACTION_TXSTART,
        ACTION_PORT_LOCAL, ACTION_PORT_REMOTE, ACTION_PLAY, ACTION_DELAY,
        ACTION_REPEAT, ACTION_STATS, ACTION_NOSTATS,
        ACTION_FOCUS, ACTION_NOFOCUS,
        ACTION_ADD_IF, ACTION_SET_IF,
        ACTION_EXIT, 
        NUM_ACTION_TYPES
    } ActionType;

    /// Table of actions for automated cmd line parsing
    typedef struct _parseentry
    {
        string          name;
        ActionType      type;
        int             minArgs;
    } ParseEntry;

    const ParseEntry parseTable[NUM_ACTION_TYPES] =
    {
        { "rx",       ACTION_RX,          1  },
        { "tx",       ACTION_TX,          1  },
        { "rxtx",     ACTION_RXTX,        1  },
        { "rxstop",   ACTION_RXSTOP,      0  },
        { "txstop",   ACTION_TXSTOP,      0  },
        { "rxstart",  ACTION_RXSTART,     0  },
        { "txstart",  ACTION_TXSTART,     0  },
        { "lport",    ACTION_PORT_LOCAL,  1  },
        { "rport",    ACTION_PORT_REMOTE, 1  },
        { "play",     ACTION_PLAY,        1  },
        { "delay",    ACTION_DELAY,       1  },
        { "repeat",   ACTION_REPEAT,      2  },
        { "stats",    ACTION_STATS,       0  },
        { "nostats",  ACTION_NOSTATS,     0  },
        { "focus",    ACTION_FOCUS,       0  },
        { "nofocus",  ACTION_NOFOCUS,     0  },
        { "add_if",   ACTION_ADD_IF,      0  },
        { "set_if",   ACTION_SET_IF,      1  },
        { "exit",     ACTION_EXIT,        0  },
    };

    typedef struct _action
    {
        string          name;
        ActionType      type;
        vector<string>  args;
    } Action;

    vector<Action> actions;
    const char* pProgName = 0;
    const char* pFirstRtpAddr = 0;
    //bool connectionExists = false;
    bool initialFocus = true;
    bool txRtp = false, rxRtp = false;
    bool txRtpEnabled = false;
    CpMediaInterfaceFactory* pFactory;
    CpMediaInterface* pIf = 0;
    UtlSList pInterfacesList;
    int graphCount = 0;
    const int DEF_RTP_PORT = 6000;
    int rtpPort_local = DEF_RTP_PORT;
    int rtpPort_remote = DEF_RTP_PORT;
    int lastConnId = -1;
    int numLastActions=0;
    bool showStats = false;
    bool exitApp = false;

    int numCodecs;
    SdpCodec** pCodecArray;
}

void showStatus();
int parseCommands (int argsNum, const char* argsArray[]);
void createMediaTaskandFlowGraph();
OsStatus createConnection(const char* pRtpAddr);
OsStatus deleteConnection(int connId);
int executeActions();
int executeAction(Action* pAction);
int consoleInput(const char* argsArray[ARG_NUM]);
int parseStr(const char* argsArray[ARG_NUM], char *str);
int createFlowGraph();
void selectFlowGraph(int index);

//******************************************************************************
void showUsage()
{
    cout << "Performs various media stream actions" << endl << endl;
    cout << "Usage: " << pProgName << " <action> [<action> ...]" << endl;
    cout << "Actions: tx <rtp_addr>    = Transmits from Mic to RTP" << endl;
    cout << "         rx <rtp_addr>    = Receives from RTP to speaker" << endl;
    cout << "         rxtx <rtp_addr>  = Enable both rx and tx" << endl;
    cout << "         rxstop           = Stops Rx for the last RTP action" << endl;
    cout << "         txstop           = Stops Tx for the last RTP action" << endl;
    cout << "         rxstart          = Starts Rx for the last RTP action" << endl;
    cout << "         txstart          = Starts Tx for the last RTP action" << endl;
    cout << "         lport <local_rtp_port>  = Sets port for next RTP action"
         << " (default=" << DEF_RTP_PORT << ")" << endl;
    cout << "         rport <remote_rtp_port>  = Sets port for next RTP action"
         << " (default=" << DEF_RTP_PORT << ")" << endl;
    cout << "         play <file> [loop] = Plays audio file to RTP if tx is" << endl
         << "                              active, else local speaker" << endl;
    cout << "         delay <msecs>    = Delay a number of milliseconds" << endl;
    cout << "         repeat <num> <count>  = Repeats last <num> actions <count> times" << endl;
    cout << "         stats            = Display full stats periodically" << endl;
    cout << "         nostats          = Don't display full stats" << endl;
    cout << "         exit             = Exit the application" << endl;
    cout << "         focus            = Give focus to this IF (default)" << endl;
    cout << "         nofocus          = Don't give focus to this IF" << endl;
    cout << "         add_if           = Create new IF" << endl;
    cout << "         set_if           = Set default IF for consecutive commands" << endl;
    cout << "Example: " << pProgName << " tx 224.10.11.12" << endl;
}

//******************************************************************************
static void ctrlCHandler(int signo)
{
   printf("\nShutting down...\n");
   exitApp = true;
}

//******************************************************************************
int main(int argc, const char* argv[])
{
    pProgName = argv[0];

    // Install Ctrl-C handler.
    if ( signal( SIGINT, ctrlCHandler ) == SIG_ERR )
    {
       printf("Couldn't install signal handler for SIGINT\n");
       exit(1);
    }
    if ( signal( SIGTERM, ctrlCHandler ) == SIG_ERR )
    {
       printf("Couldn't install signal handler for SIGTERM\n");
       exit(1);
    }

    parseCommands (argc, argv);

    createMediaTaskandFlowGraph();

    executeActions();

    showUsage();

    while(!exitApp)
    {   
        const char* argsArray[ARG_NUM];
        int argsNum;
        argsNum=consoleInput(argsArray);
        parseCommands (argsNum, argsArray);
        executeActions();
    }

    // Delete all connections
    for (int id = 1; id <= lastConnId; id++)
        deleteConnection(id);


    pInterfacesList.destroyAll();

    // And finally clean up media factory.
    sipxDestroyMediaFactoryFactory();

    OsSysLog::shutdown();

    return 0;
}

//******************************************************************************
int parseCommands (int argsNum, const char* argsArray[])
{
    numLastActions=0;
// Walk command line args to create a series of actions
    for (int argIdx = 1; argIdx < argsNum; argIdx++)
    {
        int argsLeft = argsNum - argIdx - 1;
        bool foundAction = false;
        bool notEnoughArgs = false;
        string actionName(argsArray[argIdx]);
        Action action;
        action.name = actionName;

        // Find the requested action in our table
        for (int tableIdx = 0; tableIdx < NUM_ACTION_TYPES; tableIdx++)
        {
            const ParseEntry* pEntry = &parseTable[tableIdx];
            if (pEntry->name == actionName)
            {
                foundAction = true;
                action.type = pEntry->type;
                if (argsLeft < pEntry->minArgs)
                {
                    notEnoughArgs = true;
                    break;
                }

                // Copy mandatory args from cmdline into action's arg list
                for (int args = pEntry->minArgs; args > 0; args--)
                {
                    argIdx++;
                    action.args.push_back(argsArray[argIdx]);
                    argsLeft--;
                }
                break;
            }
        }

        if (!foundAction)
        {
            cout << "ERROR: Unrecognized action \""
                 << actionName << "\"" << endl << endl;
            showUsage();
            return -1;
        }

        if (notEnoughArgs)
        {
            cout << "ERROR: Need more arguments for action \""
                 << actionName << "\"" << endl << endl;
            showUsage();
            return -1;
        }

        // Perform any additional processing for this action (if necessary)
        switch (action.type)
        {
        case ACTION_RX:
            rxRtp = true;
            if (!pFirstRtpAddr)
                pFirstRtpAddr = argsArray[argIdx];
            break;

        case ACTION_TX:
            txRtp = true;
            if (!pFirstRtpAddr)
                pFirstRtpAddr = argsArray[argIdx];
            break;

        case ACTION_RXTX:
            rxRtp = txRtp = true;
            if (!pFirstRtpAddr)
                pFirstRtpAddr = argsArray[argIdx];
            break;

        case ACTION_PLAY:
            if (argsLeft && strcmp(argsArray[argIdx+1], "loop") == 0)
            {
                argIdx++;
                action.args.push_back(argsArray[argIdx]);
            }
            break;

        case ACTION_FOCUS:
        case ACTION_NOFOCUS:
            // If rx/tx/rxtx hasn't been given yet, this must be the initial one
            if (!pFirstRtpAddr)
                initialFocus = (action.type == ACTION_FOCUS) ? true : false;
            break;

        case ACTION_REPEAT:
            {
                // Make sure there's enough previous actions to repeat
                unsigned num = atoi(action.args[0].c_str());
                if (num > actions.size())
                {
                    cout << "ERROR: There aren't " << num
                         << " actions to repeat" << endl;
                    return -1;
                }
            }
            break;
        case ACTION_ADD_IF:
           // No actions right there
           break;
        case ACTION_SET_IF:
           // No actions right there
           break;

        // OK to ignore all others
        default:
            break;
        } // switch

        actions.push_back(action);
        numLastActions++;

    } // cmd line arg loop

/*
    vector<Action>::iterator act;
    for (act = actions.begin(); act != actions.end(); act++)
    {
        cout << "Action: Type=" << act->type
             << ", Args = ";
        vector<string>::iterator arg;
        for (arg = act->args.begin(); arg != act->args.end(); arg++)
            cout << "\"" << *arg << "\" ";
        cout << endl;
    }
*/

    return 0;
}

int createFlowGraph()
{
   // Creates a CpPhoneMediaInterface (vals from CpPhoneMediaInterface defaults)
   CpMediaInterface* pNewIf = pFactory->createMediaInterface(
      0,      // publicAddress
      0,      // localAddress
      0,      // numCodecs
      NULL,   // sdpCodecArray
      "",     // locale
      QOS_LAYER3_LOW_DELAY_IP_TOS,      // expeditedIpTos
      NULL,   // szStunServer
      0,      // iStunPort
      28,     // iStunKeepAlivePeriodSecs
      NULL,   // szTurnServer
      0,      // iTurnPort
      NULL,   // szTurnUsername
      NULL,   // szTurnPassword
      0,      // stunOptions
      28,     // iStunKeepAlivePeriodSecs
      FALSE); // bEnableICE

   if (pNewIf != NULL)
   {
      pInterfacesList.insert(pNewIf);
      cout << "Created CpMediaInterface (ID:" << graphCount << ")" << endl;
      
      return graphCount++;
   }
   else
   {
      cout << "Error has been occurred during creation of CpMediaInterface!";
      return -1;
   }
}

void selectFlowGraph(int index)
{
   CpMediaInterface* pSelIf = (CpMediaInterface*)pInterfacesList.at(index);
   if (pSelIf != NULL)
   {
      cout << "Graph " << index << " has been selected as default" << endl;
      pIf = pSelIf;
   }
   else
   {
      cout << "Graph " << index << " doesn't exit!" << endl;
   }
}

//******************************************************************************
void createMediaTaskandFlowGraph()
{
    // Create foundation Media Task & Flow Graph

    OsSysLog::initialize(0, "mstream");
    OsSysLog::setLoggingPriority(PRI_DEBUG);
    OsSysLog::enableConsoleOutput(true);

    OsConfigDb cfgDb;
    if (cfgDb.loadFromFile("mstream.cfg") == OS_SUCCESS)
        cout << "(Read " << cfgDb.numEntries()
             << " config entries from mstream.cfg)" << endl;

    // Creates a sipXmediaFactoryImpl
    pFactory = sipXmediaFactoryFactory(&cfgDb);

    MpMediaTask* pMediaTask = MpMediaTask::getMediaTask();
    //sleep(0);

    char sdesName[64];
    sprintf(sdesName, "mstream-%d", OsProcess::getCurrentPID());
    cout << "Setting SDES NAME to " << sdesName << endl;

#ifdef INCLUDE_RTCP

    CSourceDescription* pSdes = CSourceDescription::GetLocalSDES();
    pSdes->SetAllComponents(
        /* NAME        */   (unsigned char*)sdesName,
        /* EMAIL       */   (unsigned char*)"",
        /* PHONE       */   (unsigned char*)"",
        /* APPLICATION */   (unsigned char*)"",
        /* LOCATION    */   (unsigned char*)"",
        /* NOTES       */   (unsigned char*)"",
        /* PRIVATE     */   (unsigned char*)"");

#endif

    selectFlowGraph(createFlowGraph());

    if (initialFocus)
    {
        pIf->giveFocus();
        cout << "Gave focus to this interface" << endl;
    }

    {
        SdpCodecList codecList;
        //UtlString codecs = "PCMU PCMA TELEPHONE-EVENT";
        UtlString codecNames = "PCMU";
        codecList.addCodecs(codecNames);

        codecList.getCodecs(numCodecs, pCodecArray);
    } 
}

//******************************************************************************
int executeActions()
{
    // Loop for each action

    vector<Action>::iterator act;
    for (act = actions.begin(); act != actions.end() && !exitApp; act++)
    {
        if ((actions.size() - numLastActions)==0)
        {
            if (act->type == ACTION_REPEAT)
            {
                int num = atoi(act->args[0].c_str());
                int count = atoi(act->args[1].c_str());

                // Point an iterator to the FIRST action to repeat
                vector<Action>::iterator startAct = act;
                for (int decStart = 0; decStart < num; decStart++)
                    startAct--;

                // Point an iterator to the LAST action to repeat
                vector<Action>::iterator endAct = act;
                endAct--;

                // Loop <count> times for each repeat cycle
                for (int repeatCycle = 0; repeatCycle < count; repeatCycle++)
                {
                    vector<Action>::iterator repeatAct = startAct;
                    int actNum = 1;
                    do
                    {
                        cout << "---(Repeat cycle " << repeatCycle+1 << " of " 
                             << count << ": action " << actNum << " of " << num 
                             << ")---"<< endl;
                        executeAction(&*repeatAct);
                        actNum++;
                    } while (repeatAct++ != endAct);
                }
            }
            // Not a repeat action - just go execute it
            else
            {
                executeAction(&*act);
            }
        }
        else
        {
            numLastActions++;
        }
    } // action loop
    return 0;
}

//******************************************************************************
int executeAction(Action* pAction)
{
    OsStatus ret;

    cout << "---------EXECUTING \"" << pAction->name;
    vector<string>::iterator arg;
    for (arg = pAction->args.begin(); arg != pAction->args.end(); arg++)
        cout << " " << *arg;
    cout << "\"------------------------------" << endl;

    switch (pAction->type)
    {
    case ACTION_RXTX:
    case ACTION_RX:
        if (createConnection(pAction->args[0].c_str()) != OS_SUCCESS)
            break;

        ret = pIf->startRtpReceive(lastConnId, numCodecs, pCodecArray);
        if (ret != OS_SUCCESS)
            cerr << "***ERROR starting RTP Receive: ret=" << ret << endl;
        else
            cout << "Started RTP Receive OK " << endl;

        // Let RXTX fall through
        if (pAction->type == ACTION_RX)
            break;

    case ACTION_TX:
        txRtpEnabled = true;
        // Fall-thru case: connection already exists
        if (pAction->type != ACTION_RXTX)
        {
            if (createConnection(pAction->args[0].c_str()) != OS_SUCCESS)
                break;
        }
        ret = pIf->startRtpSend(lastConnId, numCodecs, pCodecArray);
        if (ret != OS_SUCCESS)
            cerr << "***ERROR starting RTP Send: ret=" << ret << endl;
        else
            cout << "Started RTP Send OK " << endl;
        break;

    case ACTION_RXSTOP:
        ret = pIf->stopRtpReceive(lastConnId);
        if (ret != OS_SUCCESS)
            cerr << "***ERROR stopping RTP Receive: ret=" << ret << endl;
        else
            cout << "Stopped RTP Receive OK " << endl;
        break;

    case ACTION_TXSTOP:
        ret = pIf->stopRtpSend(lastConnId);
        if (ret != OS_SUCCESS)
            cerr << "***ERROR stopping RTP Send: ret=" << ret << endl;
        else
            cout << "Stopped RTP Send OK " << endl;
        break;

    case ACTION_RXSTART:
        ret = pIf->startRtpReceive(lastConnId, numCodecs, pCodecArray);
        if (ret != OS_SUCCESS)
            cerr << "***ERROR starting RTP Receive: ret=" << ret << endl;
        else
            cout << "Started RTP Receive OK " << endl;
        break;

    case ACTION_TXSTART:
        ret = pIf->startRtpSend(lastConnId, numCodecs, pCodecArray);
        if (ret != OS_SUCCESS)
            cerr << "***ERROR starting RTP Send: ret=" << ret << endl;
        else
            cout << "Started RTP Send OK " << endl;
        break;

    case ACTION_PORT_LOCAL:
        rtpPort_local = atoi(pAction->args[0].c_str());
        cout << "Next RTP local port set to " << rtpPort_local << "." << endl;
        break;

    case ACTION_PORT_REMOTE:
        rtpPort_remote = atoi(pAction->args[0].c_str());
        cout << "Next RTP remote port set to " << rtpPort_remote << "." << endl;
        break;

    case ACTION_PLAY:
    {
        bool loop = false;
        if (pAction->args.size() > 1 && pAction->args[1] == "loop")
            loop = true;

        ret = pIf->playAudio(pAction->args[0].c_str(), loop, !txRtpEnabled, txRtpEnabled);
        if (ret != OS_SUCCESS)
            cerr << "***ERROR playing file \"" << pAction->args[0]
                 << "\": ret=" << ret << endl;
        else
            cout << "Started playing file \"" << pAction->args[0] << "\" OK:"
                 << " loop=" << loop
                 << " local=" << !txRtpEnabled
                 << " remote=" << txRtpEnabled
                 << endl;
        break;
    }

    case ACTION_FOCUS:
        cout << "Giving focus ..." << endl;
        pIf->giveFocus();
        break;

    case ACTION_NOFOCUS:
        cout << "Losing focus ..." << endl;
        pIf->defocus();
        break;

    case ACTION_DELAY:
    {
        int msecs = atoi(pAction->args[0].c_str());
        cout << "Delaying " << msecs << "msecs ..." << endl;
        OsTask::delay(msecs);
        break;
    }

    case ACTION_STATS:
        showStats = true;
        break;

    case ACTION_NOSTATS:
        showStats = false;
        break;

    case ACTION_EXIT:
        exitApp = true;
        break;

    case ACTION_ADD_IF:
       createFlowGraph();
       break;
    case ACTION_SET_IF:
       {
          int num = atoi(pAction->args[0].c_str());
          selectFlowGraph(num);
       }
       break;

    default:
        cerr << "***ERROR: Unrecognized action " << pAction->type << " in switch" << endl;

    } // switch action type

    OsTask::delay(1000);
    cout << "---------AFTER \"" << pAction->name
         << "\"------------------------------" << endl;
    //showStatus();

    return 0;
}

//******************************************************************************
OsStatus createConnection(const char* pRtpAddr)
{
    // Don't create it twice
    //if (connectionExists)
    //    return OS_SUCCESS;

    OsStatus ret;
    UtlBoolean isMcast = OsSocket::isMcastAddr(pRtpAddr);
    ret = pIf->createConnection(lastConnId, isMcast?pRtpAddr:NULL, 
                                                    rtpPort_local);
    if (ret != OS_SUCCESS)
    {
        cerr << "***ERROR creating connection to " << pRtpAddr
             << ": ret=" << ret << endl;
        return ret;
    }
    cout << "Created ConnectionId " << lastConnId << " to " << pRtpAddr << endl;

    //sleep(0);
    //showStatus();

    // Enable DTX
    ((CpTopologyGraphInterface*)pIf)->enableDtx(lastConnId, TRUE);

    ret = pIf->setConnectionDestination(lastConnId, pRtpAddr,
                                        rtpPort_remote, rtpPort_remote+1, 0, 0);
    if (ret != OS_SUCCESS)
    {
        cerr << "***ERROR setting connection destination: ret=" << ret << endl;
        return ret;
    }
    cout << "Set connection destination OK " << endl;

    //sleep(0);
    //showStatus();

    //connectionExists = true;


    UtlString rtpHostAddress;
    int rtpAudioPort, rtcpAudioPort, rtpVideoPort, rtcpVideoPort;
    int videoBandwidth;
    int videoFramerate;
    SdpCodecList codecFactory;
    SdpSrtpParameters srtpParams;

    pIf->getCapabilities(lastConnId, rtpHostAddress,
                         rtpAudioPort, rtcpAudioPort, rtpVideoPort,  rtcpVideoPort,
                         codecFactory, srtpParams, 0, videoBandwidth, videoFramerate);
    cout << ">>>> RTP Connection Info: Host=\"" << rtpHostAddress << "\""
         << " RtpPort=" << rtpAudioPort
         << " RtcpPort=" << rtcpAudioPort
         << endl;

    return ret;
}

//******************************************************************************
OsStatus deleteConnection(int connId)
{
    OsStatus ret;
    ret = pIf->deleteConnection(connId);
    if (ret != OS_SUCCESS)
    {
        cerr << "***ERROR deleting connection ID "<<connId<<": ret="<<ret<<endl;
        return ret;
    }
    cout << "Deleted connection ID "<<connId<<" OK " << endl;
    return ret;
}

//******************************************************************************
void showStatus()
{
    MpMediaTask* pMediaTask = MpMediaTask::getMediaTask();

    pMediaTask->mediaInfo();

    const int MAX_GRAPHS = 16;
    int numGraphs;
    MpFlowGraphBase* pGraphs[MAX_GRAPHS];

    OsStatus ret;
    ret = pMediaTask->getManagedFlowGraphs(pGraphs, MAX_GRAPHS, numGraphs);
    if (ret != OS_SUCCESS)
    {
        cerr << "***ERROR: Couldn't retrieve flow graph pointers!" << endl;
        return;
    }

    //cout << "Retrieved " << numGraphs << " flow graph pointers:" << endl;
    for (int i = 0; i < numGraphs; i++)
    {
        MpFlowGraphBase::flowGraphInfo(pGraphs[i]);
    }
    cout << endl;

    return;
}
//******************************************************************************
int consoleInput(const char* argsArray[ARG_NUM])
{
    char input;
    int argsNum;
    char buff[BUFFER_SIZE];
    int f=0, buffpos=-1;
    while(f!=1)
    {
        OsTask::delay(50);
        if (kbhit()!=0)
        {
            input=getch();
            if (input==13)
               f=1;
            else if(input==8)
            {
                if(buffpos>-1)
                {				
                    cout<<input<<' '<<input;
                    buffpos--;
                }
            }
            else 
            {
                buffpos++;
                buff[buffpos]=input;
                cout<<input;
            }
        }
    }
    cout<<endl;
    buff[buffpos+1]='\0';   
    argsNum=parseStr(argsArray,buff);
    return argsNum;
}
int parseStr(const char* argsArray[ARG_NUM], char *str)
{
    char *pattern="[^\\s]*"; 
    const char *errorMsg;
    int charNumber;
    int pairsNumber;
    int resultVector[2];
    int vectorSize=2;
    int shift=0;
    int argsNum=1;
    pcre *f=pcre_compile(pattern,PCRE_MULTILINE,&errorMsg,&charNumber,NULL);
    while((pairsNumber=pcre_exec(f,NULL,str,strlen(str),shift,PCRE_NOTEMPTY,
                                                resultVector,vectorSize))>=0)
    {
        pcre_get_substring(str,resultVector,pairsNumber,0,&argsArray[argsNum]);
        shift=resultVector[1]+1;
        argsNum++;
    }
    return argsNum;
}

