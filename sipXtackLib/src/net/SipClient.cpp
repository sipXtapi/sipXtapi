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
#include <stdio.h>

//#define TEST_PRINT

// APPLICATION INCLUDES
#include <net/SipMessage.h>
#include <net/SipClient.h>
#include <net/SipMessageEvent.h>
#include <net/SipUserAgentBase.h>

#include <os/OsDateTime.h>
#include <os/OsDatagramSocket.h>
#include <os/OsSysLog.h>
#include <os/OsEvent.h>

#define SIP_DEFAULT_RTT 500
//#define _VXWORKS

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// All requests must contain at least 72 characters:
/*
X Y SIP/2.0 \n\r
i: A\n\r
f: B\n\r
t: C\n\r
c: 1\n\r
v: SIP/2.0/UDP D\n\r
l: 0 \n\r
\n\r

*/
// However to be tolerant of malformed messages we allow smaller:
#define MINIMUM_SIP_MESSAGE_SIZE 30
#define MAX_UDP_PACKET_SIZE (1024 * 64)

// STATIC VARIABLE INITIALIZATIONS
//#define TEST_PRINT
//#define LOG_TIME
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipClient::SipClient(OsSocket* socket) :
 OsTask("SipClient-%d"),
 mSocketLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
 {
   //set default value for first transcation time out
   mFirstResendTimeoutMs = SIP_DEFAULT_RTT * 4;
	sipUserAgent = NULL;
	clientSocket = socket;
    mRemoteViaPort = -1;
    mRemoteReceivedPort = -1;
    mWaitingList = NULL;
    mInUseForWrite = 0;
    mbSharedSocket = FALSE ;

    touch();
    
   if(socket)
   {
       socket->getRemoteHostName(&mRemoteHostName);
       socket->getRemoteHostIp(&mRemoteSocketAddress, &mRemoteHostPort);

#ifdef TEST_PRINT
       UtlString remoteSocketHost;
       socket->getRemoteHostName(&remoteSocketHost);
		osPrintf("SipClient created with socket descriptor: %d host: %s port: %d\n",
			socket->getSocketDescriptor(), remoteSocketHost.data(),
            socket->getRemoteHostPort());
#endif
   }

}

// Copy constructor
 SipClient::SipClient(const SipClient& rSipClient) :
mSocketLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
}

// Destructor
SipClient::~SipClient()
{
#ifdef TEST_PRINT
        osPrintf("SipClient::~SipClient Start\n");
#endif

	// Do not delete the event listers they are not subordinate

	// Free the socket
	if(clientSocket)
	{
        // Close the socket to unblock the run method
        // in case it is blocked in a isReadyToRead or
        // a read on the clientSocket.  This should also
        // cause the run method to exit.
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipClient::~SipClient 0%x socket 0%x closing socket type: %d\n",
            this, clientSocket, clientSocket->getIpProtocol());

        osPrintf("SipClient::~SipClient closing socket\n");
#endif
      

        if (!mbSharedSocket)
        {
           clientSocket->close();
        }

	   // Signal everybody to stop waiting for this SipClient
	   signalAllAvailableForWrite();

        // Wait for the task to exit so that it does not
        // reference the socket or other members after they
        // get deleted.
        if(isStarted() || isShuttingDown())
        {
#ifdef TEST_PRINT
            osPrintf("SipClient::~SipClient waitUntilShutDown\n");
#endif
            waitUntilShutDown();
        }

#ifdef TEST_PRINT
        osPrintf("SipClient::~SipClient shutDown\n");
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipClient::~SipClient 0%x socket 0%x deleting socket\n",
            this, clientSocket);
