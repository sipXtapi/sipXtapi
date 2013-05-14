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
#include "mp/MpMediaTask.h"
#include "rtcp/RTCPSource.h"
#ifdef INCLUDE_RTCP /* [ */

    // Template Comparitor for RR SSRC Match
bool RRSsrcComparitor(CReceiverReport *poReceiverReport, void *pvArgument)
{

    unsigned long ulSSRC = (unsigned long)pvArgument;

    // Check the Receiver object within thelist to determine whether the
    // SSRC's match
    if(poReceiverReport->IsOurSSRC(ulSSRC))
        return(TRUE);

    return(FALSE);

}

    // Template Comparitor for RR SSRC Match
bool SDESSsrcComparitor(CSourceDescription *poReceiverReport, void *pvArgument)
{

    unsigned long ulSSRC = (unsigned long)pvArgument;

    // Check the Receiver object within thelist to determine whether the
    // SSRC's match
    if(poReceiverReport->IsOurSSRC(ulSSRC))
        return(TRUE);

    return(FALSE);

}

/**
 *
 * Method Name:  CRTCPSource() - Constructor
 *
 *
 * Inputs:   unsigned long           ulSSRC  - The  Identifier for this source
 *           IRTCPNotify            *piRTCPNotify
 *                                      - RTCP Event Notification Interface
 *           ISetReceiverStatistics *piSetStatistics
 *                                      - Interface for setting receiver stats
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description:  Performs routine CRTCPSource object initialization include
 *               assignment of constructor arguments to internal attributes.
 *
 * Usage Notes:  It is assumed the the pointer obtained during CRTCPSource
 *               object construction shall be used in the construction or
 *               initialization of the Network Source, RTP Source, and RTCP
 *               Sender objects associated with an inbound RTP connection.
 *
 */
CRTCPSource::CRTCPSource(ssrc_t ulSSRC, IRTCPNotify *piRTCPNotify,
                         ISetReceiverStatistics *piSetStatistics)
              : CBaseClass(CBASECLASS_CALL_ARGS("CRTCPSource", __LINE__)), m_poSenderReport(NULL), m_poByeReport(NULL)
{

    // Store the SSRC ID passed
    m_ulSSRC = ulSSRC;

    // Store the Statistics interface as an attribute
    m_piSetReceiverStatistics = piSetStatistics;

    // Store RTC Event Notification Interface
    m_piRTCPNotify = piRTCPNotify;

    // Increment the reference count to the RTCP Notification and Receiver
    //  Stats Interfaces
    if(m_piRTCPNotify)
        m_piRTCPNotify->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    if(m_piSetReceiverStatistics)
        m_piSetReceiverStatistics->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));

}


/**
 *
 * Method Name: ~CRTCPSource() - Destructor
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
 *    ==> The queue containing remote RTCP Source Description objects shall be
 *        drained with the reference to each object released.
 *    ==> The reference to the remote Source Report object shall be released
 *    ==> The reference to the remote Receiver Report object shall be released
 *
 * Usage Notes: This shall override the virtual destructor in the base class
 *              so that deallocation specific to the derived class will be
 *              done despite the destruction being performed in the base class
 *              as part of the release.
 *
 */
