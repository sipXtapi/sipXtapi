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

import org.sipfoundry.sipxconfig.admin.dialplan.config.Permission;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

/**
 * IDialingRule
 */
public interface IDialingRule {
    public abstract String getDescription();

    public abstract boolean isEnabled();

    public abstract List getGateways();

    public abstract String[] getPatterns();

    public abstract Transform[] getTransforms();

    public abstract Permission[] getPermissions();

    public abstract String getName();
}