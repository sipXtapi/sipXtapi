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
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Collection;

import org.apache.tapestry.AbstractComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.components.StringSizeValidator;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class EditCallGroup extends BasePage implements PageRenderListener {
    public static final String PAGE = "EditCallGroup";

    public abstract CallGroupContext getCallGroupContext();

    public abstract Integer getCallGroupId();

    public abstract void setCallGroupId(Integer id);

    public abstract CallGroup getCallGroup();

    public abstract void setCallGroup(CallGroup callGroup);

    public abstract boolean getCommitChanges();

    public void pageBeginRender(PageEvent event_) {
        CallGroup callGroup = getCallGroup();
        if (null != callGroup) {
            return;
        }
        Integer id = getCallGroupId();
        if (null != id) {
            CallGroupContext context = getCallGroupContext();
            callGroup = context.loadCallGroup(id);
        } else {
            callGroup = new CallGroup();
        }
        setCallGroup(callGroup);
    }

    /**
     * Called when any of the submit componens on the form is activated.
     * 
     * Usually submit components are setting propertied. formSubmit will first check if the form
     * is valid, then it will call all the "action" listeners. Only one of the listeners (the one
     * that recongnizes the property that is set) will actually do something. This is a bit
     * strange consequnce of the fact that Tapestry listeners are pretty much usuless because they
     * are called while the form is still rewinding and not all changes are commited to beans.
     * 
     * @param cycle current request cycle
     */
    public void formSubmit(IRequestCycle cycle) {
        if (!isValid()) {
            return;
        }
        UserRingTable ringTable = getUserRingTable();
        delete(ringTable);
        move(ringTable);
        addRow(cycle, ringTable);

        if (getCommitChanges()) {
            saveValid();
            getCallGroupContext().activateCallGroups();
            cycle.activate(ListCallGroups.PAGE);
        }
    }

    /**
     * Saves current call group and displays add ring page.
     * 
     * @param cycle current request cycle
     * @param ringTable component with table of rings
     */
    private void addRow(IRequestCycle cycle, UserRingTable ringTable) {
        if (!ringTable.getAddRow()) {
            return;
        }
        saveValid();
        AddUserRing page = (AddUserRing) cycle.getPage(AddUserRing.PAGE);
        page.setCallGroupId(getCallGroupId());
        cycle.activate(page);
    }

    private boolean isValid() {
        IValidationDelegate delegate = TapestryUtils.getValidator(this);
        AbstractComponent component = (AbstractComponent) getComponent("common");
        StringSizeValidator descriptionValidator = (StringSizeValidator) component.getBeans()
                .getBean("descriptionValidator");
        descriptionValidator.validate(delegate);
        return !delegate.getHasErrors();
    }

    private void saveValid() {
        CallGroupContext context = getCallGroupContext();
        CallGroup callGroup = getCallGroup();
        context.storeCallGroup(callGroup);
        Integer id = getCallGroup().getId();
        setCallGroupId(id);
    }

    /**
     * 
     * CHECKSTYLE: this method should be private, but checkstyle complains it is unused
     */
    private void delete(UserRingTable ringTable) {
        Collection ids = ringTable.getRowsToDelete();
        if (null == ids) {
            return;
        }
        CallGroup callGroup = getCallGroup();
        callGroup.removeRings(ids);
    }

    private void move(UserRingTable ringTable) {
        int step = -1;
        Collection ids = ringTable.getRowsToMoveUp();
        if (null == ids) {
            step = 1;
            ids = ringTable.getRowsToMoveDown();
            if (null == ids) {
                // nothing to do
                return;
            }
        }
        CallGroup callGroup = getCallGroup();
        callGroup.moveRings(ids, step);
    }

    private UserRingTable getUserRingTable() {
        return (UserRingTable) getComponent("ringTable");
    }
}
