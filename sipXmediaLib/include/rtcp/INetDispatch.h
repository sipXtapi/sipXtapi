//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

//  Border Guard
#ifndef _INetDispatch_h
#define _INetDispatch_h

#include "rtcp/RtcpConfig.h"

// Include
#include "IBaseClass.h"

/**
 *
 * Interface Name:  INetDispatch
 *
 * Inheritance:     None
 *
 *
 * Description:     The INetDispatch interface allows a Network Source object
 *                  to forward data packets to a recipient object as they are
 *                  received on the network interface.
 *
 * Notes:
 *
 */
interface INetDispatch : public IBaseClass
 {

//  Public Methods

public:


/**
 *
 * Method Name: ProcessPacket
 *
 *
 * Inputs:   unsigned char *puchDataBuffer
 *             Data Buffer received from Network Source
 *           unsigned long ulBufferLength
 *             Length of Data Buffer
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The packet received is processed and dispatched after having
 *              been received from the Network Source object
 *
 * Usage Notes:
 *
 *
 */
    virtual void ProcessPacket(unsigned char *puchDataBuffer,
                               unsigned long ulBufferLength, int vbose=0) = 0;


};

#endif

