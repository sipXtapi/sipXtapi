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

#ifndef _SipTransaction_h_
#define _SipTransaction_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES


#include <os/OsDefs.h>
#include <os/OsSocket.h>
#include <os/OsMsgQ.h>
#include <net/Url.h>
#include <net/SipSrvLookup.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipMessage;
class SipUserAgent;
class SipTransactionList;
class OsEvent;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipTransaction : public UtlString {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    static int smTransactionNum;
    static UtlString smBranchIdBase;

    // Note: see the RFC 2543 bis for a defininition and description
    // of these transaction states and when the transitions occur
    enum transactionStates {
        TRANSACTION_UNKNOWN = 0,
        TRANSACTION_LOCALLY_INIITATED, // No messages sent (usually client)
        TRANSACTION_CALLING, // Request sent
        TRANSACTION_PROCEEDING, // Provisional response received
        TRANSACTION_COMPLETE, // Final response received
        TRANSACTION_CONFIRMED, // ACK recieved for 300-699 response classes
        TRANSACTION_TERMINATED
    };
    //: Enumerates the states of a transaction
    //!enumcode: TRANSACTION_UNKNOWN - net yet set
    //!enumcode: TRANSACTION_LOCALLY_INIITATED - No messages sent (usually client)
    //!enumcode: TRANSACTION_CALLING - Request sent
    //!enumcode: TRANSACTION_PROCEEDING - Provisional response received
    //!enumcode: TRANSACTION_COMPLETE - Final response received
    //!enumcode: TRANSACTION_CONFIRMED - ACK recieved for 300-699 response classes
    //!enumcode: TRANSACTION_TERMINATED

    enum messageRelationship {
        MESSAGE_UNKNOWN,
        MESSAGE_UNRELATED,
        MESSAGE_SAME_SESSION,
        MESSAGE_DIFFERENT_BRANCH,
        MESSAGE_REQUEST,
        MESSAGE_PROVISIONAL,
        MESSAGE_FINAL,
        MESSAGE_NEW_FINAL,
        MESSAGE_CANCEL,
        MESSAGE_CANCEL_RESPONSE,
        MESSAGE_ACK,
        MESSAGE_2XX_ACK,
        MESSAGE_DUPLICATE
    };
    //: Enumerate the relationship of a message to a transaction
    //!enumcode: MESSAGE_UNKNOWN - Relationship not yet determined, or error
    //!enumcode: MESSAGE_UNRELATED - A with different Call-Id, To or From
    //!enumcode: MESSAGE_SAME_SESSION - But not part of this TX or related branches
    //!enumcode: MESSAGE_DIFFERENT_BRANCH - Same Call-Id, to, from, cseq but different TX
    //!enumcode: MESSAGE_REQUEST - The request to this TX
    //!enumcode: MESSAGE_PROVISIONAL - A provision response to this TX
    //!enumcode: MESSAGE_FINAL - The first final response to this TX
    //!enumcode: MESSAGE_NEW_FINAL - A different final response for this TX
    //!enumcode: MESSAGE_CANCEL  - A cancel for this TX
    //!enumcode: MESSAGE_ACK - An ACK for this non-2xx TX
    //!enumcode: MESSAGE_2XX_ACK - An ACK assocated with this TX (but considered a different TX)
    //!enumcode: MESSAGE_DUPLICATE - A duplicate message for this TX

/* ============================ CREATORS ================================== */

    SipTransaction(SipMessage* request = NULL,
                   UtlBoolean isOutgoing = TRUE,
                   UtlBoolean userAgentTransaction = TRUE);
    //:Default constructor
    // When this is an out going request, this is a client
    // transaction.  The via header field MUST be added before
    // constructing this transaction as this sets the branch ID.
    //! param: userAgentTransaction - user agent (vs proxy) client or
    //         server



    virtual
    ~SipTransaction();
    //:Destructor

/* ============================ MANIPULATORS ============================== */

    void getNewBranchId(SipMessage& request,
                        UtlString& branchId);

    UtlBoolean handleOutgoing(SipMessage& outgoingMessage,
                             SipUserAgent& userAgent,
                             SipTransactionList& transactionList,
                             enum messageRelationship relationship);

