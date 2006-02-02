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
import java.util.Iterator;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.IPrimaryKeyConvertor;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.ObjectSourceDataSqueezer;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptGroup;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.ProfileManager;
import org.sipfoundry.sipxconfig.phone.RestartManager;
import org.sipfoundry.sipxconfig.search.SearchManager;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.site.line.EditLine;

/**
 * List all the phones/phones for management and details drill-down
 */
public abstract class ManagePhones extends BasePage implements PageRenderListener {
    public static final String PAGE = "ManagePhones";

    /** model of the table */
    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selected);

    public abstract void setIdConverter(IPrimaryKeyConvertor cvt);

    public abstract PhoneContext getPhoneContext();

    public abstract RestartManager getRestartManager();

    public abstract ProfileManager getProfileManager();

    public abstract Integer getGroupId();

    public abstract void setGroupId(Integer groupId);

    public abstract IPropertySelectionModel getActionModel();

    public abstract void setActionModel(IPropertySelectionModel model);

    public abstract String getQueryText();

    public abstract boolean getSearchMode();

    public abstract SearchManager getSearchManager();

    public IBasicTableModel getTableModel() {
        String queryText = getQueryText();
        if (!getSearchMode() || StringUtils.isBlank(queryText)) {
            return new PhoneTableModel(getPhoneContext(), getGroupId());
        }
        ObjectSourceDataSqueezer squeezer = new PhoneDataSqueezer(getPhoneContext());
        return new SearchPhoneTableModel(getSearchManager(), queryText, squeezer);
    }

    /**
     * When user clicks on link to edit a phone/phone
     */
    public void editPhone(IRequestCycle cycle) {
        EditPhone page = (EditPhone) cycle.getPage(EditPhone.PAGE);
        Object[] params = cycle.getServiceParameters();
        Integer phoneId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        page.setPhoneId(phoneId);
        cycle.activate(page);
    }

    public void editLine(IRequestCycle cycle) {
        Object[] params = cycle.getServiceParameters();
        Integer lineId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        EditLine page = (EditLine) cycle.getPage(EditLine.PAGE);
        page.setLineId(lineId);
        cycle.activate(page);
    }

    public void addPhone(IRequestCycle cycle) {
        NewPhone page = (NewPhone) cycle.getPage(NewPhone.PAGE);
        cycle.activate(page);
    }

    public void deletePhone(IRequestCycle cycle_) {
        PhoneContext context = getPhoneContext();

        Collection ids = getSelections().getAllSelected();
        if (ids.isEmpty()) {
            return;
        }

        for (Iterator i = ids.iterator(); i.hasNext();) {
            Integer phoneId = (Integer) i.next();
            Phone phone = context.loadPhone(phoneId);
            context.deletePhone(phone);
        }

        String msg = getMessages().format("msg.success.delete", Integer.toString(ids.size()));
        TapestryUtils.recordSuccess(this, msg);
    }

    public void generateProfiles(IRequestCycle cycle_) {
        Collection phoneIds = getSelections().getAllSelected();
        generateProfiles(phoneIds);
    }

    public void generateAllProfiles(IRequestCycle cycle_) {
        Collection phoneIds = getPhoneContext().getAllPhoneIds();
        generateProfiles(phoneIds);
    }

    private void generateProfiles(Collection phoneIds) {
        getProfileManager().generateProfilesAndRestart(phoneIds);
        String msg = getMessages().format("msg.success.profiles",
                Integer.toString(phoneIds.size()));
        TapestryUtils.recordSuccess(this, msg);
    }

    public void restart(IRequestCycle cycle_) {
        Collection phoneIds = getSelections().getAllSelected();
        getRestartManager().restart(phoneIds);
        String msg = getMessages().format("msg.success.restart",
                Integer.toString(phoneIds.size()));
        TapestryUtils.recordSuccess(this, msg);
    }

    /**
     * called before page is drawn
     */
    public void pageBeginRender(PageEvent event_) {
        PhoneContext phoneContext = getPhoneContext();

        setIdConverter(new PhoneDataSqueezer(phoneContext));

        // Generate the list of phone items
        if (getSelections() == null) {
            setSelections(new SelectMap());
        }
        initActionsModel();
    }

    private void initActionsModel() {
        Collection groups = getPhoneContext().getGroups();
        Collection actions = new ArrayList(groups.size());

        Group removeFromGroup = null;
        for (Iterator i = groups.iterator(); i.hasNext();) {
            Group g = (Group) i.next();
            if (g.getId().equals(getGroupId())) {
                // do not add the "remove from" group...
                removeFromGroup = g;
                continue;
            }
            if (actions.size() == 0) {
                actions.add(new OptGroup(getMessage("label.addTo")));
            }
            actions.add(new AddToPhoneGroupAction(g, getPhoneContext()));
        }

        if (removeFromGroup != null) {
            actions.add(new OptGroup(getMessage("label.removeFrom")));
            actions.add(new RemoveFromPhoneGroupAction(removeFromGroup, getPhoneContext()));
        }

        AdaptedSelectionModel model = new AdaptedSelectionModel();
        model.setCollection(actions);
        setActionModel(model);
    }

    /**
     * PhoneSummary is not a make up object contructed of and phone and a phone object.
     * reconstruct it here from phone and phonecontext
     */
    static class PhoneDataSqueezer extends ObjectSourceDataSqueezer {

        PhoneDataSqueezer(PhoneContext context) {
            super(context, Phone.class);
        }

        public Object getPrimaryKey(Object objValue) {
            Object pk = null;
            if (objValue != null) {
                pk = ((Phone) objValue).getPrimaryKey();
            }

            return pk;
        }

        public Object getValue(Object objPrimaryKey) {
            Phone phoneMeta = (Phone) super.getValue(objPrimaryKey);
            // reload object due to PhoneContext API (good) restriction
            PhoneContext pc = (PhoneContext) getDataObjectSource();
            Phone phone = pc.loadPhone(phoneMeta.getId());

            return phone;
        }
    }
}
