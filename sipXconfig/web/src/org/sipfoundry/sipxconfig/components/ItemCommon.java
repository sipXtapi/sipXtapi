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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IFormComponent;

public abstract class ItemCommon extends BaseComponent {
    public abstract int getMaxDescriptionLen();

    /**
     * Validates description length.
     * 
     * This is less ideal than validating inside of TextArea component because by the time we are
     * validating it the description has been already set on the bound value, but it should be
     * good enough until we use self-validating TextArea
     */
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        IFormComponent description = (IFormComponent) getComponent("description");
        if (!cycle.isRewinding() || !description.getForm().isRewinding()) {
            // if the page or the form are not rewinding we have nothing else to do
            return;
        }
        int maxDescriptionLen = getMaxDescriptionLen();
        if (maxDescriptionLen > 0) {
            StringSizeValidator validator = new StringSizeValidator();
            validator.setMax(maxDescriptionLen);
            validator.setComponent(description);
            validator.validate(description.getForm().getDelegate());
        }
    }
}
