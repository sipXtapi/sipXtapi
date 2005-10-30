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

import org.apache.tapestry.AbstractPage;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class MusicOnHoldForm extends BaseComponent implements PageRenderListener {
    public abstract boolean getCommitChanges();

    public abstract CallGroupContext getCallGroupContext();

    public abstract String getMusic();

    public abstract void setMusic(String music);

    public void pageBeginRender(PageEvent event_) {
        String music = getMusic();
        if (null == music) {
            music = getCallGroupContext().getDefaultMusicOnHold();
            setMusic(music);
        }
    }

    public void formSubmit(IRequestCycle cycle_) {
        if (!getCommitChanges()) {
            return;
        }

        if (TapestryUtils.isValid((AbstractPage) getPage())) {
            saveValid();
        }
    }

    private void saveValid() {
        CallGroupContext context = getCallGroupContext();
        context.setDefaultMusicOnHold(getMusic());
        context.activateParkOrbits();
    }
}
