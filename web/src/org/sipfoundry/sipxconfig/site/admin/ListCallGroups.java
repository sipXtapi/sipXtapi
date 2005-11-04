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

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.contrib.table.model.IPrimaryKeyConvertor;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.ObjectSourceDataSqueezer;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class ListCallGroups extends BasePage implements PageRenderListener {

    public static final String PAGE = "ListCallGroups";

    public abstract CallGroupContext getCallGroupContext();

    public abstract CoreContext getCoreContext();

    public void pageBeginRender(PageEvent event_) {
        // initialize properties
    }

    public abstract CallGroup getCurrentRow();

    public abstract void setCurrentRow(CallGroup cd);

    public abstract Collection getRowsToDelete();

    public abstract Collection getRowsToDuplicate();

    public void add(IRequestCycle cycle) {
        EditCallGroup editCallGroup = (EditCallGroup) cycle.getPage(EditCallGroup.PAGE);
        editCallGroup.setCallGroupId(null);
        editCallGroup.setCallGroup(null);
        cycle.activate(editCallGroup);
    }

    public void edit(IRequestCycle cycle) {
        EditCallGroup editCallGroup = (EditCallGroup) cycle.getPage(EditCallGroup.PAGE);
        Integer callGroupId = TapestryUtils.getBeanId(cycle);
        editCallGroup.setCallGroupId(callGroupId);
        editCallGroup.setCallGroup(null);
        cycle.activate(editCallGroup);
    }

    public void formSubmit(IRequestCycle cycle_) {
        delete();
        duplicate();
    }

    /**
     * Deletes all selected rows (on this screen deletes call groups).
     */
    private void delete() {
        Collection selectedRows = getRowsToDelete();
        if (null != selectedRows) {
            getCallGroupContext().removeCallGroups(selectedRows);
        }
    }

    /**
     * Duplicates all selected rows (on this screen duplicates call groups).
     */
    private void duplicate() {
        Collection selectedRows = getRowsToDuplicate();
        if (null != selectedRows) {
            getCallGroupContext().duplicateCallGroups(selectedRows);
        }
    }

    public IPrimaryKeyConvertor getIdConverter() {
        return new ObjectSourceDataSqueezer(getCoreContext(), CallGroup.class);
    }
}
