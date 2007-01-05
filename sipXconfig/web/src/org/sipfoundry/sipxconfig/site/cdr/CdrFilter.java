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
package org.sipfoundry.sipxconfig.site.cdr;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.cdr.CdrSearch.Mode;
import org.sipfoundry.sipxconfig.components.LocalizedOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.NewEnumPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryContext;

public abstract class CdrFilter extends BaseComponent {

    public abstract boolean getSearchMode();

    public abstract TapestryContext getTapestry();

    public abstract void setSelectionModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getSelectionModel();

    protected void prepareForRender(IRequestCycle cycle) {
        if (getSelectionModel() == null) {
            NewEnumPropertySelectionModel model = new NewEnumPropertySelectionModel();
            model.setEnumType(Mode.class);

            LocalizedOptionModelDecorator decoratedModel = new LocalizedOptionModelDecorator();
            decoratedModel.setMessages(getMessages());
            decoratedModel.setModel(model);
            decoratedModel.setResourcePrefix("filter.");

            setSelectionModel(getTapestry().addExtraOption(decoratedModel, getMessages(),
                    "label.filter"));
        }
    }
}