#endif
        if (!mbSharedSocket)
        {
		   delete clientSocket;
        }
		clientSocket = NULL;


	}
    else if(isStarted() || isShuttingDown())
    {
		#ifdef TEST_PRINT
        osPrintf("SipClient::~SipClient Wait until shutdown\n");
		#endif

        // It should not get here but just in case
        waitUntilShutDown();
    }

    if(mWaitingList)
    {
        int numEvents = mWaitingList->entries();
        if(numEvents)
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING, "SipClient::~SipClient has %d waiting events\n",
                numEvents);
        }

        delete mWaitingList;
        mWaitingList = NULL;
    }
}

/* ============================ MANIPULATORS ============================== */

int SipClient::run(void* runArg)
{
	int bytesRead;
	//char* buffer = new char[HTTP_DEFAULT_SOCKET_BUFFER_SIZE + 1];
    UtlString buffer;
	SipMessage* message = NULL;
	UtlString remoteHostName;
	UtlString viaProtocol;
    UtlString fromIpAddress;
    int fromPort;
    int numFailures = 0;
	UtlBoolean internalShutdown = FALSE;
    UtlBoolean readAMessage = FALSE;
	int readBufferSize = HTTP_DEFAULT_SOCKET_BUFFER_SIZE;

	if(clientSocket->getIpProtocol() == OsSocket::UDP)
	{
		readBufferSize = MAX_UDP_PACKET_SIZE;
	}


	while(! isShuttingDown() && 
        !internalShutdown &&
        clientSocket &&
        clientSocket->isOk())
	{
		if(clientSocket)
		{
#ifdef TEST_PRINT
           osPrintf("SIP Client waiting for bytes\n");
#endif
			//bytesRead = clientSocket->read(buffer, 
			//	HTTP_DEFAULT_SOCKET_BUFFER_SIZE,
            //    &fromIpAddress, &fromPort);

#ifdef LOG_TIME
            OsTimeLog eventTimes;
#endif
            
            message = new SipMessage();


#ifdef TEST_PRINT
            osPrintf("**************************************\n");
            osPrintf("CREATING message @ address: %X\n",message); 
            osPrintf("HttpMessageCount: %d \n",message->getHttpMessageCount());
            osPrintf("**************************************\n");
#endif            
            // Block and wait for the socket to be ready to read
            // clientSocket shouldn't be null
            // in this case some sort of race with the destructor.  This should
            // not actually ever happen.
            if(clientSocket && 
                ((readAMessage && buffer.length() > MINIMUM_SIP_MESSAGE_SIZE) ||
                 isReadyToRead())) 
            {
#ifdef LOG_TIME
                eventTimes.addEvent("locking");
#endif
                //osPrintf("SipClient::Run - Inside if(clientSocket->isReadyToRead(-1))\n");
                //Lock to prevent multitreaded read or write
#ifdef TEST_PRINT
   osPrintf("SipClient::run before mSocketLock.acquire();\n");
#endif
                mSocketLock.acquire();
#ifdef TEST_PRINT
   osPrintf("SipClient::run after mSocketLock.acquire();\n");
#endif
                //osPrintf("SipClient::Run - accuire socket - Lock to prevent multitreaded read or write\n");

#ifdef LOG_TIME
                eventTimes.addEvent("locked");
#endif
                // This second check is in case there is
                // some sort of race with the destructor.  This should
                // not actually ever happen.
                if(clientSocket)
                {
                    OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipClient::run client: 0%p socket 0%p host: %s socket address: %s via address: %s received address: %s socket type: %d locked to read\n",
                        this, clientSocket, 
                        mRemoteHostName.data(), 
                        mRemoteSocketAddress.data(), 
                        mRemoteViaAddress.data(), 
                        mReceivedAddress.data(), 
                        clientSocket->getIpProtocol());

#ifdef LOG_TIME
                    eventTimes.addEvent("reading");
#endif
                    bytesRead = message->read(clientSocket, 
                        readBufferSize, &buffer);

#ifdef LOG_TIME
                    eventTimes.addEvent("read");
#endif
                }
                else
                {
                    OsSysLog::add(FAC_SIP, PRI_ERR, "SipClient::run client 0%p socket attempt to read NULL\n",
                        this);
                    bytesRead = 0;
                }

                //osPrintf("SipClient::Run - accuire socket - Bytes Read\n");
#ifdef TEST_PRINT
   osPrintf("SipClient::run before mSocketLock.release();\n");
#endif
                mSocketLock.release();
#ifdef TEST_PRINT
   osPrintf("SipClient::run after mSocketLock.release();\n");
#endif

#ifdef LOG_TIME
                eventTimes.addEvent("released");
#endif
                //osPrintf("SipClient::Run - accuire socket - Release Lock\n");
                message->replaceShortFieldNames();
                message->getSendAddress(&fromIpAddress, &fromPort);
            }
            else
            {
                bytesRead = 0;

                if(clientSocket == NULL)
                {
                    OsSysLog::add(FAC_SIP, PRI_ERR, "SipClient::run client 0%p socket is NULL\n",
                        this);
                }
            }

#ifdef TEST_PRINT
			osPrintf("SIP client read %d bytes\n", bytesRead);
#endif
            if(clientSocket && // This second check is in case there is
                // some sort of race with the destructor.  This should
                // not actually ever happen.
               (bytesRead <= 0 || !clientSocket->isOk()))
            {
                numFailures++;
                readAMessage = FALSE;


#ifdef TEST_PRINT
      			osPrintf("SIP client read failure #%d\n", numFailures);
#endif
                if(numFailures > 8 || !clientSocket->isOk())
                {
                    // The socket has gone sour close down the client
#ifdef TEST_PRINT
                    osPrintf("SipClient::Run - The socket has gone sour close down the client\n");
                    OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipClient::Run - The socket has gone sour close down the client\n");
#endif
                    remoteHostName.remove(0);
                    clientSocket->getRemoteHostName(&remoteHostName);
                    OsSysLog::add(FAC_SIP, PRI_DEBUG, "Shutting down client: %s due to failed socket: %d\n",
	                    remoteHostName.data(), clientSocket->getSocketDescriptor());

#ifdef _VXWORKS
               mSocketLock.acquire();
#endif
                    clientSocket->close();
#ifdef _VXWORKS
               mSocketLock.release();
#endif
                    internalShutdown = TRUE;
                }
            }
            else if(bytesRead > 0)
            {
                numFailures = 0;
	            touch();
#ifdef TEST_PRINT
				osPrintf("Read SIP message:\n%s====================END====================\n", buffer.data());
#endif
                if(sipUserAgent)
                {                    
                    UtlString socketRemoteHost;
                    UtlString lastAddress;
                    UtlString lastProtocol;
                    int lastPort;

                    // Only bother processing if the logs are enabled
                    if (sipUserAgent->isMessageLoggingEnabled() || 
                            OsSysLog::willLog(FAC_SIP_INCOMING, PRI_INFO))
                    {
					    UtlString logMessage;
                        logMessage.append("Read SIP message:\n");
					    logMessage.append("----Remote Host:");
					    logMessage.append(fromIpAddress);
					    logMessage.append("---- Port: ");
					    char buff[10];
					    sprintf(buff, "%d",fromPort == 0? 5060:fromPort);
					    logMessage.append(buff);
					    logMessage.append("----\n");

                        logMessage.append(buffer.data(), bytesRead);
					    UtlString messageString;
					    logMessage.append(messageString);
                        logMessage.append("====================END====================\n");

					    sipUserAgent->logMessage(logMessage.data(), logMessage.length());
                        OsSysLog::add(FAC_SIP_INCOMING, PRI_INFO, "%s", logMessage.data());
                    }

                    // Set the date field if not present
                    long epochDate;
                    if(!message->getDateField(&epochDate))
                    {
                        message->setDateField();
                    }

                    //if (message)
	                //	delete message;
	                //message = new SipMessage(buffer, bytesRead);
	                // Save how this message got here
	                message->setSendProtocol(clientSocket->getIpProtocol());
                    message->setTransportTime(touchedTime);
	                clientSocket->getRemoteHostIp(&socketRemoteHost);
	                //message.setSentAddress(socketRemoteHost.data());
#ifdef TEST_PRINT
	                osPrintf("SipClient::run remoteHostIp: %s fromIpAddress: %s:%d\n", 
                        socketRemoteHost.data(), fromIpAddress.data(),
                        fromPort);
#endif

                    // Keep track of where this message came from
                    message->setSendAddress(fromIpAddress.data(), fromPort);

                    if(mReceivedAddress.isNull())
                    {
                        mReceivedAddress = fromIpAddress;
                        mRemoteReceivedPort = fromPort;
                    }

                    // If this is a request
                    if(!message->isResponse())
                    {
                        int receivedPort;
                        UtlBoolean receivedSet;
                        UtlBoolean maddrSet;
                        UtlBoolean receivedPortSet;
                        // Check that the via is set to the address from whence
                        // this message came
                        message->getLastVia(&lastAddress, &lastPort, &lastProtocol,
                            &receivedPort, &receivedSet, &maddrSet, &receivedPortSet);
#ifdef TEST_PRINT
                        osPrintf("Via address: %s Socket peer address: %s port: %d\n",
                           lastAddress.data(), fromIpAddress.data(), fromPort);
#endif
                        // The via address is different from that of the sockets
                        if(strcmp(lastAddress.data(), fromIpAddress.data()) != 0)
                        {
                            // Add a receive from tag
                            message->setLastViaTag(fromIpAddress.data());
                        }

                        // If the rport tag is present the sender wants to
                        // know what port this message was received from
                        int tempLastPort = lastPort;
                        if(lastPort == 0) tempLastPort = 5060;

//                        if(receivedPortSet &&
//DWW                            tempLastPort != fromPort)

                        if (receivedPortSet)
                        {
                            char portString[20];
                            sprintf(portString, "%d", fromPort);
                            message->setLastViaTag(portString, "rport");
                        }

                        int ipProtocolType = clientSocket->getIpProtocol();

                        if((ipProtocolType == OsSocket::TCP ||
                            ipProtocolType == OsSocket::SSL_SOCKET) &&
                            !receivedPortSet)
                        {
                            // we can use this socket as if it were
                            // connected to the port specified in the
                            // via field
                            mRemoteReceivedPort = lastPort;
                        }

                        // Keep track of the address the other
                        // side said they sent from.  Note, this cannot
                        // be trusted unless this transaction is
                        // authenticated (so basically it cannot be 
                        // trusted).
                        if(mRemoteViaAddress.isNull())
	                    {
                            mRemoteViaAddress = lastAddress;
                            mRemoteViaPort = lastPort > 0 ? lastPort : 5060;
                        }
                    }

                    // We read a whole message whether it is a valid one or
                    // not does not matter
                    readAMessage = TRUE;

					// Check that we have the minimum data to define a transaction
					UtlString callId;
					UtlString fromField;
					UtlString toField;
		            message->getCallIdField(&callId);
		            message->getFromField(&fromField);
		            message->getToField(&toField);

					if(!callId.isNull() &&
		                !fromField.isNull() &&
		                !toField.isNull())
	                {
#ifdef LOG_TIME
                        eventTimes.addEvent("dispatching");
#endif

#ifdef TEST_PRINT
                        osPrintf("**************************************\n");
                        osPrintf("DISPATCHING message @ address: %X\n",message); 
                        osPrintf("**************************************\n");
#endif
		                sipUserAgent->dispatch(message);
#ifdef LOG_TIME
                        eventTimes.addEvent("dispatched");
#endif
	                    // Do not delete the SIP message it gets dispatched
                        message = NULL;

	                }
                    else
	                {
                        // Only bother processing if the logs are enabled
                        if (sipUserAgent->isMessageLoggingEnabled())
                        {
                            UtlString msgBytes;
		                    int msgLen;
		                    message->getBytes(&msgBytes, &msgLen);
		                    msgBytes.insert(0, "Received incomplete message (missing To, From or Call-Id header)\n");
		                    msgBytes.append("++++++++++++++++++++END++++++++++++++++++++\n");
		                    sipUserAgent->logMessage(msgBytes.data(), msgBytes.length());
                        }

#ifdef TEST_PRINT
                        osPrintf("**************************************\n");
                        osPrintf("DELETING message @ address: %X\n",message); 
                        osPrintf("**************************************\n");
#endif
		                delete message;
                        message = NULL;
	                }

                } //if sipuseragent 

                // Get rid of the consumed stuff in the buffer so it 
                // contains only bytes which are part of the next message
                buffer.remove(0, bytesRead);

                if(buffer.length())
                {
                    osPrintf("SipClient::run buffer residual bytes: %d\n===>%s<===\n",
                        buffer.length(), buffer.data());
                }
			}

#ifdef LOG_TIME
            UtlString timeString;
            eventTimes.getLogString(timeString);
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipClient::run time log: %s\n",
                timeString.data());
#endif
            if(message) 
            {
                delete message;

#ifdef TEST_PRINT
                osPrintf("**************************************\n");
                osPrintf("DELETING message @ address: %X\n",message); 
                osPrintf("**************************************\n");
#endif            
            }
            message = NULL;
		}
		else
		{
            OsSysLog::add(FAC_SIP, PRI_ERR, "SipClient::run client 0%p socket is NULL yielding\n",
                        this);
			yield();  // I do not know why this yield is here
		}
	}

