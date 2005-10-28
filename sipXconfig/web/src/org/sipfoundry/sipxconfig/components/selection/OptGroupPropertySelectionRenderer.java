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
package org.sipfoundry.sipxconfig.components.selection;

import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.IPropertySelectionRenderer;
import org.apache.tapestry.form.PropertySelection;

public class OptGroupPropertySelectionRenderer implements IPropertySelectionRenderer {
    protected static final String LABEL_ATTR = "label";
    protected static final String DISABLED_ATTR = "disabled";
    protected static final String OPTION_ELEM = "option";
    protected static final String VALUE_ATTR = "value";
    protected static final String SELECTED_ATTR = "selected";

    private boolean m_insideOptGroup;

    protected final boolean isInsideOptGroup() {
        return m_insideOptGroup;
    }

    public final void beginRender(PropertySelection component, IMarkupWriter writer,
            IRequestCycle cycle_) {
        m_insideOptGroup = false;
        writer.begin("select");
        writer.attribute("name", component.getName());

        if (component.isDisabled()) {
            writer.attribute(DISABLED_ATTR, true);
        }

        if (component.getSubmitOnChange()) {
            writer.attribute("onchange", "javascript:this.form.submit();");
        }
        writer.println();
    }

    public final void renderOption(PropertySelection component_, IMarkupWriter writer,
            IRequestCycle cycle_, IPropertySelectionModel model, Object option, int index,
            boolean selected) {
        if (option == null) {
            renderOptGroup(writer, model, index, selected);
        } else {
            renderNormalOption(writer, model, index, selected);
        }
    }

    public final void endRender(PropertySelection component_, IMarkupWriter writer,
            IRequestCycle cycle_) {
        endOptionGroup(writer);
        writer.end(); // <select>
    }

    protected void renderNormalOption(IMarkupWriter writer, IPropertySelectionModel model,
            int index, boolean selected) {
        writer.begin(OPTION_ELEM);
        writer.attribute(VALUE_ATTR, model.getValue(index));

        if (selected) {
            writer.attribute(SELECTED_ATTR, true);
        }
        writer.print(model.getLabel(index));
        writer.end();
        writer.println();
    }

    private void renderOptGroup(IMarkupWriter writer, IPropertySelectionModel model, int index,
            boolean selected) {
        // this is really option group
        endOptionGroup(writer);
        renderOptGroupStart(writer, model, index, selected);
        m_insideOptGroup = true;
    }

    protected void renderOptGroupStart(IMarkupWriter writer, IPropertySelectionModel model,
            int index, boolean selected_) {
        writer.begin("optgroup");
        writer.attribute(LABEL_ATTR, model.getLabel(index));
        writer.println();
    }

    protected void endOptionGroup(IMarkupWriter writer) {
        if (m_insideOptGroup) {
            renderOptGroupEnd(writer);
            m_insideOptGroup = false;
        }
    }

    protected void renderOptGroupEnd(IMarkupWriter writer) {
        writer.end();
    }
}
