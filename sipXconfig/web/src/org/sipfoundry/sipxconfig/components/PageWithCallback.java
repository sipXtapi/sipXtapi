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

import org.apache.tapestry.IPage;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.html.BasePage;

public abstract class PageWithCallback extends BasePage {

    public abstract ICallback getCallback();

    public abstract void setCallback(ICallback callback);

    /**
     * Set a callback that will navigate back to the named return page on OK or Cancel.
     */
    public void setReturnPage(String returnPageName) {
        ICallback callback = new PageCallback(returnPageName);
        setCallback(callback);
    }

    public void setReturnPage(IPage returnPage) {
        ICallback callback = new PageCallback(returnPage);
        setCallback(callback);
    }
}