#ifdef TEST_PRINT
//	DWW osPrintf("SIP Client: %s/%s exiting\r\n", remoteHostName.data(),
//		viaName.data());
#endif
    //if(buffer) delete buffer;
    //buffer = NULL;

    return(0);
}

UtlBoolean SipClient::isReadyToRead()
{
   UtlBoolean readyToRead = FALSE;

#ifdef _VXWORKS

   UtlBoolean bSocketError = FALSE;
   int isReadyCount = 0;

	// On Vxworks we set a maximum block time to prevent
	// locking out of close of the socket in the SipClient
	// destructor
   do
   {
      isReadyCount++;

   	mSocketLock.acquire();
	   readyToRead = clientSocket->isReadyToReadEx(50,bSocketError);	
      mSocketLock.release();
      
   } while (!bSocketError && clientSocket->isOk() && !readyToRead);  
#else
	   readyToRead = clientSocket->isReadyToRead(-1);	
#endif
   

	return(readyToRead);
}


UtlBoolean SipClient::send(SipMessage* message)
{
#ifdef TEST_PRINT
   osPrintf("BEGIN SipClient::send\n");
#endif
   UtlBoolean sendOk = FALSE;
   UtlString viaProtocol;

   if(clientSocket)
   {
      if(!clientSocket->isOk())
      {
#ifdef TEST_PRINT
         osPrintf("Bad socket trying to reconnect\n");
#endif
         clientSocket->reconnect();
#ifdef TEST_SOCKET
         if(clientSocket)
            osPrintf("SipClient reconnected with socket descriptor: %d\n",
                     reconnect->getSocketDescriptor());
#endif
      }

      else
      {
#ifdef LOG_TIME
         OsTimeLog eventTimes;
         eventTimes.addEvent("wait to write");
#endif
#ifdef _VXWORKS
         mSocketLock.acquire();
#endif
         // Wait until the socket is ready to write
         if(clientSocket->isReadyToWrite(mFirstResendTimeoutMs))
         {
#ifdef LOG_TIME
            eventTimes.addEvent("wait to lock");
#endif
            //Lock to prevent multitreaded read or write
#ifndef _VXWORKS
            mSocketLock.acquire();
#endif
#ifdef LOG_TIME
            eventTimes.addEvent("writing");
#endif
            sendOk = message->write(clientSocket);
#ifdef LOG_TIME
            eventTimes.addEvent("releasing");
#endif
#ifndef _VXWORKS
            mSocketLock.release();
#endif
#ifdef LOG_TIME
            eventTimes.addEvent("released");
            UtlString timeString;
            eventTimes.getLogString(timeString);
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipClient::send time log: %s\n",
                          timeString.data());
#endif

            if(sendOk)
            {
               touch();
            }
         }
         else
         {
            clientSocket->close();
         }

#ifdef _VXWORKS
         mSocketLock.release();
#endif
      }
   }

