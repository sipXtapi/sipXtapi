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

import org.apache.tapestry.AbstractComponent;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.components.AssetSelector;
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
        AssetSelector assetSelector = getAssetSelector();
        assetSelector.checkFileUpload();

        if (isValid()) {
            saveValid();
        }
    }

    private boolean isValid() {
        IValidationDelegate delegate = TapestryUtils.getValidator((AbstractComponent) getPage());
        AssetSelector assetSelector = getAssetSelector();
        assetSelector.validateNotEmpty(delegate, getMessage("selector.error"));
        return !delegate.getHasErrors();
    }

    private AssetSelector getAssetSelector() {
        return (AssetSelector) getComponent("musicOnHoldSelector");
    }

    private void saveValid() {
        CallGroupContext context = getCallGroupContext();
        context.setDefaultMusicOnHold(getMusic());
        context.activateParkOrbits();
    }
}
