/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway;


public class SipTrunkModel extends GatewayModel {
    
    public SipTrunkModel() {
        setBeanId(SipTrunk.BEAN_ID);
    }

    /** SIP trunks don't have serial numbers, so return false */
    @Override
    public boolean getHasSerialNumber() {
        return false;
    }

}
