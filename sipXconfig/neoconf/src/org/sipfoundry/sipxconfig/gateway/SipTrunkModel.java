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

import org.sipfoundry.sipxconfig.phone.PhoneModel;

public class SipTrunkModel extends PhoneModel {

    public SipTrunkModel(String beanId, String label) {
        super(beanId, label);
    }
    
    /** SIP trunks don't have serial numbers, so return false */
    @Override
    public boolean getHasSerialNumber() {
        return false;
    }

}
