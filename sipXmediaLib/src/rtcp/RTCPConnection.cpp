//
// Copyright (C) 2006-2013 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// Includes
#include "rtcp/RTCPConnection.h"
#ifdef INCLUDE_RTCP /* [ */

#include "os/OsSysLog.h"

// Constants
#define DEBUGGING_RTCP_REPORTS
#ifdef DEBUGGING_RTCP_REPORTS /* [ */
#ifdef _VXWORKS /* [ */
static int REPORT_PERIOD_MS =   5000;   // 5 Seconds
extern "C" {extern int adjustRtcpPeriod(int x);};

int adjustRtcpPeriod(int newPeriod) {
   int save = REPORT_PERIOD_MS;
   if (newPeriod > 50) {
      REPORT_PERIOD_MS = newPeriod;
   }
   return save;
}
#else /* _VXWORKS ] [ */
const int REPORT_PERIOD_MS =   5000;   // 5 Seconds
#endif /* _VXWORKS ] */
#else /* DEBUGGING_RTCP_REPORTS ] [ */
const int REPORT_PERIOD_MS =   5000;   // 5 Seconds
#endif /* DEBUGGING_RTCP_REPORTS ] */

/**
 *
 * Method Name:  CRTCPConnection() - Constructor
 *
 *
 * Inputs:   unsigned long ulSSRC       - SSRC ID
 *           IRTCPNotify *piRTCPNotify  - RTCP Event Notification Interface
 *           ISDESReport *piSDESReport  - Local Source Description Interface
 *
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description:  Allocates the RTCP Source and Render components of an RTCP
 *               Connection.
 *
 * Usage Notes:  Allocation is not normally performed at construction time but
 *               a flag shall be set that will be used during the
 *               initialization phase to determine whether all went well.  This
 *               is being done so construction arguments do not have to be
 *               stored as attributes of the RTCP Session object.
 *
 */
CRTCPConnection::CRTCPConnection(unsigned long ulSSRC,
    IRTCPNotify *piRTCPNotify, ISDESReport *piSDESReport) :
        CBaseClass(CBASECLASS_CALL_ARGS("CRTCPConnection", __LINE__)),
        CRTCPTimer(REPORT_PERIOD_MS),
        m_piRTCPNetworkRender(NULL),
        m_poRTCPRender(NULL),
        m_poRTCPSource(NULL)
{
    OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPConnection::CRTCPConnection() -> %p, SSRC=0x%lx", this, ulSSRC);

    ////////////////////////////////////////////////////////////////////////////
    // HACK:  Adding this to compensate for a missing AddRef()... somewhere...
    //   that results in this being deleted one Release() too soon.
    AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    ////////////////////////////////////////////////////////////////////////////

    // Cache SSRC
    m_ulSSRC = ulSSRC;

    // Cache SDES Reporting Interface
    if(piSDESReport)
    {
        m_piSDESReport = piSDESReport;
        m_piSDESReport->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    }

    // Store Notification Interface
    m_piRTCPNotify = piRTCPNotify;
    if(m_piRTCPNotify)
    {
        m_ulEventInterest = m_piRTCPNotify->GetEventInterest();
        m_piRTCPNotify->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    }


}

/**
 *
 * Method Name: ~CRTCPConnection() - Destructor
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
 *                ==> Shall release the reference to the RTCP Render object
 *                ==> Shall release the reference to the RTCP Source object
 *
 * Usage Notes:
 *
 *
 */
CRTCPConnection::~CRTCPConnection(void)
{

    int c1, c2;
    c1 = c2 = -1;
    if (m_poRTCPRender) c1 = m_poRTCPRender->m_ulReferences;
    if (m_poRTCPSource) c2 = m_poRTCPSource->m_ulReferences;
    OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPConnection::~CRTCPConnection(%p): ref counts: %d, %d", this, c1, c2);
    // Terminate Connection and release any remaining references to interfaces
    //  and objects
    Terminate();
    m_piRTCPNotify->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
    m_piSDESReport->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
    // m_poRTCPRender->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
    // m_poRTCPSource->Release(ADD_RELEASE_CALL_ARGS(__LINE__));

}

