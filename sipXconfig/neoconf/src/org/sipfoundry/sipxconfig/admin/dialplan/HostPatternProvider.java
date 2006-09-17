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
package org.sipfoundry.sipxconfig.admin.dialplan;

/**
 * Implement if you rule wants to influence hostPattern elements of mappingrules.xml
 */
public interface HostPatternProvider extends IDialingRule {
    
    /**
     * @return ip addresses, host names, or variables defined in config.defs
     */
    public abstract String[] getHostPatterns();

}
