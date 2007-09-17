//  
// Copyright (C) 2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2007 Wirtualna Polska S.A. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Andrzej Ciarkowski <andrzejc AT wp-sa DOT pl>

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/video/MpVideoEncoder.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */
MpVideoEncoder::MpVideoEncoder(int payloadType, MprToNet* pRtpWriter)
 : mPayloadType(payloadType)
 , mpRtpSink(pRtpWriter)
{
}

MpVideoEncoder::~MpVideoEncoder()
{
}

/* ============================= MANIPULATORS ============================= */

/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */


/* ////////////////////////////// PROTECTED /////////////////////////////// */


/* /////////////////////////////// PRIVATE //////////////////////////////// */


/* ============================== FUNCTIONS =============================== */