#ifdef TEST_PRINT
   osPrintf("END SipClient::send\n");
#endif

   return(sendOk);
}

UtlBoolean SipClient::sendTo(const SipMessage& message,
                            const char* address,
                            int port)
{
	UtlBoolean sendOk = FALSE;
	UtlString viaProtocol;
       

    // In the case of using UDP, the port being 0 is o.k. but needs to be change to 5060 for sending the message.
    if (port == 0)
    {
        port = 5060 ;
    }

	if(clientSocket && 
        clientSocket->getIpProtocol() == OsSocket::UDP)
    {
        UtlString buffer;
	    int bufferLen;
	    int bytesWritten;

	    message.getBytes(&buffer, &bufferLen);
        // Wait until the socket is ready to write
        if(clientSocket->isReadyToWrite(mFirstResendTimeoutMs))
        {
            //Lock to prevent multitreaded read or write
            mSocketLock.acquire();
	        bytesWritten = ((OsDatagramSocket*)clientSocket)->write(buffer.data(), bufferLen,
               address, port);
            mSocketLock.release();

		    if(bufferLen == bytesWritten)
		    {
                sendOk = TRUE;
		    	touch();
		    }
            else
            {
               sendOk = FALSE;
            }
        }
	}
    else if (clientSocket && 
            clientSocket->getIpProtocol() == OsSocket::TCP)
    {
        sendOk = send((SipMessage*) &message) ;
    }

	return(sendOk);
}

