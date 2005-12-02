//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _INetworkRender_h
#define _INetworkRender_h

#include "rtcp/RtcpConfig.h"

// Includes
#include "IBaseClass.h"
#include "INetworkChannel.h"

/**
 *
 * Interface Name:  INetworkRender
 *
 * Inheritance:     INetworkChannel
 *
 *
 * Description:     The INetworkRender interface allows allows a consumer to
 *                  send a protocol data unit (PDU) to a network addressable
 *                  entity.
 *
 * Notes:
 *
 */
interface INetworkRender : public INetworkChannel
 {

//  Public Methods

public:

/*|><|************************************************************************
 Name:        Send

 Description: A private method that takes a data transmission delivered by the
              StreamClient's DataReceived() callback and sends it the
              associated channel socket for transmission to the respective FE.
              The Send() method also contains some conditionally compiled
              logic to collect statistics regarding packets being sent.

 Returns:     int.
 ***********************************************************************|><|*/
    virtual int  Send(unsigned char * puchDataBuffer,
                      unsigned long ulBytesToSend) = 0;


};


#endif
