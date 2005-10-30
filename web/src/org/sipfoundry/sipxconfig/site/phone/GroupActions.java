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
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtil;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptGroup;
import org.sipfoundry.sipxconfig.phone.PhoneManager;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.site.setting.BulkGroupAction;

public abstract class GroupActions extends BaseComponent {
    public abstract PhoneManager getPhoneContext();

    public abstract Collection getSelectedPhoneIds();

    public abstract Collection getGroups();

    public abstract Integer getRemoveFromGroupId();

    public abstract IActionListener getSelectedAction();

    public abstract void setSelectedAction(IActionListener action);

    public abstract IPropertySelectionModel getGroupModel();

    public abstract void setGroupModel(IPropertySelectionModel model);

    public abstract TapestryUtil getTapestry();

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

        Group removeFromGroup = null;
        for (Iterator i = groups.iterator(); i.hasNext();) {
            Group g = (Group) i.next();
            if (g.getId().equals(getRemoveFromGroupId())) {
                // do not add the "remove from" group...
                removeFromGroup = g;
                continue;
            }
            if (actions.size() == 0) {
                actions.add(new OptGroup("Add to:"));
            }
            actions.add(new AddToPhoneGroupAction(g, getPhoneContext()));
        }

        if (removeFromGroup != null) {
            actions.add(new OptGroup("Remove from:"));
            actions.add(new RemoveFromPhoneGroupAction(removeFromGroup, getPhoneContext()));
        }

        AdaptedSelectionModel model = new AdaptedSelectionModel();
        model.setCollection(actions);

        String label = getMessage("label.moreActions");
        IPropertySelectionModel enhanced = getTapestry().addExtraOption(model, label);
        setGroupModel(enhanced);
    }

    private void addToGroup(IRequestCycle cycle) {
        IActionListener a = getSelectedAction();
        if (!(a instanceof BulkGroupAction)) {
            return;
        }
        BulkGroupAction action = (BulkGroupAction) a;
        Collection selectedIds = getSelectedPhoneIds();
        action.setIds(selectedIds);

        action.actionTriggered(this, cycle);
        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils
                .getValidator((AbstractComponent) getPage());
        String msg = format("msg.success", Integer.toString(selectedIds.size()), action.getLabel(
                action, 0));
        validator.recordSuccess(msg);
    }
}