CRTCPSource::~CRTCPSource(void)
{

    // Declarations
    ISDESReport     *piSDESReport;
    IReceiverReport *piReceiverReport;

    // Release Sender object reference if it exists
    if(m_poSenderReport)
        ((ISenderReport *)m_poSenderReport)->Release(ADD_RELEASE_CALL_ARGS(__LINE__));

    // Release Bye object reference if it exists
    if(m_poByeReport)
        ((IByeReport *)m_poByeReport)->Release(ADD_RELEASE_CALL_ARGS(__LINE__));

    // Iterate through Source Description List and
    //  release all references to objects
    m_tSrcDescriptorList.TakeLock();
    piSDESReport = (ISDESReport *)m_tSrcDescriptorList.RemoveFirstEntry();
    while(piSDESReport != NULL)
    {
        // Release Reference
        piSDESReport->Release(ADD_RELEASE_CALL_ARGS(__LINE__));

        // Get Next Entry
        piSDESReport = (ISDESReport *)m_tSrcDescriptorList.RemoveNextEntry();

    }
    m_tSrcDescriptorList.ReleaseLock();

    // Iterate through Source Description List and
    //  release all references to objects
    m_tReceiverReportList.TakeLock();
    piReceiverReport = (IReceiverReport *)m_tReceiverReportList.RemoveFirstEntry();
    while (piReceiverReport != NULL)
    {
        // Release Reference
        piReceiverReport->Release(ADD_RELEASE_CALL_ARGS(__LINE__));

        // Get Next Entry
        piReceiverReport = (IReceiverReport *)m_tReceiverReportList.RemoveNextEntry();
    }
    m_tReceiverReportList.ReleaseLock();

    // Release other stored interfaces
    if(m_piRTCPNotify)
        m_piRTCPNotify->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
    if(m_piSetReceiverStatistics)
        m_piSetReceiverStatistics->Release(ADD_RELEASE_CALL_ARGS(__LINE__));

}


/**
 *
 * Method Name: ProcessPacket
 *
 *
 * Inputs:   unsigned char *puchDataBuffer
 *                                  - Data Buffer received from Network Source
 *           unsigned long ulBufferLength - Length of Data Buffer
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ProcessPacket() sequentially processes data packets
 *              received by looking for for header information that describes
 *              the single or composite RTCP report sent.  The RTCP Payload
 *              type of each header found is examined.  A packet or a composite
 *              portion is discarded if it the header not contain a valid RTCP
 *              Payload Type (Types 200 - 204).  The packet or composite
 *              portion is then routed by Payload type to the associated
 *              Sender, Receiver Source Description, or Bye method for further
 *              report processing. This iterative process continues until a
 *              packet or composite has been full processed.
 *
 *
 * Usage Notes: The intention is for this method to be processed on the
 *              Execution thread of the Network Dispatcher.  This may be
 *              modified if RTCP processing is interferring with the
 *              Dispatcher's ability to receive and route high volume RTP
 *              packets.
 *
 *
 */

// #define DEBUG_RTCP_PACKETS

#ifdef DEBUG_RTCP_PACKETS /* [ */
static int numPacketsToDump = 10;
int showRtcp(int count) {
    int save = numPacketsToDump;
    numPacketsToDump = count;
    return save;
}
#endif /* DEBUG_RTCP_PACKETS ] */