    void handleResendEvent(const SipMessage& outgoingMessage,
                            SipUserAgent& userAgent,
                            enum messageRelationship relationship,
                            SipTransactionList& transactionList,
                            int& nextTimeout,
                            SipMessage*& delayedDispatchedMessage);

    void handleExpiresEvent(const SipMessage& outgoingMessage,
                            SipUserAgent& userAgent,
                            enum messageRelationship relationship,
                            SipTransactionList& transactionList,
                            int& nextTimeout,
                            SipMessage*& delayedDispatchedMessage);

    UtlBoolean handleIncoming(SipMessage& incomingMessage,
                             SipUserAgent& userAgent,
                             enum messageRelationship relationship,
                             SipTransactionList& transactionList,
                             SipMessage*& delayedDispatchedMessage);


/* ============================ Deprecated ============================== */

    //void getMessageToSend(SipMessage*& messagePtr,
    //                      enum OsSocket::SocketProtocolTypes& protocol,
    //                      UtlString& address,
    //                      int& port,
    //                      UtlBoolean& transactionTimeoutFailure);
    //: Provides a message to send, if there is one that needs sending
    // If the tranasaction times out transactionTimeoutFailure is set
    // to true and the caller should send the given message back to
    // the application as a transport error
    //! param: messagePtr - [out] provides the message to send
    //! param: protocol - [out] provides the protocol to use to send
    //! param: address - [out] the address to send the message to
    //! param: port - [out] the port to send the message to
    //! param: transactionTimeoutFailure - the transaction has timed out
    //         a transport timeout message should be sent back to the
    //         application.

    //void sendSucceeded(OsMsgQ& timeOutMessageQueue);
    //: Updates the status of the transaction with the outcome of a successful transport send attempt
    // A timer is set to expire when the next resend or
    // when the transaction is considered failed.
    //! param: timeOutMessageQueue - [in] queue to deposit the SipMessageEvent
    //         when the timer expires.

    //void sendFailed(UtlBoolean& tryAnotherProtocol);
    //: Updates the status of the transaction with the outcome of a failed transport send attempt
    // This is called after a failed attempt to send a
    // message provided by getMessageToSend.  In the failed case
    // either another protocol is tryed (retrieved via another call
    // to getMessageToSend) or the transaction goes to a failed state
    // and the failed message should be sent back to the application
    // as a Transport error.

    void linkChild(SipTransaction& child);

    void toString(UtlString& dumpString,
                  UtlBoolean dumpMessagesAlso);
    //: Serialize the contents of this

    void dumpTransactionTree(UtlString& dumpstring, 
                             UtlBoolean dumpMessagesAlso);
    //: Serialize the contents of all the transactions in this tree
    // The parent is found first and then all children are serialized
    // recursively

    void dumpChildren(UtlString& dumpstring, 
                      UtlBoolean dumpMessagesAlso);
    //: Serialize the contents of all the child transactions to this transaction
    // All children are serialized recursively


/* ============================ ACCESSORS ================================= */

    static void getStateString(enum transactionStates state,
                               UtlString& stateString);

    static void getRelationshipString(enum messageRelationship relationship,
                                      UtlString& relationshipString);

    static void buildHash(const SipMessage& message, 
                          UtlBoolean isOutgoing,
                          UtlString& hash);

    SipTransaction* getTopMostParent() const;

    void getCallId(UtlString& callId) const;

    enum transactionStates getState() const;

    long getStartTime() const;

    long getTimeStamp() const;

    void touch();
    void touchBelow(int newDate);

    SipMessage* getRequest();

    SipMessage* getLastProvisionalResponse();

    SipMessage* getLastFinalResponse();

    void cancel(SipUserAgent& userAgent,
                SipTransactionList& transactionList);
    //: cancel any outstanding client transactions (recursively on children)

    //const SipMessage* getAck() const;

    void markBusy();

    void markAvailable();

    void notifyWhenAvailable(OsEvent* availableEvent);
    //: The given event is signaled when this transaction is not busy

    void signalNextAvailable();

    void signalAllAvailable();

/* ============================ INQUIRY =================================== */

