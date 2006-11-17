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
package org.sipfoundry.sipxconfig.components;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.sipfoundry.sipxconfig.setting.Group;

public abstract class GroupMultiSelect extends BaseComponent {
    
    public abstract Collection<Group> getSource();
    public abstract void setGroupCandidates(Collection<Group> candidates);

    public void buildGroupCandidates(String groupsString) {
        Collection candidates = TapestryUtils.getAutoCompleteCandidates(getSource(), groupsString);
        setGroupCandidates(candidates);        
    }
}
