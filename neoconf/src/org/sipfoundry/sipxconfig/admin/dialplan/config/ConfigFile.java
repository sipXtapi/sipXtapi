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
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

/**
 * ConfigFile
 */
public interface ConfigFile {
    /**
     * Called for each enabled rule
     * @param rule
     */
    public abstract void generate(IDialingRule rule);
    
    /**
     * Called after the last rule
     */
    public abstract void end();
}