    UtlBoolean isServerTransaction() const;
    //: Inquire if this transaction is a server as opposed to a client transaction

    //! Inquiry as to whether this transaction is a recursed DNS SRV child
    UtlBoolean isDnsSrvChild() const;

    UtlBoolean isUaTransaction() const;
    //: Inquire if transaction is UA based or proxy
    // Note this is different than server vs client transaction

    UtlBoolean isChildSerial();
    //: Inquire as to whether child transaction will be serial or all parallel searched
    // If all immediate child transactions have the same 
    // Q value FALSE is returned

    UtlBoolean isEarlyDialogWithMedia();
    //: Tests to see if this is an existing early dialog with early media
    // If transaction has not yet been completed and there was early media
    // (determined by the presence of SDP in a provisional response

    UtlBoolean isChildEarlyDialogWithMedia();
    //: Are any of the children in an early dialog with media

    UtlBoolean isMethod(const char* methodToMatch) const;
    //: see if this tranaction is of the given method type

    enum messageRelationship whatRelation(const SipMessage& message,
                                          UtlBoolean isOutgoing) const;
    //: Check if the given message is part of this transaction

    UtlBoolean isBusy();
    //: is this transaction being used (e.g. locked)

    //UtlBoolean isDuplicateMessage(SipMessage& message,
    //                             UtlBoolean checkIfTransactionMatches = TRUE);
    //: Check to see if this request or response has already been received by this transaction

    UtlBoolean isUriChild(Url& uri);
    // Does this URI already exist as an immediate child to this transaction
    // Search through each of the children and see if the child
    // transaction's URI matches.

    UtlBoolean isUriRecursed(Url& uri);
    // Has this URI been recursed anywhere in this transaction tree already
    // Start looking at the parent

    UtlBoolean isUriRecursedChildren(UtlString& uriString);
    // Has this URI been recursed anywhere at or below in this transaction tree already
    // Look at or below the current transaction in the transaction tree

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    void handleChildTimeoutEvent(SipTransaction& child,
                                 const SipMessage& outgoingMessage,
                                 SipUserAgent& userAgent,
                                 enum messageRelationship relationship,
                                 SipTransactionList& transactionList,
                                 int& nextTimeout,
                                 SipMessage*& delayedDispatchedMessage);
    //: tells the parent transaction the result of the timeout event

    UtlBoolean handleChildIncoming(//SipTransaction& child,
                                  SipMessage& incomingMessage,
                                  SipUserAgent& userAgent,
                                  enum messageRelationship relationship,
                                  SipTransactionList& transactionList,
                                  UtlBoolean childSaysShouldDispatch,
                                  SipMessage*& delayedDispatchedMessage);
    //: Tells the parent transaction the result of the incoming message
    //! returns: TRUE/FALSE as to whether the message should be dispatched to applications

    UtlBoolean startSequentialSearch(SipUserAgent& userAgent,
                                    SipTransactionList& transactionList);
    //: Checks to see if a final response can be sent or if sequential search should be started

    UtlBoolean recurseChildren(SipUserAgent& userAgent,
                              SipTransactionList& transactionList);
    //: Starts search on any immediate children of the highest unpursued Q value

    UtlBoolean recurseDnsSrvChildren(SipUserAgent& userAgent,
                              SipTransactionList& transactionList);
    //: Starts search on any immediate DNS SRV children of the highest unpursued Q value

    UtlBoolean findBestResponse(SipMessage& bestResponse);
    // Finds the best final response to return the the server transaction

    enum messageRelationship addResponse(SipMessage* response,
                                         UtlBoolean isOutGoing,
                                         enum messageRelationship relationship = MESSAGE_UNKNOWN);
    //: Adds the provisional or final response to the transaction

    void cancelChildren(SipUserAgent& userAgent,
                        SipTransactionList& transactionList);
    //: Cancels children transactions on a server transaction

    //enum messageRelationship addAck(SipMessage* ack,
    //            enum messageRelationship relationship = MESSAGE_UNKNOWN);
    //: Adds ACK if it is part of this transaction
    // Note: Only ACKs for transactions with NON-2XX final response
    // are part of the transaction.
    // ACK after a 2XX response are considered separate transactions