void CRTCPSource::ProcessPacket(unsigned char *puchDataBuffer,
                                unsigned long ulBufferLength, int verbose)
{

    unsigned char *SAVEpuchDataBuffer = puchDataBuffer;
    unsigned long SAVEulBufferLength = ulBufferLength;

#ifdef DEBUG_RTCP_PACKETS /* [ */
    if (0 < numPacketsToDump--) {
        verbose = 1;
    }

    if (verbose) {
        unsigned char *tp = puchDataBuffer;
        unsigned long tl = ulBufferLength;
        UtlString buf;
        OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPSource::ProcessPacket: begin processing packet at %p, length=%lu", puchDataBuffer, ulBufferLength);
        while(tl > 0) {
            buf.appendFormat(" 0x%02X,", *tp++);
            tl--;
        }
        buf.strip(UtlString::trailing, ',');
        OsSysLog::add(FAC_MP, PRI_ERR, "RTCP Packet:    %s", buf.data());
    }
#endif /* DEBUG_RTCP_PACKETS ] */

    // This could be either a simple or composite report. Let's process the
    //  buffer until there is nothing more to process.
    while(ulBufferLength > 0)
    {
        unsigned long ulBytesProcessed = 0;

        // Let's peek into the RTCP header to determine the payload type of an
        //  RTCP report and route it to the appropriate handler.
        switch(GetPayloadType(puchDataBuffer))
        {
            // Process Sender Report
            case etSenderReport:
                ulBytesProcessed = ProcessSenderReport(puchDataBuffer);
#ifdef DEBUG_RTCP_PACKETS /* [ */
                if (verbose) {
                    OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPSource::ProcessPacket: Processed SR, bytes=%lu", ulBytesProcessed);
                }
#endif /* DEBUG_RTCP_PACKETS ] */
                break;

            // Process Receiver Report
            case etReceiverReport:
                ulBytesProcessed = ProcessReceiverReport(puchDataBuffer);
#ifdef DEBUG_RTCP_PACKETS /* [ */
                if (verbose) {
                    OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPSource::ProcessPacket: Processed RR, bytes=%lu", ulBytesProcessed);
                }
#endif /* DEBUG_RTCP_PACKETS ] */
                break;

            // Process Source Description Report
            case etSDESReport:
                ulBytesProcessed = ProcessSDESReport(puchDataBuffer);
#ifdef DEBUG_RTCP_PACKETS /* [ */
                if (verbose) {
                    OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPSource::ProcessPacket: Processed SDES, bytes=%lu", ulBytesProcessed);
                }
#endif /* DEBUG_RTCP_PACKETS ] */
                break;

            // Process Bye Report
            case etByeReport:
                ulBytesProcessed = ProcessByeReport(puchDataBuffer);
#ifdef DEBUG_RTCP_PACKETS /* [ */
                if (verbose) {
                    OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPSource::ProcessPacket: Processed BYE, bytes=%lu", ulBytesProcessed);
                }
#endif /* DEBUG_RTCP_PACKETS ] */
                break;

            // Process Application Report
            case etAppReport:
                ulBytesProcessed = ProcessAppReport(puchDataBuffer);
#ifdef DEBUG_RTCP_PACKETS /* [ */
                if (verbose) {
                    OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPSource::ProcessPacket: Processed APP, bytes=%lu", ulBytesProcessed);
                }
#endif /* DEBUG_RTCP_PACKETS ] */
                break;

            // Unrecognized Report
            default:
                {
                   OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPSource::ProcessPacket: Unknown report type %d", GetPayloadType(puchDataBuffer));
                   OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPSource::ProcessPacket: Remaining buffer length of %lu", ulBufferLength);
                }
                if (!verbose) {
                    ProcessPacket(SAVEpuchDataBuffer, SAVEulBufferLength, 1);
                }
                OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPSource::ProcessPacket: Returning (error)");
                return;
        }

#ifdef DEBUG_RTCP_PACKETS /* [ */
        if (verbose) {
            unsigned char *tp = puchDataBuffer;
            unsigned long tl = ulBytesProcessed;
            int i = 0;
            while(tl > 0) {
                osPrintf(" %02X", *tp++);
                if (0xf == (0xf & i++)) osPrintf("\n");
                tl--;
            }
            if (0 != (0xf & i)) osPrintf("\n");
        }
#endif /* DEBUG_RTCP_PACKETS ] */
        // Adjust length remaining and the buffer pointer so that we are
        // prepared to recognize and process other reports.
        puchDataBuffer  += ulBytesProcessed;
        ulBufferLength -= ulBytesProcessed;

    }
#ifdef DEBUG_RTCP_PACKETS /* [ */
    if (verbose) {
        OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPSource::ProcessPacket: Returning, remaining length=%lu", ulBufferLength);
    }
#endif /* DEBUG_RTCP_PACKETS ] */
}

/**
 *
 * Method Name: SendRTCPEvent
 *
 *
 * Inputs:   ulong ulEventType    - Event Type
 *           void  *pvInterface   - Opaque pointer to an associated interface
 *           ulong ulChangeMask   - Optional Change Mask
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: A centralized processing method that will dispatch events to
 *              all subscribing parties based on their registered interest.
 *
 *
 * Usage Notes: This centralized event generation mechanism is useful in
 *              dispatching events to multiple subscribers.  This avoid the
 *              duplication of code in each method that might cause an event
 *              to be generated.
 *
 *
 */
