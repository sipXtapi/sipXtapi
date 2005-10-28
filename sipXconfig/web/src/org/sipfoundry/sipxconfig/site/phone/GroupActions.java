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
package org.sipfoundry.sipxconfig.site.phone;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.apache.tapestry.AbstractComponent;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.ExtraOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Group;

public abstract class GroupActions extends BaseComponent {
    public abstract PhoneContext getPhoneContext();

    public abstract Collection getSelectedPhoneIds();

    public abstract Collection getGroups();

    public abstract IActionListener getSelectedAction();

    public abstract void setSelectedAction(IActionListener action);

    public abstract IPropertySelectionModel getGroupModel();

    public abstract void setGroupModel(IPropertySelectionModel model);

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        initGroupModel();
        // always start with empty group
        setSelectedAction(null);
        super.renderComponent(writer, cycle);
        if (cycle.isRewinding()) {
            addToGroup(cycle);
        }
    }

    private void initGroupModel() {
        if (getGroupModel() != null) {
            return;
        }
        Collection groups = getGroups();
        Collection actions = new ArrayList(groups.size());
        for (Iterator i = groups.iterator(); i.hasNext();) {
            Group g = (Group) i.next();
            actions.add(new AddToPhoneGroupAction(g));
        }

        AdaptedSelectionModel model = new AdaptedSelectionModel();
        model.setCollection(actions);

        ExtraOptionModelDecorator decorator = new ExtraOptionModelDecorator();
        decorator.setExtraLabel("More Actions...");
        decorator.setExtraOption(null);
        decorator.setModel(model);
        setGroupModel(decorator);
    }

    private void addToGroup(IRequestCycle cycle) {
        IActionListener a = getSelectedAction();
        if (!(a instanceof AddToPhoneGroupAction)) {
            return;
        }
        AddToPhoneGroupAction action = (AddToPhoneGroupAction) a;
        Collection selectedIds = getSelectedPhoneIds();
        action.setIds(selectedIds);
        action.setPhoneContext(getPhoneContext());
        action.actionTriggered(this, cycle);
        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils
                .getValidator((AbstractComponent) getPage());
        String msg = format("msg.success", Integer.toString(selectedIds.size()), action.getLabel(
                action, 0));
        validator.recordSuccess(msg);
    }
}