    //void determineSendData(SipMessage& message,
    //                enum SipTransaction::messageRelationship& relationship,
    //                       enum OsSocket::SocketProtocolTypes protocolType,
    //                       UtlBoolean& shouldSend,
    //                       int& scheduleResendMillisec);
    //: Set the transport data of the matched message in this transaction
    // This looks for a matched message in this transaction  It also
    // determines if the message should be sent and when to schedule
    // a resend.  If the message should be sent then it sets the
    // transport information.
    // Note: it is assumed that message is a copy of one of the messages
    // in this session.  This method does not validate a match to this
    // transaction.  It only matches response code and request method
    // to determine which message matches.
    //! param: message - [in] the message to match in this transaction
    //! param: relationship - [in] what type of relationship message has
    //         with this transaction
    //! param: protocolType - [in] the protocol this message is to be sent
    //         (e.g. UDP, TCP, TLS)
    //! param: shouldSend - [out] TRUE/FALSE this message should be sent (i.e.
    //         if we have already received a response we do not need to
    //         send the request now.)
    //! param: rescheduleResendMillisec - [out] when should the timer be sent
    //         to resend this message (milliseconds) Note: <=0 if it
    //         should not be resent

    void doMarkBusy(int markValue);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    SipTransaction(const SipTransaction& rSipTransaction);
    //:Copy constructor (disabled)
    SipTransaction& operator=(const SipTransaction& rhs);
    //:Assignment operator (disabled)

    UtlBoolean doResend(SipMessage& resendMessage,
                       SipUserAgent& userAgent,
                       int& nextTimeoutMs);

    UtlBoolean doFirstSend(SipMessage& message,
                          enum messageRelationship relationship,
                          SipUserAgent& userAgent,
                          UtlString& toAddress,
                          int& port,
                          enum OsSocket::SocketProtocolTypes& toProtocol);

    void prepareRequestForSend(SipMessage& request,
                               SipUserAgent& userAgent,
                               UtlBoolean& addressRequiresDnsSrvLookup,
                               UtlString& toAddress,
                               int& port,
                               enum OsSocket::SocketProtocolTypes& toProtocol);

    // CallId  + 's' or 'c' (for server or client) is used as
    // the key for the hash (i.e. stored as the string/data in
    // this UtlString
    UtlString mCallId;
    UtlString mBranchId;
    UtlString mRequestUri;
    Url mFromField;
    Url mToField;
    UtlString mRequestMethod;
    int mCseq;
    UtlBoolean mIsServerTransaction; // vs. client transaction
    UtlBoolean mIsUaTransaction; // UA or proxy transaction
    UtlString mSendToAddress;
    int mSendToPort;
    enum OsSocket::SocketProtocolTypes mSendToProtocol;
    server_t* mpDnsSrvRecords;
    SipMessage* mpRequest;
    SipMessage* mpLastProvisionalResponse;
    SipMessage* mpLastFinalResponse;
    SipMessage* mpAck;
    SipMessage* mpCancel;
    SipMessage* mpCancelResponse;
    SipTransaction* mpParentTransaction;
    UtlSList mChildTransactions;
    long mTransactionCreateTime;  // When this thing was created
    long mTransactionStartTime; //  When the request was sent/received i.e. went to TRANSACTION_CALLING state
    long mTimeStamp; // The last this thing was used
    enum transactionStates mTransactionState;
    UtlBoolean mDispatchedFinalResponse; // For UA recursion
    UtlBoolean mProvisionalSdp; // early media

    // Recursion members
    UtlBoolean mIsCanceled;
    UtlBoolean mIsRecursing; // True if any braches have not be pursued
    UtlBoolean mIsDnsSrvChild; // This CT pursues one of the SRV records of the parent CT
    double mQvalue; // Recurse order.  equal values are recursed in parallel
    int mExpires; // Maximum time (seconds) to wait for a final outcome
    UtlBoolean mIsBusy;
    UtlString mBusyTaskName;
    UtlSList* mWaitingList;  // Events waiting until this is available
                                      // Note only a parent tx should have a waiting list

};

/* ============================ INLINE METHODS ============================ */

#endif // _SipTransaction_h_