void CRTCPSource::SendRTCPEvent(unsigned long ulEventType,
                                void *pvInterface, unsigned long ulChangeMask)
{

    // Check whether event type is of interest to subscriber(s)
    if(m_piRTCPNotify->GetEventInterest() && ulEventType == 0)
        return;

    // We have a subscriber that is interested in this event.  Let's figure
    //  out the event type so that we can cast the interface pointer correctly.
    switch(ulEventType)
    {
        case RTCP_NEW_SDES:
        case (RTCP_NEW_SDES+RTCP_SDES_UPDATE):
        {
            CSourceDescription *poSDESReport =
                                            (CSourceDescription *)pvInterface;
            ((IGetSrcDescription *)poSDESReport)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
            m_piRTCPNotify->NewSDES((IGetSrcDescription *)poSDESReport);
            break;
        }

        case RTCP_SDES_UPDATE:
        {
            CSourceDescription *poSDESReport =
                                            (CSourceDescription *)pvInterface;
            ((IGetSrcDescription *)poSDESReport)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
            m_piRTCPNotify->UpdatedSDES(
                            (IGetSrcDescription *)poSDESReport, ulChangeMask);
            break;
        }

        case RTCP_RR_RCVD:
        {
            CReceiverReport *poReceiverReport = (CReceiverReport *)pvInterface;
            ((IGetReceiverStatistics *)poReceiverReport)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
            m_piRTCPNotify->ReceiverReportReceived(
                                  (IGetReceiverStatistics *)poReceiverReport);
            break;
        }

        case RTCP_SR_RCVD:
        {
            CSenderReport *poSenderReport = (CSenderReport *)pvInterface;
            ((IGetSenderStatistics *)poSenderReport)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
            m_piRTCPNotify->SenderReportReceived(
                                      (IGetSenderStatistics *)poSenderReport);
            break;
        }

        case RTCP_BYE_RCVD:
        {
            CByeReport *poByeReport = (CByeReport *)pvInterface;
            ((IGetByeInfo *)poByeReport)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
            m_piRTCPNotify->ByeReportReceived((IGetByeInfo *)poByeReport);
            break;
        }
    }
}


/**
 *
 * Method Name: ProcessSenderReport
 *
 *
 * Inputs:   unsigned char *puchRTCPReport - A pointer to an RTCP Sender Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: Takes the RTCP Sender Report passed and calls the CSenderReport
 *              object's ISenderReport interface to parse the Sender packet and
 *              extract the sender statistics contained therein.  This method
 *              shall also check for the presence of receiver reports within
 *              the packet and call the CReceiverReport object's
 *              IReceiverReport interface to parse and extract its contents.
 *              Although no more than one receiver report is expected under the
 *              current Pingtel call model, it is possible that multiple
 *              receiver reports (one per PingTel source) may be sent.  In this
 *              case,  a new CReceiverReport object shall be created and queued
 *              if not already existing on the Receiver Report list.
 *
 *
 * Usage Notes: Notifications shall be generated to all subscribing parties to
 *              inform them of the receipt of a new Source Report.  The
 *              notification shall contain the event type and a pointer to the
 *              Source Report's IGetSenderStatistics interface.
 *
 *
 */