// Assignment operator
SipClient& 
SipClient::operator=(const SipClient& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void SipClient::notifyWhenAvailableForWrite(OsEvent& availableEvent)
{

    if(mWaitingList == NULL)
    {
        mWaitingList = new UtlSList();
    }

    UtlInt* eventNode = new UtlInt((int)&availableEvent);

    mWaitingList->append(eventNode);
}

void SipClient::signalNextAvailableForWrite()
{
    if(mWaitingList)
    {
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipClient::signalNextAvailableForWrite 0x%x\n",
                    this);
#endif

        // Remove the first event that is waiting for this transaction
        UtlInt* eventNode = (UtlInt*) mWaitingList->get();

        if(eventNode)
        {
            OsEvent* waitingEvent = (OsEvent*) eventNode->getValue();
#ifdef TEST_PRINT
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipClient::signalNextAvailableForWrite 0x%x signaling: 0x%x\n",
                    this, waitingEvent);
#endif
            if(waitingEvent)
            {
				// If the other side is done accessing the event
				// and has already signalled it, then we can delete
				// this.  Otherwise the other side must do the delete.
                if(OS_ALREADY_SIGNALED == waitingEvent->signal(1))
				{
					delete waitingEvent;
					waitingEvent = NULL;
				}
            }
            delete eventNode;
            eventNode = NULL;
        }
    }
}

