//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _CSeqManager_h_
#define _CSeqManager_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlHashMap.h"
#include "utl/UtlRandom.h"
#include "utl/UtlString.h"
#include "os/OsMutex.h"

// DEFINES
#define CSEQ_MANAGER_STATE_CHECKS
#undef  CSEQ_MANAGER_STATE_CHECKS

#define CSEQ_ID_INVITE      "INVITE"
#define CSEQ_ID_INFO        "INFO"
#define CSEQ_ID_NOTIFY      "NOTIFY"
#define CSEQ_ID_REFER       "REFER"
#define CSEQ_ID_OPTIONS     "OPTIONS" 

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef struct CSEQ_CONTEXT
{
    UtlString  identifier ;
    bool       bInTransaction ;
    int       iCSeq ;
} CSEQ_CONTEXT ;

// FORWARD DECLARATIONS


/**
 * CSeqManager manages the CSeq number space for a dialog.  It keeps track of
 * multiple identifiers to handle overlapping transactions (e.g. send a NOTIFY
 * while an INFO transaction is still outstanding).
 */
class CSeqManager
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

    /**
     * Default constructor
     */
    CSeqManager();
     
    /**
     * Destructor
     */
    virtual ~CSeqManager();

/* ============================ MANIPULATORS ============================== */

    /**
     * startTransaction notes that a transaction has been started and will
     * return FALSE if any other transaction was already started.  It is 
     * assumed that if startTransaction fails (returns false), the developer
     * will try to start the transaction again later.  This method will 
     * increase the CSeq number regardless of success.
     */
    bool startTransaction(const char* szIdentifier, int& iCSeq) ;

    /**
     * endTransaction marks the end of a transaction.
     */
    bool endTransaction(const char* szIdentifier) ;

    /**
     * Dump state into specified string
     */
    void dumpState(UtlString& state) ;

/* ============================ ACCESSORS ================================= */
    
    /**
     * Get the current csequence number
     */
    int getCSeqNumber(const char* szIdentifier) ;

/* ============================ INQUIRY =================================== */

    /** 
     * Is the specified identifer in a transaction already?
     */ 
    bool isInTransaction(const char* szIdentifier) ;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
    UtlHashMap   mHashMap ;
    UtlRandom    mRandomGenerator ;
    OsMutex      mGuard ;
    int          miCSeq ;

    /** 
     * Gets the context for the specified identifier.  If no context is 
     * available, one is created.
     */
    CSEQ_CONTEXT* getContext(const char* szIdentifier) ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

    /** Disabled copy constructor */ 
    CSeqManager(const CSeqManager& rCSeqManager);     

    /** Disabled assignment operator */
    CSeqManager& operator=(const CSeqManager& rhs);  

};

/* ============================ INLINE METHODS ============================ */

#endif  // _CSeqManager_h_
