//
// Copyright (C) 2022 SIP Spectrum, Inc.  All rights reserved.
//
// Copyright (C) 2006-2017 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


//  Includes
#include "rtcp/RTCPSession.h"
#ifdef INCLUDE_RTCP /* [ */

//  Constants
const int MAX_CONNECTIONS  = 64;

//  Static Declarations
static unsigned long ulMasterSessionCount = 1;

#if RTCP_DEBUG /* [ */
static const char *suchSDESFieldNames[] = {
    " ",
    "CNAME FIELD   ",
    "NAME FIELD    ",
    "EMAIL FIELD   ",
    "PHONE FIELD   ",
    "LOCATION FIELD",
    "APPNAME FIELD ",
    "NOTE FIELD    ",
    "PRIVATE FIELD "};
#endif /* RTCP_DEBUG ] */

//  Template Comparitor for RTCP Connection Match based upon SSRC ID
bool RTCPConnectionComparitor(CRTCPConnection *poRTCPConnection,
                              void *pvArgument)
{

    IRTCPConnection *piRTCPConnection = (IRTCPConnection *) pvArgument;

    // Check the Connection object within the list to determine whether the
    // SSRC's match
    if(poRTCPConnection == piRTCPConnection)
        return(TRUE);

    return(FALSE);

}


/**
 *
 * Method Name:  CRTCPSession() - Constructor
 *
 *
 * Inputs:   IRTCPNotify *piRTCPNotify - RTCP Event Notification Interface
 *           ISDESReport *piSDESReport
 *                                 - Local Source Description Report Interface
 *
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description:  Performs routine assignment of constructor arguments to
 *               internal attributes.
 *
 * Usage Notes:
 *
 */
CRTCPSession::CRTCPSession(IRTCPNotify *piRTCPNotify, ISDESReport *piSDESReport) :
    CBaseClass(CBASECLASS_CALL_ARGS("CRTCPSession", __LINE__)),
    CTLinkedList<CRTCPConnection *>(),  // Template Contructor Initialization
    m_ulEventInterest(ALL_EVENTS),
    m_etMixerMode(MIXER_DISABLED)
{

    // Store RTCP Notification Interface
    m_piRTCPNotify = piRTCPNotify;

    // Cache the local SDES Report interface
    m_piSDESReport = piSDESReport;

    // Increment reference counts to interface passed
    if(m_piRTCPNotify)
        m_piRTCPNotify->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    if(m_piSDESReport)
        m_piSDESReport->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));

    // Set Session number
    m_ulSessionID = ulMasterSessionCount++;

}

/**
 *
 * Method Name: ~CRTCPSession() - Destructor
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Shall deallocated and/or release all resources which was
 *              acquired over the course of runtime.  In particular, the
 *              following shall occur:
 *    ==> The list containing RTCP Connection objects shall be drained with
 *        the reference to each object released.
 *
 * Usage Notes:
 *
 *
 */
CRTCPSession::~CRTCPSession(void)
{

    // Release Source Description interface reference
    if(m_piSDESReport)
        m_piSDESReport->Release(ADD_RELEASE_CALL_ARGS(__LINE__));

    // Release Event Notification interface reference
    if(m_piRTCPNotify)
        m_piRTCPNotify->Release(ADD_RELEASE_CALL_ARGS(__LINE__));

}


/**
 *
 * Method Name: CreateRTCPConnection
 *
 *
 * Inputs:   ssrc_t localSSRC    - local SSRC for this connection
 *
 * Outputs:  None
 *
 * Returns:  IRTCPConnection * - interface for controlling an RTCP Connection
 *
 * Description: The CreateRTCPConnection shall manage the instantiation of
 *              RTCP Connection object used to report and track the identity
 *              and performance of active RTP connection.
 *
 *
 * Usage Notes: An RTCPConnection object shall be create per RTP Connection.
 *
 *
 */
IRTCPConnection * CRTCPSession::CreateRTCPConnection(ssrc_t localSSRC)
{

    CRTCPConnection *poRTCPConnection;
    // Create The RTCP Connection object
    poRTCPConnection =
           new CRTCPConnection(localSSRC, (IRTCPNotify *)this, m_piSDESReport);
    if (poRTCPConnection == NULL)
    {
        osPrintf("**** FAILURE ***** CRTCPSession::CreateRTCPConnection() -"
                                " Unable to create RTCP Connection object\n");
        return(NULL);
    }

    // Initialize RTCP Connection object
    else if(!poRTCPConnection->Initialize())
    {
        // Release the RTCP Connection reference.
        //  This should cause the object to be destroyed
        osPrintf("**** FAILURE ***** CRTCPSession::CreateRTCPConnection() -"
                            " Unable to Initialize RTCP Connection object\n");
        poRTCPConnection->Terminate();
        ((IRTCPConnection *)poRTCPConnection)->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        return(NULL);
    }


    // Place the new RTCP Connection object on the collection list
    else if(!AddEntry(poRTCPConnection))
    {
        // Release the RTCP Connection reference.
        //  This should cause the object to be destroyed
        osPrintf("**** FAILURE ***** CRTCPSession::CreateRTCPConnection() -"
                     " Unable to Add RTCP Connection object to Collection\n");
        poRTCPConnection->Terminate();
        ((IRTCPConnection *)poRTCPConnection)->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        return(NULL);
    }

#if RTCP_DEBUG /* [ */
    if(bPingtelDebug)
    {
            osPrintf("*** RTCP CONNECTION CREATED ****\n");
        osPrintf("\t ON SESSION ==> %d\n", GetSessionID());
    }
#endif /* RTCP_DEBUG ] */

    // Increment the reference count to account for the
    //  interface being returned
    //  I THINK THIS IS THE LEAK:  ((IRTCPConnection *)poRTCPConnection)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));

    return((IRTCPConnection *)poRTCPConnection);
}


/**
 *
 * Method Name: TerminateRTCPConnection
 *
 *
 * Inputs:   IRTCPConnection *piRTCPConnection
 *                     - The Connection Interface pointer returned on creation
 *
 * Outputs:  None
 *
 * Returns:  bool
 *
 * Description:  The TerminateRTCPConnection() method shall manage the
 *               termination of and RTCP session.  This shall include the
 *               graceful release of all associated objects as well as the
 *               deallocation of all resources associated with each contained
 *               RTCP connection.
 *
 *
 * Usage Notes:
 *
 *
 */
