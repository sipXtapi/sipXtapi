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
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.common.ExtensionPool;
import org.sipfoundry.sipxconfig.common.ExtensionPoolContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class ExtensionPoolsPage extends BasePage {

    public static final String PAGE = "ExtensionPools";

    /**
     * Properties
     */
    
    public abstract ExtensionPoolContext getExtensionPoolContext();
    public abstract void setExtensionPoolContext(ExtensionPoolContext pool);
    
    public abstract ExtensionPool getUserExtensionPool();
    public abstract void setUserExtensionPool(ExtensionPool pool);
    
    public abstract ICallback getCallback();
    public abstract void setCallback(ICallback callback);
    
    /**
     * Listeners
     */  
    public void commit(IRequestCycle cycle_) {
        // Proceed only if Tapestry validation succeeded
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        
        // For now, we are not letting the user edit the pool's nextExtension value,
        // which controls where to start looking for the next free extension.
        // Set it to be the same as the start of the range.
        ExtensionPool pool = getUserExtensionPool();
        pool.setNextExtension(pool.getFirstExtension());
        
        getExtensionPoolContext().saveExtensionPool(pool);
    }

}
