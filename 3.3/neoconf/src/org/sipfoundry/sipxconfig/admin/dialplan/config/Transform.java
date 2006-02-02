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

import org.dom4j.Element;

/**
 * Transform
 */
public abstract class Transform {
    public Element addToParent(Element parent) {
        Element transform = parent.addElement("transform");
        addChildren(transform);
        return transform;
    }

    protected abstract void addChildren(Element transform);
}
