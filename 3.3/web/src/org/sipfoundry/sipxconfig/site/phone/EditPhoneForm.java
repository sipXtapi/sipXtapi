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
package org.sipfoundry.sipxconfig.site.phone;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IFormComponent;
import org.sipfoundry.sipxconfig.components.StringSizeValidator;

public abstract class EditPhoneForm extends BaseComponent {
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        IFormComponent description = (IFormComponent) getComponent("phoneDescription");
        if (!cycle.isRewinding() || !description.getForm().isRewinding()) {
            // if the page or the form are not rewinding we have nothing else to do
            return;
        }
        StringSizeValidator validator = new StringSizeValidator();
        validator.setComponent(description);
        validator.validate(description.getForm().getDelegate());
    }
}