unsigned long CRTCPSource::ProcessSenderReport(unsigned char *puchRTCPReport)
{

    unsigned long ulSenderSSRC     = GetSenderSSRC(TRUE, puchRTCPReport);
    unsigned long ulReportCount    = 0;
    unsigned long ulBytesProcessed;

    // Has a Sender Report object been instantiated for this participant?
    //  Probably not, if this is the first Sender Report received in a session
    if(m_poSenderReport != NULL);

    // Create The Sender Report Class
    else if((m_poSenderReport =
          new CSenderReport(ulSenderSSRC, m_piSetReceiverStatistics)) == NULL)
    {
        osPrintf("**** FAILURE **** CRTCPSource::ProcessSenderReport() -"
                          " Unable to create Inbound Sender Report Object\n");
        return(GetReportLength(puchRTCPReport));
    }

    // Initialize Sender Report Class
    else if(!m_poSenderReport->Initialize())
    {
        // Release the Sender Report reference.  This should cause the object
        // to be destroyed
        osPrintf("**** FAILURE **** CRTCPSource::ProcessSenderReport() -"
                      " Unable to Initialize Inbound Sender Report Object\n");
        ((ISenderReport *)m_poSenderReport)->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        return(GetReportLength(puchRTCPReport));
    }

    // A Sender object exists to process this report.  Let's delegate to its
    //  parsing methods to complete this report's processing.
    if((ulBytesProcessed =
                    m_poSenderReport->ParseSenderReport(puchRTCPReport)) == 0)
    {
        osPrintf("**** FAILURE **** CRTCPSource::ProcessSenderReport() -"
                                  " Unable to Parse Inbound Sender Report\n");
        return(GetReportLength(puchRTCPReport));
    }

    // Let's check whether there are Receiver reports associated with this
    //  Sender Report.  If so, delegate to the Receiver Report's processing
    //  method.
    else if((ulReportCount = m_poSenderReport->GetReportCount()) > 0)
    {
        unsigned long ulRRBytesProcessed = 0;

        // Send Event
        SendRTCPEvent(RTCP_SR_RCVD, (void *)m_poSenderReport);

        // Adjust the buffer so that it points to the beginning of the
        //  reception report section.
        puchRTCPReport += ulBytesProcessed;

        // Call Receiver Report processing method.
        if((ulRRBytesProcessed =
                   ProcessReceiverReport(puchRTCPReport, ulReportCount)) == 0)
            return(GetReportLength(puchRTCPReport));

        // Bump Total Byte Processed
        ulBytesProcessed += ulRRBytesProcessed;
    }

    return(ulBytesProcessed);

}


/**
 *
 * Method Name: ProcessReceiverReport
 *
 *
 * Inputs:   unsigned char *puchRTCPReport - pointer to an RTCP Receiver Report
 *           unsigned long ulReportCount   - Optional Count of receiver reports
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: Takes the RTCP Receiver Report passed and calls the
 *              CReceiverReport object's IReceiverReport interface to parse the
 *              Receiver packet and extract the receiver statistics contained
 *              therein.  Although no more than one receiver report is expected
 *              under the current Pingtel call model, it is possible that
 *              multiple receiver reports (one per PingTel source) may be sent.
 *              In this case, a new CReceiverReport object shall be created and
 *              queued if not already existing on the Receiver Report list.
 *
 *
 * Usage Notes: Notifications shall be generated to all subscribing parties to
 *              inform them of the receipt of a new Receiver Report.  The
 *              notification shall contain the event type and a pointer to the
 *              Receiver Report's IGetReceiverStatistics interface.
 *
 *
 */
