/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.form.IPropertySelectionModel;

/**
 * Tapestry utilities available to web pages 
 */
public class TapestryContext {
    
    /**
     * Add a option to the dropdown model with a label to instruct the user to make a selection.
     * If not item is selected, your business object method will be explicitly set to null
     */
    public IPropertySelectionModel instructUserToSelect(IPropertySelectionModel model) {   
        ExtraOptionModelDecorator decorated = new ExtraOptionModelDecorator();
        decorated.setExtraLabel("select...");
        decorated.setExtraOption(null);
        decorated.setModel(model);
        
        return decorated;
    }
}
