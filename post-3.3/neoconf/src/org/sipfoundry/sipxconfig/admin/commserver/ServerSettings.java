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
package org.sipfoundry.sipxconfig.admin.commserver;

/**
 * Settings about the sipX servers.
 * 
 * <pre>
 *         BAD Instead of this:
 *           String value = sipx.getServer().getSettings().getSetting(&quot;some/path&quot;).getValue();
 *           
 *         GOOD Do this:
 *           String value = sipx.getServer().getServerSettings().getDomainName();
 * </pre>
 * 
 * You will find mapping in SipxProcessContextImpl
 */
public interface ServerSettings {

    static final String DOMAIN_NAME = "domain/SIPXCHANGE_DOMAIN_NAME";
    static final String PRESENCE_SIGN_IN_CODE = "presence/SIP_PRESENCE_SIGN_IN_CODE";
    static final String PRESENCE_SIGN_OUT_CODE = "presence/SIP_PRESENCE_SIGN_OUT_CODE";
    static final String PRESENCE_SERVER_SIP_PORT = "presence/PRESENCE_SERVER_SIP_PORT";
    // note: the name of the setting is misleading - this is actually full host name not just a
    // domain name
    static final String PRESENCE_SERVER_LOCATION = "presence/SIP_PRESENCE_DOMAIN_NAME";   
    static final String PRESENCE_API_PORT = "presence/SIP_PRESENCE_HTTP_PORT";
    
    public String getDomainName();

    public String getPresenceSignInCode();

    public String getPresenceSignOutCode();

    public String getPresenceServerSipPort();
    
    public String getPresenceApiPort();
}
