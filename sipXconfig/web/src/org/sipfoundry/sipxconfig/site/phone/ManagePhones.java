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

import java.util.ArrayList;
import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptGroup;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.search.SearchManager;
import org.sipfoundry.sipxconfig.setting.Group;

/**
 * List all the phones/phones for management and details drill-down
 */
public abstract class ManagePhones extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "ManagePhones";

    @Bean
    public abstract SelectMap getSelections();

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @InjectObject(value = "spring:searchManager")
    public abstract SearchManager getSearchManager();

    public abstract Integer getGroupId();

    public abstract void setGroupId(Integer groupId);

    public abstract IPropertySelectionModel getActionModel();

    public abstract void setActionModel(IPropertySelectionModel model);

    public abstract String getQueryText();

    public abstract boolean getSearchMode();

    public abstract PhoneModel getPhoneModel();

    public IBasicTableModel getTableModel() {
        String queryText = getQueryText();
        if (!getSearchMode() || StringUtils.isBlank(queryText)) {
            return new PhoneTableModel(getPhoneContext(), getGroupId());
        }
        return new SearchPhoneTableModel(getSearchManager(), queryText, getPhoneContext());
    }

    /**
     * called before page is drawn
     */
    public void pageBeginRender(PageEvent event_) {
        initActionsModel();
    }

    public void formSubmit(IRequestCycle cycle) {
        PhoneModel model = getPhoneModel();
        if (model != null) {
            NewPhone newPhone = (NewPhone) cycle.getPage(NewPhone.PAGE);
            newPhone.setPhoneModel(model);
            cycle.activate(newPhone);
        }
    }

    private void initActionsModel() {
        Collection<Group> groups = getPhoneContext().getGroups();
        Collection actions = new ArrayList(groups.size());

        Group removeFromGroup = null;
        for (Group g : groups) {
            if (g.getId().equals(getGroupId())) {
                // do not add the "remove from" group...
                removeFromGroup = g;
                continue;
            }
            if (actions.size() == 0) {
                actions.add(new OptGroup(getMessages().getMessage("label.addTo")));
            }
            actions.add(new AddToPhoneGroupAction(g, getPhoneContext()));
        }

        if (removeFromGroup != null) {
            actions.add(new OptGroup(getMessages().getMessage("label.removeFrom")));
            actions.add(new RemoveFromPhoneGroupAction(removeFromGroup, getPhoneContext()));
        }

        AdaptedSelectionModel model = new AdaptedSelectionModel();
        model.setCollection(actions);
        setActionModel(model);
    }
}