bool CRTCPSession::TerminateRTCPConnection(IRTCPConnection *piRTCPConnection)
{
    CRTCPConnection *poRTCPConnection;

    // Remove the RTCP Connection object from the collection list
    if((poRTCPConnection = RemoveEntry(RTCPConnectionComparitor,
                                      (void *)piRTCPConnection)) != NULL)
    {
        
#if RTCP_DEBUG /* [ */
        if(bPingtelDebug)
        {
            osPrintf("*** RTCP CONNECTION TERMINATED ****\n");
            osPrintf("\t ON SESSION ==> %d\n", GetSessionID());
            osPrintf("\t TO SSRC    ==> %u\n",
                                           piRTCPConnection->GetRemoteSSRC());
        }
#endif /* RTCP_DEBUG ] */

        // Terminate RTCPConnection and release reference
        poRTCPConnection->Terminate();

        // Release reference twice.  Once for its removal from the collection
        //  and once on behalf fo the client since this method serves to
        //  terminate the connection and release the client's reference.
        ((IRTCPConnection *)poRTCPConnection)->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        ((IRTCPConnection *)poRTCPConnection)->Release(ADD_RELEASE_CALL_ARGS(__LINE__));

        return(TRUE);
    }

    return(FALSE);
}

/**
 *
 * Method Name: ResetAllConnections
 *
 *
 * Inputs:      unsigned char *puchReason     - Reason for Reset
 *
 * Outputs:     None
 *
 * Returns:     void

 *
 * Description:  The ResetAllConnections() method shall manage the termination
 *               of all RTCP connections.  This shall include the graceful
 *               release of all associated objects as well as the deallocation
 *               of all resources associated with each contained RTCP
 *               connection.
 *
 *
 * Usage Notes:
 *
 *
 */
void CRTCPSession::ResetAllConnections(unsigned char *puchReason)
{

    ssrc_t aulCSRC[MAX_CONNECTIONS];
    unsigned long ulCSRCs = 0;
    CRTCPConnection *poRTCPConnection;

    // Check whether the session is acting as an Audio Mixer.  If so,
    // We should include these sites into our contributing source list.
    if(m_etMixerMode == MIXER_ENABLED)
    {
        // Check each entry of the connection list
        TakeLock();
        poRTCPConnection = GetFirstEntry();

        // Iterate through the list until all entries have been exhausted
        for(ulCSRCs = 0; poRTCPConnection != NULL && ulCSRCs < MAX_CONNECTIONS; ulCSRCs++)
        {
            // Bump Reference Count of Connection Object
            poRTCPConnection->AddRef(ADD_RELEASE_CALL_ARGS(-__LINE__));

            // Get the SSRC ID of the connection
            aulCSRC[ulCSRCs] = poRTCPConnection->GetRemoteSSRC();

            // Release Reference to Connection Object
            poRTCPConnection->Release(ADD_RELEASE_CALL_ARGS(-__LINE__));

            // Get the next connection from the list
            poRTCPConnection = GetNextEntry();
        }
        ReleaseLock();
    }

    // Check each entry of the connection list again
    TakeLock();
    poRTCPConnection = GetFirstEntry();
    while (poRTCPConnection != NULL)
    {

        // Bump Reference Count of Connection Object
        poRTCPConnection->AddRef(ADD_RELEASE_CALL_ARGS(-__LINE__));

        poRTCPConnection->GenerateRTCPReports(puchReason, aulCSRC, ulCSRCs);

        // Release Reference to Connection Object
        poRTCPConnection->Release(ADD_RELEASE_CALL_ARGS(-__LINE__));

        // Get Next Entry
        poRTCPConnection = GetNextEntry();
    }
    ReleaseLock();
}


/**
 *
 * Method Name: TerminateAllConnections
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     bool

 *
 * Description: The TerminateAllConnections() method shall manage the
 *              termination of all RTCP connections.  This shall include the
 *              graceful release of all associated objects as well as the
 *              deallocation of all resources associated with each contained
 *              RTCP connection.
 *
 *
 * Usage Notes:
 *
 *
 */
void CRTCPSession::TerminateAllConnections(void)
{

    // Reset All Connections
    ResetAllConnections((unsigned char *)"Normal Session Termination");

    // Remove all RTCP Connections
    TakeLock();
    CRTCPConnection *poRTCPConnection = RemoveFirstEntry();
    while (poRTCPConnection != NULL)
    {
#if RTCP_DEBUG /* [ */
        if(bPingtelDebug)
        {
            osPrintf("*** RTCP CONNECTION TERMINATED ****\n");
            osPrintf("\t ON SESSION ==> %d\n", GetSessionID());
            osPrintf("\t TO SSRC    ==> %u\n",
                                           poRTCPConnection->GetRemoteSSRC());
        }
#endif /* RTCP_DEBUG ] */

        // Terminate RTCPConnection and release reference
        poRTCPConnection->Terminate();

        // Release reference
        ((IRTCPConnection *)poRTCPConnection)->Release(ADD_RELEASE_CALL_ARGS(__LINE__));

        // Remove Next Entry
        poRTCPConnection = RemoveNextEntry();
    }
    ReleaseLock();

}


/**
 *
 * Method Name:  ReassignSSRC
 *
 *
 * Inputs:   unsigned long  ulSSRC     - Source ID
 *           unsigned char *puchReason - Optional Reason for SSRC Reassignment
 *
 * Outputs:  None
 *
 * Returns:  void
 *
 * Description: Reassigns the Source Identifier associated with an RTP session
 *              due to collision detection and resolution.  Calling of this
 *              method shall result in the resetting of the SSRC IDs of
 *              associated Sender, Receiver, and SDES Reports.
 *
 * Usage Notes:
 *
 * SLG - This method has been commented out, since this was used in the uncorrect collision
 *       detection and resolution logic, that has been commented out.  Leaving for a
 *       reference in case we ever attempt to fix the logic.
 *
 */
