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
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.BaseComponent;

/**
 * CancelButton activates return page.
 * 
 * We may want to add functionality of bypassing client and server validation here. Seeting
 * "canceled" property on the form etc.
 * 
 * Dynamic navigation get be implemented by binding ognl expression (such as method call) to
 * returnPageName property.
 */
public abstract class CancelButton extends BaseComponent {
    public abstract String getReturnPageName();

    public String cancel() {
        return getReturnPageName();
    }
}