unsigned long CRTCPSource::ProcessReceiverReport(unsigned char *puchRTCPReport,
                                                 unsigned long ulReportCount)
{
    bool             bRTCPHeader      = FALSE;
    unsigned long    ulReceiverSSRC   = 0;
    CReceiverReport *poReceiverReport = NULL;
    int              ulBytesProcessed = 0;
    int              ulReportSize     = 0;

    // Determine the Receiver Report count if it hasn't already been provided
    //  to us.
    if(ulReportCount == 0)
    {
        // Set a flag to identify the existence of an RTCP Header
        bRTCPHeader = TRUE;

        // Pull the Report Count from the header so we know how many reports
        //  we need to process
        ulReportCount = GetReportCount(puchRTCPReport);
        if (0 == ulReportCount) {
            // $$$ bRTCPHeader is the culprit that makes this necessary...
            //   It is passed to a couple of routines that should not care
            //   whether the header is still present.  The header is
            //   processed in the call to ParseReceiverReport when the
            //   header flag is set.  That processing should be done here,
            //   the length set to 8, then all the remaining reports should
            //   be processed without regard to the header.  But
            //         BETTER IS THE ENEMY OF GOOD!
            //   hzm 16Aug01
            ulReportSize = GetReportLength(puchRTCPReport);
            OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPSource::ProcessReceiverReport: RR/RC=0, len=%d", ulReportSize);
        }
    }

    // Let's prepare to iterate through each reception report until complete
    while(ulReportCount > 0)
    {

        // Extract the corresponding SSRC with the knowledge that this report
        //  comes with a full formed RTCP header.
        ulReceiverSSRC = GetReceiverSSRC(bRTCPHeader, puchRTCPReport);

        // Has a Receiver Report object been instantiated for this participant?
        // Probably not, if this is the first Receiver Report received in a
        //  session
        poReceiverReport = m_tReceiverReportList.GetEntry(RRSsrcComparitor, (void *)ulReceiverSSRC);
        if (NULL != poReceiverReport) ;

  // $$$ IS THE ABOVE LINE, ENDING WITH A SEMICOLON, CORRECT??? - hzm
  // Why, yes it is!  It sets us up to use all the else-if's that follow!

        // Create The Receiver Report object
        else if((poReceiverReport = new CReceiverReport(ulReceiverSSRC)) == NULL)
        {
            OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPSource::ProcessReceiverReport() - Unable to create Inbound Receiver Report Object");
         // BUT WHAT SHOULD WE RETURN?
         //     If we don't consume this report, we will come right back here... forever...
            assert(0 != ulReportSize);
            return(ulReportSize);
        }

        // Initialize Receiver Report object
        else if(!poReceiverReport->Initialize())
        {
            // Release the Receiver Report reference.
            // This should cause the object to be destroyed
            ((IReceiverReport *)poReceiverReport)->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
            OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPSource::ProcessReceiverReport() - Unable to Initialize Inbound Receiver Report Object");
         // BUT WHAT SHOULD WE RETURN?
         //     If we don't consume this report, we will come right back here... forever...
            assert(0 != ulReportSize);
            OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPSource::ProcessReceiverReport() - Unable to Initialize Inbound Receiver Report Object");
         // BUT WHAT SHOULD WE RETURN?
         //     If we don't consume this report, we will come right back here... forever...
            assert(0 != ulReportSize);
            return(ulReportSize);
        }

        // Place the new Receiver Report object on the collection list
        else if(!m_tReceiverReportList.AddEntry(poReceiverReport))
        {
            // Release the Receiver Report reference.
            // This should cause the object to be destroyed
            ((IReceiverReport *)poReceiverReport)->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
            OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPSource::ProcessReceiverReport() - Unable to Add Inbound Receiver Report Object to Collection");
         // BUT WHAT SHOULD WE RETURN?
         //     If we don't consume this report, we will come right back here... forever...
            assert(0 != ulReportSize);
            OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPSource::ProcessReceiverReport() - Unable to Add Inbound Receiver Report Object to Collection");
         // BUT WHAT SHOULD WE RETURN?
         //     If we don't consume this report, we will come right back here... forever...
            assert(0 != ulReportSize);
            return(ulReportSize);
        }

        // A Receiver object exists to process this report.
        // Let's delegate to its parsing methods to complete this report's
        //  processing.
        ulBytesProcessed = poReceiverReport->ParseReceiverReport(bRTCPHeader, puchRTCPReport);
        if (0 == ulBytesProcessed)
        {
            OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPSource::ProcessReceiverReport() - Unable to Parse Inbound Receiver Report\n");
         // BUT WHAT SHOULD WE RETURN?
         //     If we don't consume this report, we will come right back here... forever...
            assert(0 != ulReportSize);
            return(ulReportSize);
        }

        // We've made it through our first receiver report successfully and we
        //  must adjust the flags, counts, and report pointers for the next
        //  go-round.
        bRTCPHeader     = FALSE;
        ulReportSize    += ulBytesProcessed;
        puchRTCPReport  += ulBytesProcessed;
        ulReportCount--;

        // Send Event
        SendRTCPEvent(RTCP_RR_RCVD, (void *)poReceiverReport);

    }

    return(ulReportSize);
}


/**
 *
 * Method Name: ProcessSDESReport
 *
 *
 * Inputs:   unsigned char *puchRTCPReport
 *                              - pointer to an RTCP Source Description Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: Takes the RTCP SDES Report passed and calls the
 *              CSourceDescription object's ISDESReport interface to parse the
 *              Source Description packet and extract the identification
 *              information contained therein.
 *
 *
 * Usage Notes: A call connection to a Mixer would cause all SDES Reports to
 *              be forward as multiple SDES Reports. In this case, a new
 *              CSourceDescription object shall be created and queued if not
 *              already existing on the SrcDescription list.  The SSRC ID will
 *              be used to determine uniqueness among reports.
 *
 *              Notifications shall be generated to all subscribing parties to
 *              inform them of the new Source Descriptions or changes in
 *              previously existing Source Descriptions.  The notification
 *              shall contain the event type and a pointer to the new or
 *              modified Source Description interface (IGetSrcDescription).
 */