/*
void CRTCPSession::ReassignSSRC(unsigned long ulSSRC,
                                unsigned char *puchReason)
{
#if RTCP_DEBUG 
    if(bPingtelDebug)
    {
        osPrintf("*** SSRC REASSIGNED ****\n");
        osPrintf("\t ON SESSION ==> %d\n", GetSessionID());
        osPrintf("\t NEW SSRC    ==> %u\n", ulSSRC);
        osPrintf("\t REASON     ==> %s\n", puchReason);
    }
#endif 

    // Reset all connections first
    ResetAllConnections(puchReason);

    // Set new Session SSRC
    m_ulSSRC = ulSSRC & SSRC_SESSION_MASK;

    // Check the each entry of the connection list
    TakeLock();
    CRTCPConnection *poRTCPConnection = GetFirstEntry();

    // Iterate through the list until all entries have been exhausted
    while(poRTCPConnection != NULL)
    {
        // Bump Reference Count of Connection Object
        poRTCPConnection->AddRef(ADD_RELEASE_CALL_ARGS(-__LINE__));

        // Get Render Interface
        IRTCPRender *piRTCPRender = poRTCPConnection->GetRenderInterface();

        // Instruct the Render filter of the new SSRC.
        // It will take care of the reset
        piRTCPRender->ReassignSSRC(ulSSRC);

        // Release Render Interface
        piRTCPRender->Release(ADD_RELEASE_CALL_ARGS(__LINE__));

        // Release Reference to Connection Object
        poRTCPConnection->Release(ADD_RELEASE_CALL_ARGS(-__LINE__));

        // Get the next connection from the list
        poRTCPConnection = GetNextEntry();
    }
    ReleaseLock();

}*/


/**
 *
 * Method Name: CheckLocalSSRCCollisions
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Check that our local SSRC is not colliding with one of the
 *              SSRCs of a participating site.
 *
 * Usage Notes:
 *
 * SLG - This method isn't currently used and was part of an original attempt
 *       to detect collisions after connection setup.  However, it's detection
 *       and handling logic was flawed and thus commented out.  Full collision
 *       detection as specified in RFC3550 section 8.2 is still a TODO.
 *
 */

/******************************************************************
 * It appears that this is called every 5 seconds (or so), on the
 # RTCP timer event.
 *
 *  THIS NEEDS SERIOUS REVISION.  The SSRCs should be kept
 * independently per connection, not per session.  And, I cannot
 * tell whether each connection should only check against its own
 * SSRC, or against all other SSRCs in the session/flowgraph.
 * 
 * SLG - since the above comment changes have been made to ensure
 *       that SSRC's are unique per connection, however full collision
 *       detection as specified in RFC3550 section 8.2 is still a TODO.
 *
 *****************************************************************/

/////////////////////////////////////////////////////////////////////////////////////
// Debugging trick: force phantom collisions to see how we handle them
// #define FORCE_SSRC_COLLISIONS 64 // If non-zero, force a collision every Nth call
#if FORCE_SSRC_COLLISIONS /* [ */
static bool collide()
{
    static int counter = 0;
    return (0 == (++counter % FORCE_SSRC_COLLISIONS));
}
#else /* FORCE_SSRC_COLLISIONS ] [ */
#define collide() 0
#endif /* FORCE_SSRC_COLLISIONS ] */
/////////////////////////////////////////////////////////////////////////////////////
void CRTCPSession::CheckLocalSSRCCollisions(void)
{
    /*
    // Check the each entry of the connection list
    UtlBoolean collisionFound = FALSE;
    
    TakeLock();
    CRTCPConnection *poRTCPConnection = GetFirstEntry();
    
    // Iterate through the list until all entries have been exhausted
    while(poRTCPConnection != NULL)
    {
        // Bump Reference Count of Connection Object
        poRTCPConnection->AddRef(ADD_RELEASE_CALL_ARGS(-__LINE__));
    
        // Get the SSRC ID of the connection to determine whether it is
        //  conflicting with ours
        if (collide() || (poRTCPConnection->isRemoteSSRCValid() && ((SSRC_SESSION_MASK & poRTCPConnection->GetRemoteSSRC()) == m_ulSSRC)))
        {
            collisionFound = TRUE;
            OsSysLog::add(FAC_MP, PRI_WARNING, "CRTCPSession::CheckLocalSSRCCollisions: COLLISION:  Session: %p RTCPCxn: %p, SSRC: 0x%08X", this, poRTCPConnection, m_ulSSRC);
            // Adding reference count to connection and session so that
            // we can send a notification referencing connection and session
            // in the collision handling code down below
            poRTCPConnection->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
            ((IRTCPSession *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));

            // As we are bailing on this loop, we must release the ref count
            // on the connection that is normally done below
            poRTCPConnection->Release(ADD_RELEASE_CALL_ARGS(-__LINE__));

            break;
        }
    
        // Release Reference to Connection Object
        poRTCPConnection->Release(ADD_RELEASE_CALL_ARGS(-__LINE__));
    
        // Get the next connection from the list
        poRTCPConnection = GetNextEntry();
    }
    ReleaseLock();

    // Found a connection with a collision
    if(collisionFound)
    {
        // A collision has been detected.
        // Let's reset all the connections.
        // ResetAllConnections does not actually change the SSRC
        ResetAllConnections((unsigned char *)"SSRC Collision");
    
        // Let's inform the RTC Manager and its subscribing clients of
        // this occurence.  Its the receiver of the notification
        // that is responsible for changing the SSRC to address the
        // collision
        TakeLock();
        m_piRTCPNotify->LocalSSRCCollision(
               (IRTCPConnection *)poRTCPConnection, (IRTCPSession *)this);
        ReleaseLock();
    }
    */
}

/**
 *
 * Method Name: CheckRemoteSSRCCollisions
 *
 *
 * Inputs:      IRTCPConnection *piRTCPConnection  - Connection Interface
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Check that other remote SSRCs aren't colliding with each other.
 *
 * Usage Notes:
 *
 * SLG - This method isn't currently used and was part of an original attempt
 *       to detect collisions after connection setup.  However, it's detection
 *       and handling logic was flawed and thus commented out.  Full collision
 *       detection as specified in RFC3550 section 8.2 is still a TODO.
 *
 */