/**
 *
 * Method Name:  Initialize
 *
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      bool
 *
 * Description:  Performs runtime initialization that may be failure prone and
 *               therefore inappropriate for execution within a constructor.
 *               This includes the verification of construction time
 *               initialization as well as the initialization of the RTCP
 *               Timer object.
 *
 * Usage Notes:  This is an overload of the CBaseClass Initialize() method.
 *
 */
bool CRTCPConnection::Initialize(void)
{
    ISetReceiverStatistics *piReceiverStats=NULL;
    bool                    bInitialized = FALSE;

    // Create The RTCP Render object
    m_poRTCPRender = new CRTCPRender(m_ulSSRC, (IRTCPNotify *)this, m_piSDESReport);
    if (m_poRTCPRender == NULL)
    {
        OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPConnection::Initialize - Unable to Create CRTCPRender object");
        return(bInitialized);
    }

    // Initialize RTCP Render object
    else if(m_poRTCPRender->Initialize())
    {
        // Retrieve the Receiver Report Statistical Interface so that we might
        //  use it to initialize the RTCP Source object.
        m_poRTCPRender->GetReceiveStatInterface(&piReceiverStats);
    }
    else
    {
        // Release Transient references
        OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPConnection::Initialize - Unable to Initialize CRTCPRender object");
        m_poRTCPRender->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        return(bInitialized);
    }

    // Create The RTCP Source object
    m_poRTCPSource = new CRTCPSource(m_ulSSRC, (IRTCPNotify *)this, piReceiverStats);
    if (m_poRTCPSource == NULL)
    {
        OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPConnection::Initialize - Unable to Create CRTCPSource object");
        m_poRTCPRender->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        return(bInitialized);
    }

    // Initialize RTCP Source object.  If initialization succeeds, set the
    //  initialized flag for the connection object to True.
    else if(m_poRTCPSource->Initialize())
    {
       bInitialized = TRUE;
    }

    // Initialization Failure.  Release all resource created and return an
    //  indication of the failure.
    else
    {
        OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPConnection::Initialize - Unable to Initialize CRTCPSource object");
        m_poRTCPRender->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        m_poRTCPSource->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
    }
    OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPConnection::Initialize: this=%p, m_poRTCPRender=%p, m_poRTCPSource=%p", this, m_poRTCPRender, m_poRTCPSource);


    // Release Transient reference
    piReceiverStats->Release(ADD_RELEASE_CALL_ARGS(__LINE__));

    return(bInitialized);

}



/**
 *
 * Method Name:  GetDispatchInterfaces()
 *
 *
 * Inputs:   None
 *
 * Outputs:  INetDispatch         **ppiNetDispatch
 *                                     - Interface used to route Network
 *                                       packets to the RTCP Source object
 *           IRTPDispatch         **ppRTPDispatch
 *                                     - Interface used to route RTP Header
 *                                       packets to the RTPRender object's
 *                                       instance of an RTCP Receiver Report
 *           ISetSenderStatistics **ppiSenderStats
 *                                     -  Statistical Dispatch Interface
 *
 * Returns:  void
 *
 * Description: This method allow a consumer to obtain the dispatch interfaces
 *              that are crucial to routing inbound RTCP and RTP network
 *              packets.
 *
 *
 * Usage Notes:
 *
 */
void CRTCPConnection::GetDispatchInterfaces(INetDispatch **ppiNetDispatch,
                                        IRTPDispatch **ppiRTPDispatch,
                                        ISetSenderStatistics **ppiSenderStats)
{

    // Assign the Network Dispatch interface to the corresponding RTCP Source object
    if (m_poRTCPSource) {
        *ppiNetDispatch = (INetDispatch *)m_poRTCPSource;
    } else {
        *ppiNetDispatch = NULL;
    }

    // Assign the RTP Dispatch interface to the Receiver Report associated
    //  with the RTCP Render object
    if (m_poRTCPRender)
    {
        *ppiRTPDispatch = (IRTPDispatch *)m_poRTCPRender;

        // Retrieve the Sender Report Statistical Interface
        m_poRTCPRender->GetSenderStatInterface(ppiSenderStats);
    } else {
        *ppiRTPDispatch = NULL;
        *ppiSenderStats = NULL;
    }
}