unsigned long CRTCPSource::ProcessSDESReport(unsigned char *puchRTCPReport)
{

    bool                bRTCPHeader      = TRUE;
    unsigned long       ulEventMask      = 0;
    unsigned long       ulSDESLength     = 0;
    unsigned long       ulReportCount    = 0;
    ssrc_t       ulSenderSSRC     = 0;
    unsigned long       ulReportSize     = GetReportLength(puchRTCPReport);
    CSourceDescription *poSDESReport     = NULL;

    // Pull the Report Count from the header so we no how many reports we need
    //  to process
    ulReportCount = GetReportCount(puchRTCPReport);

    // Let's prepare to iterate through each reception report until complete
    while(ulReportCount > 0)
    {
        // Extract the corresponding SSRC with the knowledge that this report
        //  comes with a full formed RTCP header.
        ulSenderSSRC = GetSenderSSRC(bRTCPHeader, puchRTCPReport);

        // Has a SDES Report object been instantiated for this participant?
        // Probably not, if this is the first SDES Report received in a session
        if((poSDESReport = m_tSrcDescriptorList.GetEntry(SDESSsrcComparitor,
                                              (void *)ulSenderSSRC)) != NULL);

        // Create The SDES Report object
        else if((poSDESReport = new CSourceDescription(ulSenderSSRC)) == NULL)
        {
            osPrintf("**** FAILURE **** CRTCPSource::ProcessSDESReport()"
                          " - Unable to Create Inbound SDES Report Object\n");
            return(ulReportSize);
        }

        // Initialize SDES Report object
        else if(!poSDESReport->Initialize())
        {
            // Release the SDES Report reference.  This should cause the
            //  object to be destroyed
            osPrintf("**** FAILURE **** CRTCPSource::ProcessSDESReport()"
                      " - Unable to Initialize Inbound SDES Report Object\n");
            ((ISDESReport *)poSDESReport)->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
            return(ulReportSize);
        }

        // Place the new SDES Report object on the collection list
        else if(!m_tSrcDescriptorList.AddEntry(poSDESReport))
        {
            // Release the SDES Report reference.
            // This should cause the object to be destroyed
            osPrintf("**** FAILURE **** CRTCPSource::ProcessSDESReport()"
                       " - Unable to Add SDES Report Object to Collection\n");
            ((ISDESReport *)poSDESReport)->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
            return(ulReportSize);
        }

        else
        {
            // Set the event mask to indicate that a new SDES was received
            ulEventMask |= RTCP_NEW_SDES;
        }


        // A SDES object exists to processes this report.  Let's delegate to
        //  its parsing methods to complete this report's processing.
        if((ulSDESLength =
             poSDESReport->ParseSDESReport(bRTCPHeader, puchRTCPReport)) == 0)
        {
            osPrintf("**** FAILURE **** CRTCPSource::ProcessSDESReport()"
                                  " - Unable to Parse Inbound SDES Report\n");
            return(ulReportSize);
        }

        // We've made it through our first SDES report successfully and we must
        // adjust the flags, counts, and report pointers for the next go-round.
        bRTCPHeader     = FALSE;
        puchRTCPReport  += ulSDESLength;
        ulReportCount--;

        // Check whether any information has changed in an existing SDES Report
        ulEventMask |= RTCP_SDES_UPDATE;
        SendRTCPEvent(ulEventMask,
                            (void *)poSDESReport, poSDESReport->GetChanges());
        ulEventMask = 0;

    }

    return(ulReportSize);
}