void CRTCPSession::CheckRemoteSSRCCollisions(IRTCPConnection *piRTCPConnection)
{

    // Check for Collision
    TakeLock();
    CRTCPConnection *poRTCPConnection = GetFirstEntry();
    while(poRTCPConnection)
    {
        // Bump Reference Count of Connection Object
        poRTCPConnection->AddRef(ADD_RELEASE_CALL_ARGS(-__LINE__));

        bool bInitialSSRCFound = FALSE;
        if(poRTCPConnection->GetRemoteSSRC() ==
                                            piRTCPConnection->GetRemoteSSRC())
        {
            if(!bInitialSSRCFound)
                bInitialSSRCFound = TRUE;
            else
            {
                // A collision has been detected.
                // Let's inform the RTC Manager and its subscribing client's
                //  of this occurence.
                poRTCPConnection->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
                ((IRTCPSession *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
                m_piRTCPNotify->RemoteSSRCCollision(
                   (IRTCPConnection *)poRTCPConnection, (IRTCPSession *)this);
            }
        }

        // Release Reference to Connection Object
        poRTCPConnection->Release(ADD_RELEASE_CALL_ARGS(-__LINE__));

        // Get the next connection from the list
        poRTCPConnection = GetNextEntry();
    }
    ReleaseLock();
}


/**
 *
 * Method Name: IsSSRCInUse
 *
 *
 * Inputs:   ssrc_t ssrc  - The SSRC value to check
 *
 * Outputs:  None
 *
 * Returns:  void
 *
 * Description: Check that provided SSRC doesn't the a local or remote SSRC of an
                existing connection.
 *
 * Usage Notes:
 *
 * SLG - This method is newer than the above 2 methods and while it doesn't 
 *       accomplish the full goals of RFC3550 section 8.2.  It does at least 
 *       ensure that at generation time we don't generate an SSRC value 
 *       that is already known to be in use.
 *
 */
bool CRTCPSession::IsSSRCInUse(ssrc_t ssrc)
{
    bool bIsSSRCInUse = false;

    // Check if used in each connection, check both local and remote SSRCs
    TakeLock();
    CRTCPConnection* poRTCPConnection = GetFirstEntry();
    while (poRTCPConnection)
    {
        // Bump Reference Count of Connection Object
        poRTCPConnection->AddRef(ADD_RELEASE_CALL_ARGS(-__LINE__));

        if (poRTCPConnection->GetLocalSSRC() == ssrc ||
            poRTCPConnection->GetRemoteSSRC() == ssrc)
        {
            bIsSSRCInUse = true;

            // Release Reference to Connection Object
            poRTCPConnection->Release(ADD_RELEASE_CALL_ARGS(-__LINE__));

            break;
        }

        // Release Reference to Connection Object
        poRTCPConnection->Release(ADD_RELEASE_CALL_ARGS(-__LINE__));

        // Get the next connection from the list
        poRTCPConnection = GetNextEntry();
    }
    ReleaseLock();

    return bIsSSRCInUse;
}


/**
 *
 * Method Name: ForwardSDESReport
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *                              - Interface for getting SDES Report Statistics
 *           IRTCPConnection  *piRTCPConnection
 *                              - Interface to RTCP Connection originating SDES
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ForwardSDESReport() method shall enable the RTC Manager to
 *              pass interfaces to SDES Reports received from participating
 *              site while acting in the mode of a conference Mixer.  The
 *              Mixer's role in this situation is to transmit these reports
 *              unchanged to others participating within a conference.  The
 *              handoff of an SDES Report to the CRTCPRender will cause the
 *              report to be transmitted to a participating site using the
 *              associated Network Render object.
 *
 * Usage Notes: The interface for the local site's Source Description Report
 *              generator is passed as an argument at construction time.
 *
 *
 */
void CRTCPSession::ForwardSDESReport(IGetSrcDescription *piGetSrcDescription,
                                     IRTCPConnection    *piRTCPConnection)
{

    // Check the each entry of the connection list and forward this report to
    //  those not matching the originators SSRC
    TakeLock();
    CRTCPConnection *poRTCPConnection = GetFirstEntry();

    // Iterate through the list until all entries have been exhausted
    while(poRTCPConnection != NULL)
    {
        // Bump Reference Count of Connection Object
        poRTCPConnection->AddRef(ADD_RELEASE_CALL_ARGS(-__LINE__));

        // Get the SSRC ID of the connection to determine whether it is not
        //  from the originator
        if(poRTCPConnection->GetRemoteSSRC() !=
                                            piRTCPConnection->GetRemoteSSRC())
        {
            // Get Render Interface
            IRTCPRender *piRTCPRender = poRTCPConnection->GetRenderInterface();

#if RTCP_DEBUG /* [ */
            if(bPingtelDebug)
            {
                osPrintf("*** FORWARDING SDES REPORT ****\n");
                osPrintf("\t ON SESSION ==> %d\n", GetSessionID());
                osPrintf("\t FROM SSRC  ==> %u\n",
                                           piRTCPConnection->GetRemoteSSRC());
                osPrintf("\t TO SSRC    ==> %d\n",
                                           poRTCPConnection->GetRemoteSSRC());
            }
#endif /* RTCP_DEBUG ] */

            // Different connection.  Forward the SDES Report for delivery.
            ISDESReport *piSDESReport =
                                      piGetSrcDescription->GetSDESInterface();
            piRTCPRender->ForwardSDESReport(piSDESReport);

            // Release the reference to SDES Report interface
            piSDESReport->Release(ADD_RELEASE_CALL_ARGS(__LINE__));

            // Release Render Interface
            piRTCPRender->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        }

        // Release Reference to Connection Object
        poRTCPConnection->Release(ADD_RELEASE_CALL_ARGS(-__LINE__));

        // Get the next connection from the list
        poRTCPConnection = GetNextEntry();
    }
    ReleaseLock();

}


/**
 *
 * Method Name: ForwardByeReport
 *
 *
 * Inputs:   IGetByeInfo *piGetByeInfo
 *                         - Interface used to retrieve Bye Report information
 *           IRTCPConnection *piRTCPConnection
 *                              - Interface to RTCP Connection originating Bye
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ForwardByeReport() method shall enable the RTC Manager to
 *              pass interfaces to Bye Reports received from participating site
 *              while acting in the mode of a conference Mixer.  The Mixer's
 *              role in this situation is to transmit these reports unchanged
 *              to others participating within a conference.  The handoff of a
 *              Bye Report to the CRTCPRender will cause the report to be
 *              transmitted to a participating site using the associated
 *              Network Render object.
 *
 * Usage Notes:
 *
 *
 */
void CRTCPSession::ForwardByeReport(IGetByeInfo      *piGetByeInfo,
                                    IRTCPConnection  *piRTCPConnection)
{

    // Check the each entry of the connection list and forward this report to
    //  those not matching the originators SSRC
    TakeLock();
    CRTCPConnection *poRTCPConnection = GetFirstEntry();

    // Iterate through the list until all entries have been exhausted
    while(poRTCPConnection != NULL)
    {
        // Bump Reference Count of Connection Object
        poRTCPConnection->AddRef(ADD_RELEASE_CALL_ARGS(-__LINE__));

        // Get the SSRC ID of the connection to determine whether it is not
        //  from the originator
        if(poRTCPConnection->GetRemoteSSRC() !=
                                            piRTCPConnection->GetRemoteSSRC())
        {
            // Get Render Interface
            IRTCPRender *piRTCPRender = poRTCPConnection->GetRenderInterface();

#if RTCP_DEBUG /* [ */
            if(bPingtelDebug)
            {
                osPrintf("*** FORWARDING BYE REPORT ****\n");
                osPrintf("\t ON SESSION ==> %d\n", GetSessionID());
                osPrintf("\t FROM SSRC  ==> %u\n",
                                           piRTCPConnection->GetRemoteSSRC());
                osPrintf("\t TO SSRC    ==> %d\n",
                                           poRTCPConnection->GetRemoteSSRC());
            }
#endif /* RTCP_DEBUG ] */

            // Different connection.  Forward the Bye Report for delivery.
            IByeReport *piByeReport = piGetByeInfo->GetByeInterface();
            piRTCPRender->ForwardByeReport(piByeReport);

            // Release Bye Interface
            piByeReport->Release(ADD_RELEASE_CALL_ARGS(__LINE__));

            // Release Render Interface
            piRTCPRender->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        }

        // Release Reference to Connection Object
        poRTCPConnection->Release(ADD_RELEASE_CALL_ARGS(-__LINE__));

        // Get the next connection from the list
        poRTCPConnection = GetNextEntry();
    }
    ReleaseLock();

}

/**
 *
 * Method Name:  NewSDES()
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *                              - Interface to the new Source Description info
 *           IRTCPConnection    *piRTCPConnection
 *                                 - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                                 - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The NewSDES() event method shall inform the RTCP Session of a
 *              new Source Description and shall include the SSRC ID and the
 *              IGetSrcDescription interface for accessing the contents of this
 *              new Source Description.  The RTCP Session shall forward this
 *              SDES Report to other RTCP Connections in the session while
 *              acting as a Mixer.  It shall also forward the event to the RTC
 *              Manager for distribution to other subscribers.
 *
 * Usage Notes:
 *
 */
void CRTCPSession::NewSDES(IGetSrcDescription *piGetSrcDescription,
                           IRTCPConnection    *piRTCPConnection,
                           IRTCPSession       *piRTCPSession)
{

    // Now forward this event to the RTC Manager so that it can be dispatched
    //  to subscribers with matching interests
    ((IRTCPSession *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    m_piRTCPNotify->NewSDES(piGetSrcDescription,
                                      piRTCPConnection, (IRTCPSession *)this);

#if RTCP_DEBUG /* [ */
    if(bPingtelDebug)
    {
        osPrintf("*** NEW SDES RECEIVED ****\n");
        osPrintf("\t ON SESSION ==> %d\n", GetSessionID());
        osPrintf("\t FROM SSRC  ==> %u\n", piGetSrcDescription->GetSSRC());

        unsigned char uchFieldBuffer[MAX_ENTRYSIZE];
        unsigned long ulChangeMask = piGetSrcDescription->GetChanges();
        unsigned long ulFieldID;

        osPrintf("\t\t******************************************\n");

        while(ulChangeMask)
        {
            ulChangeMask = piGetSrcDescription->GetFieldChange(ulChangeMask,
                                                  &ulFieldID, uchFieldBuffer);
            osPrintf("\t\tSDES REPORT ==>  TYPE %s ** CONTENT %s\n",
                               suchSDESFieldNames[ulFieldID], uchFieldBuffer);
        }
        osPrintf("\t\t******************************************\n\n");

    }
#endif /* RTCP_DEBUG ] */

}

/**
 *
 * Method Name:  UpdatedSDES()
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *                              - Interface to the new Source Description info
 *           unsigned long       ulChangeMask
 *                                - The SDES fields that were subject to change
 *           IRTCPConnection    *piRTCPConnection
 *                                - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                                - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The UpdateSDES() event method shall inform the RTCP Session of
 *              a new Source Description and shall include the SSRC ID and the
 *              IGetSrcDescription interface for accessing the contents of this
 *              new Source Description.  The RTCP Session shall forward this
 *              SDES Report to other RTCP Connections in the session while
 *              acting as a Mixer.  It shall also forward the event to the RTC
 *              Manager for distribution to other subscribers.
 *
 * Usage Notes:
 *
 */
void CRTCPSession::UpdatedSDES(IGetSrcDescription *piGetSrcDescription,
                               unsigned long       ulChangeMask,
                               IRTCPConnection    *piRTCPConnection,
                               IRTCPSession       *piRTCPSession)
{

    // Now forward this event to the RTC Manager so that it can be dispatched
    //  to subscribers with matching interests
    ((IRTCPSession *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    m_piRTCPNotify->UpdatedSDES(piGetSrcDescription,
                        ulChangeMask, piRTCPConnection, (IRTCPSession *)this);

#if RTCP_DEBUG /* [ */
    if(bPingtelDebug)
    {
        osPrintf("*** SDES UPDATE RECEIVED ****\n");
        osPrintf("\t ON SESSION ==> %d\n", GetSessionID());
        osPrintf("\t FROM SSRC  ==> %u\n", piGetSrcDescription->GetSSRC());

        unsigned char uchFieldBuffer[MAX_ENTRYSIZE];
        unsigned long ulFieldID;

        if(!ulChangeMask)
        {
            osPrintf("\t\tNO CHANGES PRESENT IN THIS SDES REPORT\n\n");
            return;
        }

        osPrintf("\t\t******************************************\n");

        while(ulChangeMask)
        {
            ulChangeMask = piGetSrcDescription->GetFieldChange(ulChangeMask,
                                                  &ulFieldID, uchFieldBuffer);
            osPrintf("\t\tSDES REPORT ==>  TYPE %s ** CONTENT %s\n",
                               suchSDESFieldNames[ulFieldID], uchFieldBuffer);
        }
        osPrintf("\t\t******************************************\n\n");

    }
#endif /* RTCP_DEBUG ] */
}


/**
 *
 * Method Name:  SenderReportReceived()
 *
 *
 * Inputs:   IGetSenderStatistics *piGetSenderStatistics
 *                                   - Interface to the Sender Statistics
 *           IRTCPConnection      *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession         *piRTCPSession
 *                                   - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The SenderReportReceived() event method shall inform the
 *              recipient of a change in Sender Statistics and shall include
 *              the IGetSenderStatistics interface for accessing the contents
 *              of this updated Sender Report. This event shall be forwarded
 *              to the RTC Manager for distribution to other subscribers.
 *
 * Usage Notes:
 *
 */
void CRTCPSession::SenderReportReceived(
                           IGetSenderStatistics *piGetSenderStatistics,
                           IRTCPConnection       *piRTCPConnection,
                           IRTCPSession          *piRTCPSession)
{

    // Now forward this event to the RTC Manager so that it can be dispatched
    //  to subscribers with matching interests
    ((IRTCPSession *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    m_piRTCPNotify->SenderReportReceived(piGetSenderStatistics,
                                      piRTCPConnection, (IRTCPSession *)this);

#if RTCP_DEBUG /* [ */
    if(bPingtelDebug)
    {
        unsigned long ulPacketCount, ulOctetCount;

        osPrintf("*** SENDER REPORT RECEIVED ****\n");
        osPrintf("\t ON SESSION ==> %d\n", GetSessionID());
        osPrintf("\t FROM SSRC  ==> %u\n", piGetSenderStatistics->GetSSRC());

        piGetSenderStatistics->GetSenderStatistics(&ulPacketCount,
                                                               &ulOctetCount);
        osPrintf("\t\t******************************************\n");
        osPrintf("\t\tPACKET COUNT IS ==> %u\n",  ulPacketCount);
        osPrintf("\t\tPACKET OCTET IS ==> %u\n",  ulOctetCount);
        osPrintf("\t\t******************************************\n\n");
    }
#endif /* RTCP_DEBUG ] */
}


/**
 *
 * Method Name:  ReceiverReportReceived()
 *
 *
 * Inputs:   IGetReceiverStatistics *piGetReceiverStatistics
 *                                    - Interface to the Receiver Statistics
 *           IRTCPConnection        *piRTCPConnection
 *                                    - Interface to associated RTCP Connection
 *           IRTCPSession           *piRTCPSession
 *                                    - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ReceiverReportReceived() event method shall inform the
 *              recipient of a change in Receiver Statistics and shall include
 *              the IGetReceiverStatistics interface for accessing the contents
 *              of this updated Receiver Report. This event shall be forwarded
 *              to the RTC Manager for distribution to other subscribers.
 *
 * Usage Notes:
 *
 */
void CRTCPSession::ReceiverReportReceived(
                              IGetReceiverStatistics *piGetReceiverStatistics,
                              IRTCPConnection        *piRTCPConnection,
                              IRTCPSession           *piRTCPSession)
{

    // Now forward this event to the RTC Manager so that it can be dispatched
    //  to subscribers with matching interests
    ((IRTCPSession *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    m_piRTCPNotify->ReceiverReportReceived(piGetReceiverStatistics,
                                      piRTCPConnection, (IRTCPSession *)this);

#if RTCP_DEBUG /* [ */
    if(bPingtelDebug)
    {
        unsigned long ulFractionalLoss, ulCumulativeLoss, ulHighestSequenceNo,
                      ulInterarrivalJitter, ulSRTimestamp, ulPacketDelay;

        osPrintf("*** RECEIVED REPORT RECEIVED ****\n");
        osPrintf("\t ON SESSION ==> %d\n", GetSessionID());
        osPrintf("\t FROM SSRC  ==> %u\n", piGetReceiverStatistics->GetSSRC());

        piGetReceiverStatistics->GetReceiverStatistics(&ulFractionalLoss,
                                                       &ulCumulativeLoss,
                                                       &ulHighestSequenceNo,
                                                       &ulInterarrivalJitter,
                                                       &ulSRTimestamp,
                                                       &ulPacketDelay);

        osPrintf("\t\t******************************************\n");
        osPrintf("\t\tFRACTIONAL LOSS IS     ==> %u\n",  ulFractionalLoss);
        osPrintf("\t\tCUMULATIVE LOSS IS     ==> %u\n",  ulCumulativeLoss);
        osPrintf("\t\tHIGH SEQUENCE # IS     ==> %u\n",  ulHighestSequenceNo);
        osPrintf("\t\tINTERARRIVAL JITTER IS ==> %u\n",  ulInterarrivalJitter);
        osPrintf("\t\tLAST SR TIMESTAMP IS   ==> %u\n",  ulSRTimestamp);
        osPrintf("\t\tREPORT DELAY           ==> %u\n",  ulPacketDelay);
        osPrintf("\t\t******************************************\n\n");
    }
#endif /* RTCP_DEBUG ] */

}


/**
 *
 * Method Name:  ByeReportReceived()
 *
 *
 * Inputs:   IGetByeInfo     *piGetByeInfo
 *                         - Interface used to retrieve Bye Report information
 *           IRTCPConnection *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession    *piRTCPSession
 *                                      - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ByeReportReceived() event method shall inform the
 *              recipient of the discontinuation of an SSRC.  This may result
 *              from an SSRC collision or the termination of an associated
 *              RTP connection.  The RTCP Session shall forward this Bye
 *              Report to other RTCP Connections in the session while acting
 *              as a Mixer.  It shall also forward the event to the RTC
 *              Manager for distribution to other subscribers.
 *
 * Usage Notes:
 *
 */
void CRTCPSession::ByeReportReceived(IGetByeInfo     *piGetByeInfo,
                                     IRTCPConnection *piRTCPConnection,
                                     IRTCPSession    *piRTCPSession)
{

    // Now forward this event to the RTC Manager so that it can be dispatched
    //  to subscribers with matching interests
    ((IRTCPSession *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    m_piRTCPNotify->ByeReportReceived(piGetByeInfo,
                                      piRTCPConnection, (IRTCPSession *)this);

#if RTCP_DEBUG /* [ */
    if(bPingtelDebug)
    {
        osPrintf("*** BYE REPORT RECEIVED ****\n");
        osPrintf("\t ON SESSION ==> %d\n", GetSessionID());
        osPrintf("\t FROM SSRC  ==> %u\n", piGetByeInfo->GetSSRC());

        unsigned char uchReason[MAX_ENTRYSIZE];

        osPrintf("\t\t******************************************\n");
        unsigned long aulCSRCs[32];
        piGetByeInfo->GetReason(uchReason);
        osPrintf("\t\tBYE REPORT ==>  REASON IS %s\n", uchReason);

        unsigned long ulCSRCs = piGetByeInfo->GetCSRC(aulCSRCs);
        if(ulCSRCs == 0)
            osPrintf("\t\tBYE REPORT ==>  NO AFFECTED CSRCS\n");
        else
        {
            osPrintf("\t\tBYE REPORT ==>  AFFECTED CSRCS ARE ");
            for(unsigned long ulCount; ulCount < ulCSRCs; ulCount++)
                osPrintf("%u ", aulCSRCs[ulCount]);
            osPrintf("\n");
        }
        osPrintf("\t\t******************************************\n");
    }
#endif /* RTCP_DEBUG ] */

}

/**
 *
 * Method Name:  GetSSRC()
 *
 *
 * Inputs:       integer triple, identifying the stream
 *
 * Outputs:      None
 *
 * Returns:      ssrc_t - Local SSRC associated with the session
 *
 * Description:  Retrieves the SSRC associated with a session.
 *
 * Usage Notes:
 *
 */


/**
 *
 * Method Name:  SDESReportSent()
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *                                 - Interface to the local Source Description
 *           IRTCPConnection    *piRTCPConnection
 *                                 - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                                 - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The SDESReportSent() event method shall inform the recipient
 *              of a newly transmitted SDES Report and shall include the
 *              IGetSrcDescription interface for accessing the contents of
 *              this transmitted SDES Report.
 *
 * Usage Notes:
 *
 */
void CRTCPSession::SDESReportSent(IGetSrcDescription *piGetSrcDescription,
                                  IRTCPConnection    *piRTCPConnection,
                                  IRTCPSession       *piRTCPSession)
{

    // Forward this event to the RTC Manager so that it can be dispatched to
    //  subscribers with matching interests
    ((IRTCPSession *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    m_piRTCPNotify->SDESReportSent(piGetSrcDescription,
                                      piRTCPConnection, (IRTCPSession *)this);

#if RTCP_DEBUG /* [ */
    if(bPingtelDebug)
    {
        osPrintf("*** SDES REPORT SENT ****\n");
        osPrintf("\t ON SESSION ==> %d\n", GetSessionID());
        osPrintf("\t TO SSRC    ==> %u\n", piRTCPConnection->GetRemoteSSRC());

        unsigned char uchFieldBuffer[MAX_ENTRYSIZE];
        unsigned long ulFieldID;
        unsigned long ulChangeMask = piGetSrcDescription->GetChanges();

        osPrintf("\t\t******************************************\n");

        while(ulChangeMask)
        {
            ulChangeMask = piGetSrcDescription->GetFieldChange(ulChangeMask,
                                                  &ulFieldID, uchFieldBuffer);
            osPrintf("\t\tSDES REPORT ==>  TYPE %s ** CONTENT %s\n",
                               suchSDESFieldNames[ulFieldID], uchFieldBuffer);
        }
        osPrintf("\t\t******************************************\n\n");

    }
#endif /* RTCP_DEBUG ] */
}

/**
 *
 * Method Name:  SenderReportSent()
 *
 *
 * Inputs:   IGetSenderStatistics *piGetSenderStatistics
 *                                   - Interface to the Sender Statistics
 *           IRTCPConnection      *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession         *piRTCPSession
 *                                   - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The SenderReportSent() event method shall inform the recipient
 *              of a newly transmitted Sender Report and shall include the
 *              IGetSenderStatistics interface for accessing the contents of
 *              this transmitted Sender Report.
 *
 * Usage Notes:
 *
 */
void CRTCPSession::SenderReportSent(
                                  IGetSenderStatistics *piGetSenderStatistics,
                                  IRTCPConnection      *piRTCPConnection,
                                  IRTCPSession         *piRTCPSession)
{

    // Forward this event to the RTC Manager so that it can be dispatched
    //  to subscribers with matching interests
    ((IRTCPSession *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    m_piRTCPNotify->SenderReportSent(piGetSenderStatistics,
                                      piRTCPConnection, (IRTCPSession *)this);

#if RTCP_DEBUG /* [ */
    if(bPingtelDebug)
    {
        unsigned long ulPacketCount, ulOctetCount;

        osPrintf("*** SENDER REPORT SENT ****\n");
        osPrintf("\t ON SESSION ==> %d\n", GetSessionID());
        osPrintf("\t TO SSRC    ==> %u\n", piRTCPConnection->GetRemoteSSRC());

        piGetSenderStatistics->GetSenderStatistics(&ulPacketCount,
                                                               &ulOctetCount);
        osPrintf("\t\t******************************************\n");
        osPrintf("\t\tPACKET COUNT IS ==> %u\n",  ulPacketCount);
        osPrintf("\t\tPACKET OCTET IS ==> %u\n",  ulOctetCount);
        osPrintf("\t\t******************************************\n\n");

    }
#endif /* RTCP_DEBUG ] */
}


/**
 *
 * Method Name:  ReceiverReportSent()
 *
 *
 * Inputs:   IGetReceiverStatistics *piGetReceiverStatistics
 *                                  - Interface to the Receiver Statistics
 *           IRTCPConnection    *piRTCPConnection
 *                                  - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                                  - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ReceiverReportSent() event method shall inform the
 *              recipient of a newly transmitted Receiver Report and shall
 *              include the IGetReceiverStatistics interface for accessing
 *              the contents of this transmitted Receiver Report.
 *
 * Usage Notes:
 *
 */
void CRTCPSession::ReceiverReportSent(
                              IGetReceiverStatistics *piGetReceiverStatistics,
                              IRTCPConnection        *piRTCPConnection,
                              IRTCPSession           *piRTCPSession)
{

    // Forward this event to the RTC Manager so that it can be dispatched
    //  to subscribers with matching interests
    ((IRTCPSession *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    m_piRTCPNotify->ReceiverReportSent(piGetReceiverStatistics,
                                      piRTCPConnection, (IRTCPSession *)this);

#if RTCP_DEBUG /* [ */
    if(bPingtelDebug)
    {
        unsigned long ulFractionalLoss, ulCumulativeLoss, ulHighestSequenceNo,
                      ulInterarrivalJitter, ulSRTimestamp, ulPacketDelay;

        osPrintf("*** RECEIVED REPORT SENT ****\n");
        osPrintf("\t ON SESSION ==> %d\n", GetSessionID());
        osPrintf("\t TO SSRC    ==> %u\n", piRTCPConnection->GetRemoteSSRC());

        piGetReceiverStatistics->GetReceiverStatistics(&ulFractionalLoss,
                                                       &ulCumulativeLoss,
                                                       &ulHighestSequenceNo,
                                                       &ulInterarrivalJitter,
                                                       &ulSRTimestamp,
                                                       &ulPacketDelay);

        osPrintf("\t\t******************************************\n");
        osPrintf("\t\tFRACTIONAL LOSS IS     ==> %u\n",  ulFractionalLoss);
        osPrintf("\t\tCUMULATIVE LOSS IS     ==> %u\n",  ulCumulativeLoss);
        osPrintf("\t\tHIGH SEQUENCE # IS     ==> %u\n",  ulHighestSequenceNo);
        osPrintf("\t\tINTERARRIVAL JITTER IS ==> %u\n",  ulInterarrivalJitter);
        osPrintf("\t\tLAST SR TIMESTAMP IS   ==> %u\n",  ulSRTimestamp);
        osPrintf("\t\tREPORT DELAY           ==> %u\n",  ulPacketDelay);
        osPrintf("\t\t******************************************\n\n");
    }
#endif /* RTCP_DEBUG ] */
}


/**
 *
 * Method Name:  ByeReportSent()
 *
 *
 * Inputs:   IGetByeInfo     *piGetByeInfo
 *                         - Interface used to retrieve Bye Report information
 *           IRTCPConnection *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession    *piRTCPSession
 *                                   - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ByeReportSent() event method shall inform the recipient of
 *              a newly transmitted BYE Report and shall include the SSRC ID
 *              and reason to identify the connection and the cause of
 *              termination.
 *
 * Usage Notes:
 *
 */
void CRTCPSession::ByeReportSent(IGetByeInfo     *piGetByeInfo,
                                 IRTCPConnection *piRTCPConnection,
                                 IRTCPSession    *piRTCPSession)
{

    // Forward this event to the RTC Manager so that it can be dispatched
    //  to subscribers with matching interests
    ((IRTCPSession *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    m_piRTCPNotify->ByeReportSent(piGetByeInfo,
                                      piRTCPConnection, (IRTCPSession *)this);

#if RTCP_DEBUG /* [ */
    if(bPingtelDebug)
    {
        osPrintf("*** BYE REPORT SENT ****\n");
        osPrintf("\t ON SESSION ==> %d\n", GetSessionID());
        osPrintf("\t TO SSRC  ==> %u\n\n", piRTCPConnection->GetRemoteSSRC());

        unsigned char uchReason[MAX_ENTRYSIZE];

        osPrintf("\t\t******************************************\n");
        unsigned long aulCSRCs[32];
        piGetByeInfo->GetReason(uchReason);
        osPrintf("\t\tBYE REPORT ==>  REASON IS %s\n", uchReason);

        unsigned long ulCSRCs = piGetByeInfo->GetCSRC(aulCSRCs);
        if(ulCSRCs == 0)
            osPrintf("\t\tBYE REPORT ==>  NO AFFECTED CSRCS\n");
        else
        {
            osPrintf("\t\tBYE REPORT ==>  AFFECTED CSRCS ARE ");
            for(unsigned long ulCount; ulCount < ulCSRCs; ulCount++)
                osPrintf("%u ", aulCSRCs[ulCount]);
            osPrintf("\n");
        }
        osPrintf("\t\t******************************************\n");

    }
#endif /* RTCP_DEBUG ] */

}

/**
 *
 * Method Name:  RTCPReportingAlarm()
 *
 *
 * Inputs:   IRTCPConnection *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession    *piRTCPSession
 *                                   - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ReportAlarm() event method shall inform the recipient of
 *              the expiration of reporting period.  This event usually causes
 *              RTCP Reports to be sent out on the associated session.
 *
 * Usage Notes:
 *
 */
void CRTCPSession::RTCPReportingAlarm(IRTCPConnection     *piRTCPConnection,
                                      IRTCPSession        *piRTCPSession)
{

    // OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPSession::RTCPReportingAlarm");
    // Send the event with the corresponding info.
    ((IRTCPSession *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    m_piRTCPNotify->RTCPReportingAlarm(piRTCPConnection, (IRTCPSession *)this);

#if RTCP_DEBUG /* [ */
    if(bPingtelDebug)
    {
        osPrintf("*** RTCP REPORTING ALARM ****\n");
        osPrintf("\t ON SESSION ==> %d\n", GetSessionID());
        osPrintf("\t TO SSRC  ==> %u\n\n", piRTCPConnection->GetRemoteSSRC());
    }
#endif /* RTCP_DEBUG ] */
}

/**
 *
 * Method Name:  RTCPConnectionStopped()
 *
 *
 * Inputs:   IRTCPConnection *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession    *piRTCPSession
 *                                   - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The RTCPConnectionStopped() event method shall inform the
 *              recipient of the imminent suspension of an RTCP outbound
 *              connection.  This will allow whatever connection related
 *              operations to be suspended until it again resumes.
 *
 * Usage Notes:
 *
 */
void CRTCPSession::RTCPConnectionStopped(IRTCPConnection *piRTCPConnection,
                                         IRTCPSession    *piRTCPSession)
{
    ssrc_t aulCSRC[MAX_CONNECTIONS];
    unsigned long ulCSRCs = 0;
    CRTCPConnection *poRTCPConnection;

    poRTCPConnection = GetEntry(RTCPConnectionComparitor, (void *)piRTCPConnection);

    // Get the associated RTCP Connection object from the collection list
    if(poRTCPConnection != NULL)
    {
#if RTCP_DEBUG /* [ */
        if(bPingtelDebug)
        {
            osPrintf("*** RTCP CONNECTION STOPPED ****\n");
            osPrintf("\t ON SESSION ==> %d\n", GetSessionID());
            osPrintf("\t TO SSRC    ==> %u\n",
                                           piRTCPConnection->GetRemoteSSRC());
        }
#endif /* RTCP_DEBUG ] */

        // Check whether the session is acting as an Audio Mixer.  If so,
        // We should include these sites into our contributing source list.
        if(m_etMixerMode == MIXER_ENABLED)
        {
            // Check each entry of the connection list
            TakeLock();
            CRTCPConnection *poPeerConnection = GetFirstEntry();

            // Iterate through the list until all entries have been exhausted
            for(ulCSRCs = 0; poPeerConnection != NULL; ulCSRCs++)
            {
                // Bump Reference Count of Connection Object
                poPeerConnection->AddRef(ADD_RELEASE_CALL_ARGS(-__LINE__));

                // Get the SSRC ID of the connection
                aulCSRC[ulCSRCs] = poPeerConnection->GetRemoteSSRC();

                // Check that we are not adding the Remote SSRC of the
                //  connection being terminated.  If so, we will backup the
                //  CSRC count
                if(poPeerConnection->GetRemoteSSRC() == aulCSRC[ulCSRCs])
                    ulCSRCs--;

                // Release Reference to Connection Object
                poPeerConnection->Release(ADD_RELEASE_CALL_ARGS(-__LINE__));

                // Get the next connection from the list
                poPeerConnection = GetNextEntry();
            }
            ReleaseLock();
        }

        // Bump Reference Count of Connection Object
        poRTCPConnection->AddRef(ADD_RELEASE_CALL_ARGS(-__LINE__));

        poRTCPConnection->GenerateRTCPReports(
          (unsigned char *)"Normal Connection Termination", aulCSRC, ulCSRCs);

        // Release Reference to Connection Object
        poRTCPConnection->Release(ADD_RELEASE_CALL_ARGS(-__LINE__));

    }

}

#endif /* INCLUDE_RTCP ] */
