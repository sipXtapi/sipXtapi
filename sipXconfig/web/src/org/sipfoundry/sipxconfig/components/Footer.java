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

public abstract class Footer extends BaseComponent {
    public abstract SkinControl getSkin();

    // TODO: it would be better if we can make skin to intercept resolution of messages...
    public String getProductCopyright() {
        String copyright = getSkin().getCopyright();
        if (copyright != null) {
            return copyright;
        }
        return getMessages().getMessage("product.copyright");
    }

    public String getProductName() {
        String productName = getSkin().getProductName();
        if (productName != null) {
            return productName;
        }
        return getMessages().getMessage("product.name");
    }
}
