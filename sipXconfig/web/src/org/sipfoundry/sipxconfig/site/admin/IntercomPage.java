/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.RandomStringUtils;
import org.apache.tapestry.AbstractPage;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEndRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.intercom.Intercom;
import org.sipfoundry.sipxconfig.admin.intercom.IntercomManager;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class IntercomPage extends BasePage
    implements PageBeginRenderListener, PageEndRenderListener {
    
    public static final String PAGE = "Intercom";
    private static final int CODE_LEN = 8;

    public abstract Intercom getIntercom();
    public abstract void setIntercom(Intercom intercom);
    
    public abstract String getGroupsString();
    public abstract void setGroupsString(String groupsString);
    
    public abstract IntercomManager getIntercomManager();    
    public abstract PhoneContext getPhoneContext();
    public abstract SettingDao getSettingDao();
    
    public abstract Collection getGroupsCandidates();
    public abstract void setGroupCandidates(Collection groupsList);
    
    public void buildGroupCandidates(String groupsString) {
        List allGroups = getPhoneContext().getGroups();
        Collection candidates = TapestryUtils.getAutoCompleteCandidates(allGroups, groupsString);
        setGroupCandidates(candidates);        
    }
    
    public void pageBeginRender(PageEvent event) {
        // Look up the Intercom, creating it if necessary
        Intercom intercom = getIntercom();
        if (intercom == null) {
            intercom = getIntercomManager().getIntercom();
            if (intercom == null) {
                throw new RuntimeException("Internal error in IntercomPage: null Intercom object");
            }
            setIntercom(intercom);
        }
        
        // Create a random code if no code has been set
        String code = intercom.getCode();
        if (code == null) {
            intercom.setCode(RandomStringUtils.randomAlphanumeric(CODE_LEN));
        }
    }
    
    // Update the Intercom object if the input data is valid
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        
        if (!cycle.isRewinding()) {
            Intercom intercom = getIntercom();

            if (getGroupsString() == null) {
                List groups = intercom.getGroupsAsList();
                if (groups != null && groups.size() > 0) {
                    String groupsString = getSettingDao().getGroupsAsString(groups); 
                    setGroupsString(groupsString);
                }
            }
        }
        
        super.renderComponent(writer, cycle);
        
        if (cycle.isRewinding()) {
            Intercom intercom = getIntercom();

            // Don't take any actions if the page is not valid
            if (!TapestryUtils.isValid((AbstractPage) getPage())) {
                return;
            }

            String groupsString = getGroupsString();
            if (groupsString != null) {
                List groups = getSettingDao().getGroupsByString(Phone.GROUP_RESOURCE_ID, groupsString);
                intercom.setGroupsAsList(groups);
            }
        }
    }
    
    /**
     * Listeners
     */
    public void commit() {
        // Proceed only if Tapestry validation succeeded
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        
        // If there is an Intercom, then save/update it
        Intercom intercom = getIntercom();
        if (intercom != null) {
            getIntercomManager().saveIntercom(intercom);
        }
    }
}