/**
 *
 * Method Name:  GetRenderInterface()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     IRTCPRender *   - RTCP Render Interface
 *
 * Description: This method allow a consumer to gain access to the RTCP Render
 *              interface associated with a specific RTCP Connection.
 *
 *
 * Usage Notes:
 *
 */
IRTCPRender *CRTCPConnection::GetRenderInterface(void)
{

    if(m_poRTCPRender)
        ((IRTCPRender *)m_poRTCPRender)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));

    return((IRTCPRender *)m_poRTCPRender);

}

/**
 *
 * Method Name:  StartRenderer()
 *
 *
 * Inputs:      OsSocket& rRtcpSocket   - Reference to an RTCP Socket object
 *
 * Outputs:     None
 *
 * Returns:     bool                    - Result of Starting the Renderer
 *
 * Description: This method starts the RTCP Renderer allowing it to generate
 *              RTCP reports on regular intervals.
 *
 *
 * Usage Notes:
 *
 */
#ifdef PINGTEL_OSSOCKET
bool CRTCPConnection::StartRenderer(OsSocket& rRtcpSocket)
#else
bool CRTCPConnection::StartRenderer(INetworkRender *piRTCPNetworkRender)
#endif
{

#ifdef PINGTEL_OSSOCKET
    // OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPConnection::StartRenderer(OsSocket& %p)", &rRtcpSocket);
    // Create a wrapper object for the OSSocket reference passed.  This allows
    //  us to use this resource without changing the internals of the code.
    m_piRTCPNetworkRender = (INetworkRender *)new CNetworkChannel(rRtcpSocket);
    if (!(m_piRTCPNetworkRender))
    {
        // Identify source of Error
        OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPConnection::StartRenderer - Unable to Create Net Render Object");
        return(FALSE);
    }
#else
    // OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPConnection::StartRenderer(INetworkRender *%p)", piRTCPNetworkRender);
    // Load the Netwok Render object internally and bump the reference count
    m_piRTCPNetworkRender = piRTCPNetworkRender;
    m_piRTCPNetworkRender->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
#endif

    // Let's load up the RTCP Render Object with the Network Interface
    m_poRTCPRender->SetNetworkRender(m_piRTCPNetworkRender);

    // Initialize RTCP Timer object
    if(!CRTCPTimer::Initialize())
    {
        OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPConnection::StartRenderer - Unable to Start Connection Report Timer");
        // Let's Clear and release the Renderer in light of the recent failure
        m_poRTCPRender->ClearNetworkRender();
        m_piRTCPNetworkRender->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        m_piRTCPNetworkRender = NULL;
        return(FALSE);
    }

    // Send Notification telling the Parent Session that
    //  it has started/re-started
    if(m_piRTCPNotify)
        m_piRTCPNotify->RTCPConnectionStarted((IRTCPConnection *)this);

    // Set the initialization flag to True
    m_bInitialized = TRUE;

    return(TRUE);

}


/**
 *
 * Method Name: GenerateRTCPReports
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: The GenerateRTCPReports() method is called when the RTCP Report
 *              Timer has expired.  This method will obtain the interface of
 *              the CRTCPRender object and instruct it to generate reports.
 *              The reports generate shall be returned and shall result in
 *              event delivery to the RTC Manager onto subscribing clients.
 *
 * Usage Notes:
 *
 *
 */
