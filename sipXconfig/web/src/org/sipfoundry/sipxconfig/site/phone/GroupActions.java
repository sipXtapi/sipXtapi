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

import java.util.Collection;

import org.apache.tapestry.AbstractComponent;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.ExtraOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Group;

public abstract class GroupActions extends BaseComponent {
    public abstract PhoneContext getPhoneContext();

    public abstract Collection getSelectedPhoneIds();

    public abstract Collection getGroups();

    public abstract Integer getSelectedGroupId();

    public abstract void setSelectedGroupId(Integer groupId);

    public abstract IPropertySelectionModel getGroupModel();

    public abstract void setGroupModel(IPropertySelectionModel model);

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        initGroupModel();
        // always start with empty group
        setSelectedGroupId(null);
        super.renderComponent(writer, cycle);
        if (cycle.isRewinding()) {
            addToGroup();
        }
    }

    private void initGroupModel() {
        if (getGroupModel() != null) {
            return;
        }
        ObjectSelectionModel model = new ObjectSelectionModel();
        model.setCollection(getGroups());
        model.setLabelExpression("name");
        model.setValueExpression("id");

        ExtraOptionModelDecorator decorator = new ExtraOptionModelDecorator();
        decorator.setExtraLabel("More Actions:");
        decorator.setExtraOption(null);
        decorator.setModel(model);
        setGroupModel(decorator);
    }

    private void addToGroup() {
        Integer groupId = getSelectedGroupId();
        if (groupId == null) {
            return;
        }
        Collection selectedIds = getSelectedPhoneIds();
        getPhoneContext().addToGroup(groupId, selectedIds);
        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils
                .getValidator((AbstractComponent) getPage());
        Group group = (Group) getPhoneContext().load(Group.class, groupId);
        String msg = format("msg.success", Integer.toString(selectedIds.size()), group.getName());
        validator.recordSuccess(msg);
    }
}
