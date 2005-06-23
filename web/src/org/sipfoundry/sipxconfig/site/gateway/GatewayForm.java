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
package org.sipfoundry.sipxconfig.site.gateway;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.components.StringSizeValidator;

public abstract class GatewayForm extends BaseComponent {
    /**
     * Should be called when page wants to validate its components (typically from isValid)
     */
    public void validate(IValidationDelegate delegate) {
        StringSizeValidator descriptionValidator = (StringSizeValidator) getBeans().getBean(
                "descriptionValidator");
        descriptionValidator.validate(delegate);
    }
}
