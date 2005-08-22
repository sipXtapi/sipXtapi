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

/***
 * Settings about the sipX servers.  
 * 
 * <pre>
 * BAD Instead of this:
 *   String value = sipx.getServer().getSettings().getSetting("some/path").getValue();
 *   
 * GOOD Do this:
 *   String value = sipx.getServer().getServerSettings().getDomainName();
 * </pre>    
 * 
 * You will find mapping in SipxProcessContextImpl
 */
public interface ServerSettings {
    
    public String getDomainName();

}
