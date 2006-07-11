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

import org.apache.commons.lang.ObjectUtils;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.IPropertySelectionRenderer;
import org.apache.tapestry.form.PropertySelection;

/**
 * A version of PropertySelection model that preserves renderer functionality available in
 * Tapestry 3
 */
public abstract class FlexiblePropertySelection extends PropertySelection {

    public abstract IPropertySelectionRenderer getRenderer();

    protected void renderFormComponent(IMarkupWriter writer, IRequestCycle cycle) {
        IPropertySelectionRenderer renderer = getRenderer();
        renderDelegatePrefix(writer, cycle);
        renderer.beginRender(this, writer, cycle);
        renderDelegateAttributes(writer, cycle);

        if (isDisabled()) {
            writer.attribute("disabled", "true");
        }

        if (getSubmitOnChange()) {
            writer.attribute("onchange", "javascript: this.form.events.submit();");
        }

        renderIdAttribute(writer, cycle);

        renderDelegateAttributes(writer, cycle);

        getValidatableFieldSupport().renderContributions(this, writer, cycle);

        // Apply informal attributes.
        renderInformalParameters(writer, cycle);

        writer.println();

        IPropertySelectionModel model = getModel();
        int count = model.getOptionCount();

        boolean foundSelected = false;

        Object value = getValue();

        for (int i = 0; i < count; i++) {
            Object option = model.getOption(i);
            boolean selected = !foundSelected && ObjectUtils.equals(option, value);

            if (selected) {
                foundSelected = true;
            }

            renderer.renderOption(this, writer, cycle, model, option, i, selected);
        }

        renderer.endRender(this, writer, cycle);
        renderDelegateSuffix(writer, cycle);
    }
}
