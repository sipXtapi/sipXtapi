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

import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;

import org.apache.tapestry.IComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.contrib.table.model.IPrimaryKeyConvertor;
import org.apache.tapestry.contrib.table.model.ITableModel;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.ObjectSourceDataSqueezer;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.line.EditLine;

/**
 * List all the phones/phones for management and details drill-down
 */
public abstract class ManagePhones extends BasePage implements PageRenderListener {

    public static final String PAGE = "ManagePhones";

    private static final String TABLE_MODEL_BEAN = "phoneTableModel";

    /** model of the table */
    public abstract void setPhones(Collection phones);

    public abstract Collection getPhones();

    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selected);

    public abstract void setIdConverter(IPrimaryKeyConvertor cvt);

    public abstract PhoneContext getPhoneContext();

    public abstract Integer getGroupId();

    public abstract void setGroupId(Integer groupId);

    public ITableModel getWrappedTableModel() {
        PhoneTableModel model = (PhoneTableModel) getBeans().getBean(TABLE_MODEL_BEAN);
        model.setGroupId(getGroupId());
        return model.getWrappedTableModel();
    }

    public IComponent getThis() {
        return this;
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
        Phone[] phones = getSelectedPhones();
        for (int i = 0; i < phones.length; i++) {
            context.deletePhone(phones[i]);
        }
    }

    public void generateProfiles(IRequestCycle cycle_) {
        Phone[] selectedPhones = getSelectedPhones();
        if (selectedPhones.length > 0) {
            Collection phones = Arrays.asList(selectedPhones);
            getPhoneContext().generateProfilesAndRestart(phones);
        }
    }

    public void generateAllProfiles(IRequestCycle cycle_) {
        getPhoneContext().generateProfilesAndRestartAll();
    }

    private Phone[] getSelectedPhones() {
        PhoneContext phoneContext = getPhoneContext();

        SelectMap selections = getSelections();
        Iterator phoneIds = selections.getAllSelected().iterator();
        Phone[] phones = new Phone[selections.getAllSelected().size()];
        for (int i = 0; i < phones.length; i++) {
            Integer phoneId = (Integer) phoneIds.next();
            phones[i] = phoneContext.loadPhone(phoneId);
        }

        return phones;
    }

    public void restart(IRequestCycle cycle_) {
        Phone[] selectedPhones = getSelectedPhones();
        if (selectedPhones.length > 0) {
            Collection phones = Arrays.asList(selectedPhones);
            getPhoneContext().restart(phones);
        }
    }

    /**
     * called before page is drawn
     */
    public void pageBeginRender(PageEvent event_) {
        PhoneContext phoneContext = getPhoneContext();

        setIdConverter(new PhoneDataSqueezer(phoneContext));

        // Generate the list of phone items
        setPhones(phoneContext.loadPhones());
        if (getSelections() == null) {
            setSelections(new SelectMap());
        }
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
