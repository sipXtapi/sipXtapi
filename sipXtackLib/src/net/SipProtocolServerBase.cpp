// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <net/SipProtocolServerBase.h>
#include <net/SipUserAgent.h>
#include <os/OsDateTime.h>
#include <os/OsEvent.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
//#define TEST_PRINT
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipProtocolServerBase::SipProtocolServerBase(SipUserAgent* userAgent,
                                             const char* protocolString,
                                             const char* taskName) :
     OsTask(taskName),
     mClientLock(OsMutex::Q_FIFO)
{
   mSipUserAgent = userAgent;
   mProtocolString = protocolString;
   mDefaultPort = SIP_PORT;
}

// Copy constructor
SipProtocolServerBase::SipProtocolServerBase(const SipProtocolServerBase& rSipProtocolServerBase) :
    mClientLock(OsMutex::Q_FIFO)
{
}

// Destructor
SipProtocolServerBase::~SipProtocolServerBase()
{
    int iteratorHandle = mClientList.getIteratorHandle();
    SipClient* client = NULL;;
    while ((client = (SipClient*)mClientList.next(iteratorHandle)))
    {
        mClientList.remove(iteratorHandle);
        delete client;
    }
    mClientList.releaseIteratorHandle(iteratorHandle);
}

/* ============================ MANIPULATORS ============================== */



UtlBoolean SipProtocolServerBase::send(SipMessage* message,
                            const char* hostAddress,
                            int hostPort)
{
    UtlBoolean sendOk = FALSE;

    SipClient* client = createClient(hostAddress, hostPort);
    if(client)
    {
    	int isBusy = client->isInUseForWrite();
    	UtlString clientNames;

    	client->getClientNames(clientNames);
    	OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServerBase::send %p isInUseForWrite %d, client info\n %s",
                mProtocolString.data(), client, isBusy, clientNames.data());

        sendOk = client->sendTo(*message, hostAddress, hostPort);        
        if(!sendOk)
        {
            OsTask* pCallingTask = OsTask::getCurrentTask();
            int callingTaskId = -1;
            int clientTaskId = -1;

            if ( pCallingTask )
               pCallingTask->id(callingTaskId);
            client->id(clientTaskId); 

            if (clientTaskId != callingTaskId)
            {
				// Do not need to clientLock.acquireWrite();
				// as deleteClient uses the locking list lock
				// which is all that is needed as the client is
				// already marked as busy when we called 
				// createClient above.
				deleteClient(client);
				client = NULL;				
            }
        }
    }

	if(client)
	{
	    releaseClient(client);
	}

    return(sendOk);
}

void SipProtocolServerBase::releaseClient(SipClient* client)
{
    mClientLock.acquireWrite();

    if(client &&
		clientExists(client))
    {
        if(client->isInUseForWrite())
        {
            client->markAvailbleForWrite();
        }
        else
        {
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipProtocolServerBase::releaseClient releasing %s client not locked: %p",
                mProtocolString.data(), client);
        }
    }
    else
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipProtocolServerBase::releaseClient releasing %s client not in list: %p",
                mProtocolString.data(), client);
    }

    mClientLock.releaseWrite();
}

