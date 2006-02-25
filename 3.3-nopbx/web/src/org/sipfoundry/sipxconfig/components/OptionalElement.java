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

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.AbstractComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;

/**
 * It is similar to Any component. It renders the element and the element's content, or it renders
 * content only if the element is empty
 */
public abstract class OptionalElement extends AbstractComponent {

    public abstract void setElement(String element);

    public abstract String getElement();

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        String element = getElement();

        if (!cycle.isRewinding() && StringUtils.isNotBlank(element)) {
            writer.begin(element);

            renderInformalParameters(writer, cycle);
        }

        renderBody(writer, cycle);

        if (!cycle.isRewinding() && StringUtils.isNotBlank(element)) {
            writer.end(element);
        }
    }
}
