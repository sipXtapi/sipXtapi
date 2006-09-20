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
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.permission.Permission;

/**
 * IDialingRule
 */
public interface IDialingRule {
    public abstract String getDescription();

    public abstract void setDescription(String description);

    public abstract boolean isEnabled();

    public abstract void setEnabled(boolean enabled);

    public abstract String getName();

    public abstract void setName(String name);

    public abstract List<Gateway> getGateways();

    public abstract String[] getPatterns();

    public abstract Transform[] getTransforms();

    public abstract List<Permission> getPermissions();

    public abstract Integer getId();

    public abstract boolean isInternal();

    public abstract String[] getTransformedPatterns(Gateway gateway);

    /**
     * List of host patterns for this rule, if empty rule will be appended to default host match
     * 
     * @return ip addresses, host names, or variables defined in config.defs
     */
    public abstract String[] getHostPatterns();
}