SipClient* SipProtocolServerBase::createClient(const char* hostAddress,
                                      int hostPort)
{
    UtlString remoteHostAddr;
    UtlBoolean clientStarted = FALSE;

    mClientLock.acquireWrite();

    SipClient* client = getClient(hostAddress, hostPort);

    if(! client)
    {
#ifdef TEST_PRINT
        osPrintf("Creating %s client: %s given port: %d\n",
            mProtocolString.data(), hostAddress, hostPort);
#endif
        if(hostPort <= 0)
        {
            hostPort = mDefaultPort;
            //osPrintf("Over riding port to: %d\n", hostPort);
        }

        OsTime time;
        OsDateTime::getCurTimeSinceBoot(time);
        long beforeSecs = time.seconds();

        OsSocket* clientSocket = buildClientSocket(hostPort, hostAddress);

        OsDateTime::getCurTimeSinceBoot(time);
        long afterSecs = time.seconds();
        if(afterSecs - beforeSecs > 1)
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING, "SIP %s socket create for %s:%d took %d seconds",
                mProtocolString.data(), hostAddress, hostPort,
                (int)(afterSecs - beforeSecs));
        }

        UtlBoolean isOk = clientSocket->isOk();
        int writeWait = 3000; // mSec
        UtlBoolean isReadyToWrite = clientSocket->isReadyToWrite(writeWait);

        if(!isReadyToWrite)
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING, "SIP %s socket %s:%d not ready for writing after %d seconds",
                mProtocolString.data(), hostAddress, hostPort, (int) (writeWait/1000));
        }

        if(isOk &&
           isReadyToWrite)
        {
#ifdef TEST
            osPrintf("Socket OK, creating client\n");
#endif
            client = new SipClient(clientSocket) ;
            if (client && mSipUserAgent->getUseRport() && 
                    clientSocket->getIpProtocol() == OsSocket::UDP)
            {   
                client->setSharedSocket(TRUE) ;
            }

#ifdef TEST
            osPrintf("Created client\n");
#endif
            if(mSipUserAgent)
            {
                client->setUserAgent(mSipUserAgent);
            }

            if (clientSocket->getIpProtocol() != OsSocket::UDP) 
            {
                //osPrintf("starting client\n");
                clientStarted = client->start();
                if(!clientStarted)
                {
                    osPrintf("SIP %s client failed to start\n",
                        mProtocolString.data());
                }
            }

            OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServer::createClient client: %p %s:%d\n",
                mProtocolString.data(), client, hostAddress, hostPort);

            mClientList.push(client);
        }

        // The socket failed to be connected
        else
        {
            if(clientSocket)
            {
                if (!mSipUserAgent->getUseRport() ||
                        (clientSocket->getIpProtocol() == OsSocket::TCP))
                {
                    delete clientSocket;
                }
                clientSocket = NULL;
            }
            OsSysLog::add(FAC_SIP, PRI_WARNING, "Sip%sServer::createClient client %p Failed to create socket %s:%d\n",
                mProtocolString.data(), this, hostAddress, hostPort);
        }
    }

    int isBusy = FALSE;
    if(client)
    {
        isBusy = client->isInUseForWrite();

        if(!isBusy)
            client->markInUseForWrite();
    }

    mClientLock.releaseWrite();

    if(client && isBusy)
    {
        if(!waitForClientToWrite(client)) client = NULL;
    }

    return(client);
}

UtlBoolean SipProtocolServerBase::waitForClientToWrite(SipClient* client)
{
    UtlBoolean exists;
    UtlBoolean busy = FALSE;
    int numTries = 0;

    do
    {
        numTries++;

        mClientLock.acquireWrite();
        exists = clientExists(client);

        if(exists)
        {
            busy =  client->isInUseForWrite();
            if(!busy)
            {
                client->markInUseForWrite();
                mClientLock.releaseWrite();
//#ifdef TEST_PRINT
				if(numTries > 1)
				{
					OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServerBase::waitForClientToWrite %p locked after %d tries\n",
						mProtocolString.data(), client, numTries);
				}
//#endif
            }
            else
            {
                // We set an event to be signaled when a
                // transaction is released.
                OsEvent* waitEvent = new OsEvent();
                client->notifyWhenAvailableForWrite(*waitEvent);

                // Must unlock while we wait or there is a dead lock
                mClientLock.releaseWrite();
#ifdef TEST_PRINT
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServerBase::waitForClientToWrite %p waiting on: %p after %d tries\n",
                    mProtocolString.data(), client, waitEvent, numTries);
#endif

				// Do not block for ever
				OsTime maxWaitTime(0, 500000);

				// If the other side signalled
                if(waitEvent->wait(maxWaitTime)  == OS_SUCCESS)
				{
					// The other side is no longer referencing
					// the event.  This side must clean it up
					delete waitEvent;
					waitEvent = NULL;
				}
				// A timeout occured and the otherside did not signal yet
				else
				{
					// Signal the other side to indicate we are done
					// with the event.  If already signaled, we lost
					// a race and the other side was done first.
					if(waitEvent->signal(0) == OS_ALREADY_SIGNALED)
					{
						delete waitEvent;
						waitEvent = NULL;
					}
				}

#ifdef TEST_PRINT
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServerBase::waitForClientToWrite %p done waiting after %d tries\n",
                    mProtocolString.data(), client, numTries);
#endif
            }
        }
        else
        {
            mClientLock.releaseWrite();

//#ifdef TEST_PRINT
            OsSysLog::add(FAC_SIP, PRI_ERR, "Sip%sServerBase::waitForClientToWrite %p gone after %d tries\n",
                    mProtocolString.data(), client, numTries);
//#endif
        }
    }
    while(exists && busy);

    return(exists && !busy);
}