void SipClient::signalAllAvailableForWrite()
{
    if(mWaitingList)
    {
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipClient::signalAllAvailableForWrite 0x%x\n",
                    this);
#endif

        // Remove the first event that is waiting for this transaction
        UtlInt* eventNode = NULL;
        while((eventNode = (UtlInt*) mWaitingList->get()))
        {
            if(eventNode)
            {
                OsEvent* waitingEvent = (OsEvent*) eventNode->getValue();
#ifdef TEST_PRINT
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipClient::signalAllAvailableForWrite 0x%x signaling: 0x%x\n",
                    this, waitingEvent);
#endif
                if(waitingEvent)
                {
					// If the other side is done accessing the event
					// and has already signalled it, then we can delete
					// this.  Otherwise the other side must do the delete.
					if(OS_ALREADY_SIGNALED == waitingEvent->signal(1))
					{
						delete waitingEvent;
						waitingEvent = NULL;
					}
                }
                delete eventNode;
                eventNode = NULL;
            }
        }
    }
}

void SipClient::setSharedSocket(UtlBoolean bShared) 
{
    mbSharedSocket = bShared ;
}
/* ============================ ACCESSORS ================================= */

void SipClient::getClientNames(UtlString& clientNames) const
{
    char portString[40];

    // host DNS name
    sprintf(portString, "%d", mRemoteHostPort);
    clientNames = "\tremote host: ";
    clientNames.append(mRemoteHostName);
    clientNames.append(":");
    clientNames.append(portString);

    // host IP address
    clientNames.append("\n\tremote IP: ");
    clientNames.append(mRemoteSocketAddress);
    clientNames.append(":");
    clientNames.append(portString);

    // via address
    sprintf(portString, "%d", mRemoteViaPort);
    clientNames.append("\n\tremote Via address: ");
    clientNames.append(mRemoteViaAddress);
    clientNames.append(":");
    clientNames.append(portString);

    // recieved address
    sprintf(portString, "%d", mRemoteReceivedPort);
    clientNames.append("\n\treceived address: ");
    clientNames.append(mReceivedAddress);
    clientNames.append(":");
    clientNames.append(portString);
}