void CRTCPConnection::GenerateRTCPReports(unsigned char *puchByeReason,
                                          ssrc_t aulCSRC[],
                                          unsigned long ulCSRCs)
{
    IGetSrcDescription     *piGetSrcDescription;
    IGetSenderStatistics   *piSenderStatistics;
    IGetReceiverStatistics *piReceiverStatistics;
    IGetByeInfo            *piGetByeInfo;
    unsigned long           ulReportMask;

    // Check whether Render object is still active

    // Let's instruct the RTCP Render object to go generate some RTCP reports.
    // If a Bye Reason has been provided, we must also tack a Bye report onto
    //  the end of the other outgoing reports; otherwise, send out the normal
    //  complement of reports.
    if(puchByeReason)
        ulReportMask =
           m_poRTCPRender->GenerateByeReport(aulCSRC, ulCSRCs, puchByeReason);
    else
        ulReportMask = m_poRTCPRender->GenerateRTCPReports();

    // Let's get the interface to all the reports just generated so that we
    //  may use them in upcoming event notifications
    m_poRTCPRender->GetStatistics(&piGetSrcDescription, &piSenderStatistics,
                                  &piReceiverStatistics, &piGetByeInfo);

    // Check to see which events were generate and issue the associated
    //  notifications

    // Check for SDES Report
    if(ulReportMask & RTCP_SDES_SENT)
    {
        piGetSrcDescription->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
        ((IRTCPConnection *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
        m_piRTCPNotify->SDESReportSent(piGetSrcDescription,
                                                     (IRTCPConnection *)this);
    }

    // Check for Sender Report
    if(ulReportMask & RTCP_SR_SENT)
    {
        piSenderStatistics->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
        ((IRTCPConnection *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
        m_piRTCPNotify->SenderReportSent(piSenderStatistics,
                                                     (IRTCPConnection *)this);
    }

    // Check for Receiver Report
    if(ulReportMask & RTCP_RR_SENT)
    {
        piReceiverStatistics->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
        ((IRTCPConnection *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
        m_piRTCPNotify->ReceiverReportSent(piReceiverStatistics,
                                                     (IRTCPConnection *)this);
    }

    // Check for Bye Report
    if(ulReportMask & RTCP_BYE_SENT)
    {
        piGetByeInfo->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
        ((IRTCPConnection *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
        m_piRTCPNotify->ByeReportSent(piGetByeInfo, (IRTCPConnection *)this);
    }

    // Release references
    piGetSrcDescription->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
    piSenderStatistics->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
    piReceiverStatistics->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
    piGetByeInfo->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
}

/**
 *
 * Method Name:  StopRenderer
 *
 *
 * Inputs:       bool   bRelease
 *
 * Outputs:      None
 *
 * Returns:      bool
 *
 * Description:  Performs connection based shutdown related to the orderly
 *               deallocation of connection based filters.
 *
 * Usage Notes:
 *
 */
bool CRTCPConnection::StopRenderer(void)
{
    // OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPConnection::StopRenderer()");

    // Check that the initialization had successfully completed
    if(!m_bInitialized)
        return(FALSE);

    // Shutdown Timer
    CRTCPTimer::Shutdown();

    // Send Notification to Parent Session so that it can do something
    //  intelligent
    if(m_piRTCPNotify)
        m_piRTCPNotify->RTCPConnectionStopped((IRTCPConnection *)this);


    // Clear the Network Render interface if it has already been registered
    if(m_piRTCPNetworkRender)
    {
        m_poRTCPRender->ClearNetworkRender();
        m_piRTCPNetworkRender->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        m_piRTCPNetworkRender = NULL;
    }

    // Reset connection to uninitialized
    m_bInitialized = FALSE;

    return(TRUE);
}


/**
 *
 * Method Name:  Terminate
 *
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      bool
 *
 * Description:  Performs connection based shutdown related to the orderly
 *               deallocation of connection based filters.
 *
 * Usage Notes:  This method must be called in order for the Connection
 *               object and its consituents to be gracefully deallocated.
 *
 */
bool CRTCPConnection::Terminate()
{

    OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPConnection::Terminate(%p)", this);
    // Let's Stop the Render if it hasn't already been done
    StopRenderer();

    // Release RTCP Source object
    if(m_poRTCPSource)
    {
        // m_poRTCPSource->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__)); while (m_poRTCPSource->Release(__LINE__));
        m_poRTCPSource->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        m_poRTCPSource = NULL;
    }

    // Release RTCP Render object
    if(m_poRTCPRender)
    {
        // m_poRTCPRender->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__)); while(m_poRTCPRender->Release(__LINE__));
        m_poRTCPRender->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        m_poRTCPRender = NULL;
    }

    return(TRUE);

}

/**
 *
 * Method Name:  NewSDES()
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *                                -Interface to the new Source Description info
 *           IRTCPConnection    *piRTCPConnection
 *                                - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                                - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: Provides the corresponding RTCPConnection interface to the
 *              notification and passes it along to the subscribing Session
 *              Object.
 *
 * Usage Notes:
 *
 */
void CRTCPConnection::NewSDES(IGetSrcDescription *piGetSrcDescription,
                          IRTCPConnection    *piRTCPConnection,
                          IRTCPSession       *piRTCPSession)
{

    // Send the event with the corresponding info.
    ((IRTCPConnection *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    m_piRTCPNotify->NewSDES(piGetSrcDescription, (IRTCPConnection *)this);

}

/**
 *
 * Method Name:  UpdatedSDES()
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *                               - Interface to the new Source Description info
 *           unsigned long       ulChangeMask
 *                               - The SDES fields that were subject to change
 *           IRTCPConnection    *piRTCPConnection
 *                               - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                               - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: Provides the corresponding RTCPConnection interface to the
 *              notification and passes it along to the subscribing Session
 *              Object.
 *
 * Usage Notes:
 *
 */
void CRTCPConnection::UpdatedSDES(IGetSrcDescription *piGetSrcDescription,
                              unsigned long       ulChangeMask,
                              IRTCPConnection    *piRTCPConnection,
                              IRTCPSession       *piRTCPSession)
{

    // Send the event with the correpsonding info.
    ((IRTCPConnection *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    m_piRTCPNotify->UpdatedSDES(piGetSrcDescription,
                                       ulChangeMask, (IRTCPConnection *)this);

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
 * Description: Provides the corresponding RTCPConnection interface to the
 *              notification and passes it along to the subscribing Session
 *              Object.
 *
 * Usage Notes:
 *
 */
void CRTCPConnection::SenderReportReceived(
              IGetSenderStatistics *piGetSenderStatistics,
              IRTCPConnection *piRTCPConnection, IRTCPSession *piRTCPSession)
{

    // Send the event with the correpsonding info.
    ((IRTCPConnection *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    m_piRTCPNotify->SenderReportReceived(piGetSenderStatistics,
                                                     (IRTCPConnection *)this);

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
 * Description: Provides the corresponding RTCPConnection interface to the
 *              notification and passes it along to the subscribing Session
 *              Object.
 *
 * Usage Notes:
 *
 */
void CRTCPConnection::ReceiverReportReceived(
               IGetReceiverStatistics *piGetReceiverStatistics,
               IRTCPConnection *piRTCPConnection, IRTCPSession *piRTCPSession)
{

    // Send the event with the correpsonding info.
    ((IRTCPConnection *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    m_piRTCPNotify->ReceiverReportReceived(piGetReceiverStatistics,
                                                     (IRTCPConnection *)this);
}


/**
 *
 * Method Name:  ByeReportReceived()
 *
 *
 * Inputs:   IGetByeInfo      *piGetByeInfo
 *                                 - Interface used to retrieve Bye Report
 *                                   information
 *           IRTCPConnection  *piRTCPConnection
 *                                 - Interface to associated RTCP Connection
 *           IRTCPSession     *piRTCPSession
 *                                 - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: Provides the corresponding RTCPConnection interface to the
 *              notification and passes it along to the subscribing Session
 *              Object.
 *
 * Usage Notes:
 *
 */
void CRTCPConnection::ByeReportReceived(IGetByeInfo        *piGetByeInfo,
                                        IRTCPConnection    *piRTCPConnection,
                                        IRTCPSession       *piRTCPSession)
{

    // Send the event with the correpsonding info.
    ((IRTCPConnection *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    m_piRTCPNotify->ByeReportReceived(piGetByeInfo,  (IRTCPConnection *)this);


}


/**
 *
 * Method Name:  RTCPReportingAlarm()
 *
 *
 * Inputs:   IRTCPConnection    *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
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
 *
 * Usage Notes:
 *
 */
void CRTCPConnection::RTCPReportingAlarm(IRTCPConnection *piRTCPConnection,
                                         IRTCPSession    *piRTCPSession)
{

    // Send the event with the correpsonding info if the timer is
    //  still in force
    ((IRTCPConnection *)this)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    m_piRTCPNotify->RTCPReportingAlarm((IRTCPConnection *)this);

}
#endif /* INCLUDE_RTCP ] */