SipClient* SipProtocolServerBase::getClient(const char* hostAddress,
                                  int hostPort)
{
    //UtlString remoteHostAddr;
    UtlBoolean isSameHost = FALSE;
    //UtlString clientCallId;
    //UtlString clientToField;
    //UtlString clientFromField;
    //int clientPort;
    UtlString hostAddressString(hostAddress ? hostAddress : "");
    int iteratorHandle = mClientList.getIteratorHandle();
    SipClient* client = NULL;

    while ((client = (SipClient*)mClientList.next(iteratorHandle)))
    {
        // Are these the same host?

        isSameHost = client->isConnectedTo(hostAddressString, hostPort);

        if(isSameHost && client->isOk())
        {
#ifdef TEST_PRINT
            osPrintf("Using existing %s client for %s port: %d\n",
                mProtocolString.data(), hostAddress, hostPort);
#endif
            break;
        }
        else if(isSameHost)
        {
            if(!client->isOk())
            {
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "%s Client matches but is not OK\n",
                    mProtocolString.data());
            }
        }
    }
    mClientList.releaseIteratorHandle(iteratorHandle);

    return(client);
}

void SipProtocolServerBase::deleteClient(SipClient* sipClient)
{
    // Find the client in the list of clients and shut it down
    int iteratorHandle = mClientList.getIteratorHandle();
    SipClient* client = NULL;

#ifdef TEST_PRINT

    OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServer::deleteClient(%p)\n",
        mProtocolString.data(), sipClient);
#endif
    while ((client = (SipClient*)mClientList.next(iteratorHandle)))
    {
        // Remove this bad client
		// This used to be a little over zealous and delete any
		// SipClient that was not ok.  It was not checking if
		// the SipClient was busy or not so bad things could
		// happen.  This is now on the conservative side and
		// deleting only the thing it is supposed to.
        if(client == sipClient)
        {
#ifdef TEST_PRINT
            UtlString clientNames;
            client->getClientNames(clientNames);
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "Removing %s client %p names:\n%s\n",
                mProtocolString.data(), this, clientNames.data());
#endif
            mClientList.remove(iteratorHandle);

            break;
        }
    }
    mClientList.releaseIteratorHandle(iteratorHandle);

	// Delete the client outside the lock on the list as
	// it can create a deadlock.  If the client is doing
	// an operation that requires the locking list, the 
	// client gets blocked from shutting down.  We then
	// block here trying to delete the client forever.
	if(client)
	{
		delete client;
        client = NULL;
	}

#ifdef TEST_PRINT
    OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServer::deleteClient(%p) done\n",
        mProtocolString.data(), sipClient);
#endif
}

void SipProtocolServerBase::removeOldClients(long oldTime)
{
    mClientLock.acquireWrite();
    // Find the old clients in the list  and shut them down
    int iteratorHandle = mClientList.getIteratorHandle();
    SipClient* client;
    int numClients = mClientList.getCount();
    int numDelete = 0;
    int numBusy = 0;
    SipClient** deleteClientArray = NULL;


#ifdef TEST_PRINT
    UtlString clientNames;
#endif

    while ((client = (SipClient*)mClientList.next(iteratorHandle)))
    {
        if(client->isInUseForWrite()) numBusy++;

        // Remove any client with a bad socket
        // With TCP clients let them stay around if they are still
        // good as the may stay open for the session
        // The clients opened from this side for sending requests
        // get closed by the server (i.e. other side).  The clients
        // opened as servers for requests from the remote side are
        // explicitly closed on this side when the final response is
        // sent.
        if(!client->isInUseForWrite() &&
            (!client->isOk()
           || client->getLastTouchedTime() < oldTime))
        {
#ifdef TEST_PRINT
            client->getClientNames(clientNames);
            osPrintf("Removing %s client names:\n%s\r\n",
                mProtocolString.data(), clientNames.data());
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "Removing %s client names:\n%s\r\n",
                mProtocolString.data(), clientNames.data());
    	    int isBusy = client->isInUseForWrite();
    	    OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServerBase::removeOldClients %p isInUseForWrite %d",
                mProtocolString.data(), client, isBusy);
#endif
            mClientList.remove(iteratorHandle);
            // Delete the clients after releasing the lock
            if(!deleteClientArray) deleteClientArray =
                new SipClient*[numClients];

            deleteClientArray[numDelete] = client;
            numDelete++;

            client = NULL;
        }
        else
        {
            UtlString names;
            client->getClientNames(names);
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServer::removeOldClients leaving client:\n%s\n",
                mProtocolString.data(), names.data());
        }
    }
    mClientList.releaseIteratorHandle(iteratorHandle);
    mClientLock.releaseWrite();

    if ( numDelete || numBusy ) // get rid of lots of 'doing nothing when nothing to do' messages in the log
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServer::removeOldClients deleting %d of %d SipClients (%d busy)\n",
                      mProtocolString.data(), numDelete, numClients, numBusy);
    }
    // These have been removed from the list so delete them
    // after releasing the locks
    for(int clientIndex = 0; clientIndex < numDelete; clientIndex++)
    {
        delete deleteClientArray[clientIndex];
    }

    if(deleteClientArray)
    {
	delete[] deleteClientArray;
	deleteClientArray = NULL;
    }
}

