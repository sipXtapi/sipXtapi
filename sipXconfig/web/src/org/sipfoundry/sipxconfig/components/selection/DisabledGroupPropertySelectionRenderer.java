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
import org.apache.tapestry.form.IPropertySelectionModel;

public class DisabledGroupPropertySelectionRenderer extends OptGroupPropertySelectionRenderer {
    protected void renderOptGroupStart(IMarkupWriter writer, IPropertySelectionModel model,
            int index, boolean selected) {
        writer.begin(OPTION_ELEM);
        writer.attribute(VALUE_ATTR, model.getValue(index));

        writer.attribute(DISABLED_ATTR, true);
        if (selected) {
            writer.attribute(SELECTED_ATTR, true);
        }

        writer.print(model.getLabel(index));
        writer.end();
        writer.println();
    }

    protected void renderNormalOption(IMarkupWriter writer, IPropertySelectionModel model,
            int index, boolean selected) {
        writer.begin(OPTION_ELEM);
        writer.attribute(VALUE_ATTR, model.getValue(index));
        String label = model.getLabel(index);
        writer.attribute(LABEL_ATTR, label);
        if (selected) {
            writer.attribute(SELECTED_ATTR, true);
        }

        writer.print(label);
        writer.end();
        writer.println();
    }

    protected void renderOptGroupEnd(IMarkupWriter writer_) {
        // do nothing
    }
}
