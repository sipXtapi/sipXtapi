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

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.html.BasePage;

public abstract class PageWithCallback extends BasePage {
    
    public abstract ICallback getCallback();
    public abstract void setCallback(ICallback callback);
    
    /**
     * Activate this page, setting a callback that will navigate back to the named
     * return page on OK or Cancel.
     */
    public void activatePageWithCallback(String returnPageName, IRequestCycle cycle) {
        // Set the callback
        ICallback callback = new PageCallback(returnPageName);
        setCallback(callback);

        // Go to the page
        cycle.activate(this);        
    }

}