// Assignment operator
SipProtocolServerBase&
SipProtocolServerBase::operator=(const SipProtocolServerBase& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void SipProtocolServerBase::startClients()
{
        int iteratorHandle = mClientList.getIteratorHandle();
    SipClient* client = NULL;
    while ((client = (SipClient*)mClientList.next(iteratorHandle)))
    {
        client->start();
    }
    mClientList.releaseIteratorHandle(iteratorHandle);
}

void SipProtocolServerBase::shutdownClients()
{
        // For each client request shutdown
    int iteratorHandle = mClientList.getIteratorHandle();
    SipClient* client = NULL;
    while ((client = (SipClient*)mClientList.next(iteratorHandle)))
    {
        client->requestShutdown();
    }
    mClientList.releaseIteratorHandle(iteratorHandle);
}

/* ============================ ACCESSORS ================================= */
int SipProtocolServerBase::getClientCount()
{
    return(mClientList.getCount());
}

void SipProtocolServerBase::addClient(SipClient* client)
{
    if(client)
    {
        mClientList.push(client);
    }
}

UtlBoolean SipProtocolServerBase::clientExists(SipClient* client)
{
    SipClient* listClient;
    UtlBoolean found = FALSE;

    int iteratorHandle = mClientList.getIteratorHandle();
    while ((listClient = (SipClient*)mClientList.next(iteratorHandle)))
    {
        if(client == listClient)
        {
            found = TRUE;
            break;
        }
    }

    mClientList.releaseIteratorHandle(iteratorHandle);

    return(found);
}

void SipProtocolServerBase::printStatus()
{
    int numClients = mClientList.getCount();
    int iteratorHandle = mClientList.getIteratorHandle();

    OsTime time;
    OsDateTime::getCurTimeSinceBoot(time);
    long currentTime = time.seconds();

    //long currentTime = OsDateTime::getSecsSinceEpoch();
    SipClient* client;
    UtlString clientNames;
    long clientTouchedTime;
    UtlBoolean clientOk;

    osPrintf("%s %d clients in list at: %ld\n",
        mProtocolString.data(), numClients, currentTime);

    while ((client = (SipClient*)mClientList.next(iteratorHandle)))
    {
        // Remove this or any other bad client
        clientTouchedTime = client->getLastTouchedTime();
        clientOk = client->isOk();
        client->getClientNames(clientNames);

        osPrintf("%s client %p last used: %ld ok: %d names:\n%s\n",
            mProtocolString.data(), this, clientTouchedTime,
            clientOk, clientNames.data());
    }
    mClientList.releaseIteratorHandle(iteratorHandle);
}

/* ============================ INQUIRY =================================== */

#ifdef LOOKING_FOR_T220_COMPILER_BUG /* [ */
int SipProtocolServerBase::dumpLayout(void *Ths)
{
   SipProtocolServerBase* THIS = (SipProtocolServerBase*) Ths;
   printf("SipProtocolServerBase: size = %d bytes\n", sizeof(*THIS));
   printf("  offset(startOfSipProtocolServerBase) = %d\n",
      (((int) &(THIS->startOfSipProtocolServerBase)) - ((int) THIS)));
   printf("  offset(mProtocolString) = %d\n",
      (((int) &(THIS->mProtocolString)) - ((int) THIS)));
   printf("  offset(mDefaultPort) = %d\n",
      (((int) &(THIS->mDefaultPort)) - ((int) THIS)));
   printf("  offset(mSipUserAgent) = %d\n",
      (((int) &(THIS->mSipUserAgent)) - ((int) THIS)));
   printf("  offset(mClientLock) = %d\n",
      (((int) &(THIS->mClientLock)) - ((int) THIS)));
   printf("  offset(mClientList) = %d\n",
      (((int) &(THIS->mClientList)) - ((int) THIS)));
   printf("  offset(endOfSipProtocolServerBase) = %d\n",
      (((int) &(THIS->endOfSipProtocolServerBase)) - ((int) THIS)));
   OsLockingList::dumpLayout(Ths);
   return sizeof(*THIS);
}
#endif /* LOOKING_FOR_T220_COMPILER_BUG ] */ 

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