/**
 *
 * Method Name: ProcessByeReport
 *
 *
 * Inputs:   unsigned char *puchRTCPReport - A pointer to an RTCP Bye Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: Takes the RTCP Bye Report and extracts the associated SSRC ID.
 *              A notification shall be generated to all subscribing parties to
 *              inform them of the receipt of the BYE along with the associated
 *              SSRC ID.
 *
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTCPSource::ProcessByeReport(unsigned char *puchRTCPReport)
{
    unsigned long ulSenderSSRC     = GetSenderSSRC(TRUE, puchRTCPReport);
    unsigned long ulBytesProcessed = GetReportLength(puchRTCPReport);

    // Has a Bye Report object been instantiated for this participant?
    // Probably not, if this is the first Sender Report received in a session
    if(m_poByeReport != NULL);

    // Create The Bye Report Class
    else if((m_poByeReport = new CByeReport(ulSenderSSRC)) == NULL)
    {
        osPrintf("**** FAILURE **** CRTCPSource::ProcessByeReport()"
                           " - Unable to Create Inbound Bye Report Object\n");
        return(ulBytesProcessed);
    }

    // Initialize Bye Report Class
    else if(!m_poByeReport->Initialize())
    {
        // Release the Bye Report reference.  This should cause the object
        // to be destroyed
        osPrintf("**** FAILURE **** CRTCPSource::ProcessByeReport()"
                       " - Unable to Initialize Inbound Bye Report Object\n");
        ((IByeReport *)m_poByeReport)->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        return(ulBytesProcessed);
    }

    // A Bye object exists to processes this report.  Let's delegate to its
    // parsing methods to complete this report's processing.
    m_poByeReport->ParseByeReport(puchRTCPReport);

    // Set the event mask to refelect receiving this report
    SendRTCPEvent(RTCP_BYE_RCVD, (void *)m_poByeReport);

    return(ulBytesProcessed);

}


/**
 *
 * Method Name: ProcessAppReport
 *
 *
 * Inputs:   unsigned char *puchRTCPReport
 *                                   - A pointer to an RTCP Application Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: No processing is performed on an Application Report but the
 *              length is extracted from the header and returned so that other
 *              composite reports might still be processed.
 *
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTCPSource::ProcessAppReport(unsigned char *puchRTCPReport)
{

    // Disregard this packet
    return(GetReportLength(puchRTCPReport));
}

/**
 *
 * Method Name:  GetStatistics
 *
 *
 * Inputs:   None
 *
 * Outputs:  IGetSrcDescription     **piGetSrcDescription
 *                                      - Source Description Interface Pointer
 *           IGetSenderStatistics   **piSenderStatistics
 *                                       - Sender Statistics Interface Pointer
 *           IGetReceiverStatistics **piReceiverStatistics
 *                                     - Receiver Statistics Interface Pointer
 *           IByeInfo               **piGetByeInfo
 *                                  - Interface for retrieving Bye Report Info
 *
 * Returns:  None
 *
 * Description: Returns pointers to the Sender, Receiver and Bye statistics
 *              interfaces upon request.
 *
 * Usage Notes: This would be used by the QOS object or the RTCP Statistics
 *              object if a polling method were supported.  These objects
 *              could alternatively be informed of statistic changes via
 *              notification with the interfaces contained within the callback.
 *
 *
 */
void CRTCPSource::GetStatistics(IGetSrcDescription     **piGetSrcDescription,
                                IGetSenderStatistics   **piSenderStatistics,
                                IGetReceiverStatistics **piReceiverStatistics,
                                IGetByeInfo            **piGetByeInfo)
{


    // Load the pointers to the corresponding statistics retrieval interfaces
    if(piSenderStatistics != NULL)
    {
        *piSenderStatistics = (IGetSenderStatistics *)m_poSenderReport;
        (*piSenderStatistics)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    }
    if(piReceiverStatistics != NULL)
    {
        m_tReceiverReportList.TakeLock();
        *piReceiverStatistics = (IGetReceiverStatistics *)m_tReceiverReportList.GetFirstEntry();
        (*piReceiverStatistics)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
        m_tReceiverReportList.ReleaseLock();
    }

    if(piGetByeInfo != NULL)
    {
        *piGetByeInfo = (IGetByeInfo *)m_poByeReport;
        (*piGetByeInfo)->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
    }

}
#endif /* INCLUDE_RTCP ] */
