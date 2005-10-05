/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

public class MediantGateway extends AudioCodesGateway {
    protected void defaultSettings() {
        super.defaultSettings();
        setSettingValue("SIP_Params/SIPGATEWAYNAME", getDefaults().getDomainName());
        setSettingValue("SIP_Params/SIPDESTINATIONPORT", getDefaults().getProxyServerSipPort());
    }
}