void SipClient::setUserAgent(SipUserAgentBase* sipUA)
{
	sipUserAgent = sipUA;
   //mFirstResendTimeoutMs = (sipUserAgent->getFirstResendTimeout()) * 4;
}

long SipClient::getLastTouchedTime() const
{
	return(touchedTime);
}

void SipClient::touch()
{
   OsTime time;
   OsDateTime::getCurTimeSinceBoot(time);
   touchedTime = time.seconds();
   //OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipClient::touch client: 0x%x time: %d\n",
   //             this, touchedTime);
}

int SipClient::isInUseForWrite(void)
{
    return(mInUseForWrite);
}

/* ============================ INQUIRY =================================== */

UtlBoolean SipClient::isOk()
{
	return(clientSocket->isOk() && !isShuttingDown());
}

UtlBoolean SipClient::isConnectedTo(UtlString& hostName, int hostPort)
{
    UtlBoolean isSame = FALSE;
    int tempHostPort = hostPort > 0 ? hostPort : 5060;

    // If the ports match and the host is the same as either the
    // original name that the socket was constructed with or the
    // name it was resolved to (usual an IP address).
    if(mRemoteHostPort == tempHostPort &&
        (hostName.compareTo(mRemoteHostName, UtlString::ignoreCase) == 0 ||
         hostName.compareTo(mRemoteSocketAddress, UtlString::ignoreCase) == 0))
    {
        isSame = TRUE;
    }

    else if(mRemoteReceivedPort == tempHostPort &&
        hostName.compareTo(mReceivedAddress, UtlString::ignoreCase) == 0)
    {
        isSame = TRUE;
    }
    else if(mRemoteViaPort == tempHostPort &&
        hostName.compareTo(mRemoteViaAddress, UtlString::ignoreCase) == 0)
    {
        // Cannot trust what the other side said was their IP address
        // as this is a bad spoofing/denial of service whole
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipClient::isConnectedTo matches via address: %s port: %d but is not trusted\n",
            mRemoteViaAddress.data(), mRemoteViaPort);
    }

    return(isSame);
}

void SipClient::markInUseForWrite()
{
    OsTime time;
    OsDateTime::getCurTimeSinceBoot(time);
    mInUseForWrite = time.seconds();
}

void SipClient::markAvailbleForWrite()
{
    mInUseForWrite = 0;
    signalNextAvailableForWrite();
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

