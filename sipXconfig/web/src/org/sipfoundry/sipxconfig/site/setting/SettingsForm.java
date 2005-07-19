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
package org.sipfoundry.sipxconfig.site.setting;

import java.util.Collection;
import java.util.Iterator;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IForm;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.setting.FilterRunner;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingFilter;

public abstract class SettingsForm extends BaseComponent {

    public abstract Setting getCurrentSetting();

    public abstract Setting getSettings();

    public abstract IActionListener getAction();

    public abstract IActionListener getCancelListener();

    public Collection getFlattenedSettings() {
        return FilterRunner.filter(SettingFilter.ALL, getSettings());
    }

    public abstract boolean getShowAdvanced();

    public abstract void setShowAdvanced(boolean showAdvanced);

    /**
     * Called when any of the submit componens on the form is activated.
     * 
     * Check if cancel was pressed and if so call the cancel action. Otherwise verify if form is
     * valid and only call the cancel action if that's the case.
     * 
     * 
     * @param cycle current request cycle
     */
    public void formSubmit(IRequestCycle cycle) {
        IActionListener action = getAction();
        IValidationDelegate delegate = getValidator();
        if (null == action) {
            delegate.clear();
            return;
        }
        if (action == getCancelListener() || !delegate.getHasErrors()) {
            action.actionTriggered(this, cycle);
        }
    }

    private IValidationDelegate getValidator() {
        IForm form = (IForm) getComponent("settingsForm");
        IValidationDelegate delegate = form.getDelegate();
        return delegate;
    }

    /**
     * Collects ids of avanced settings to be used to refresh only those when toggle advanced link
     * is clicked
     * 
     * @return collection of decorated settings ids
     */
    public boolean getHasAdvancedSettings() {
        Collection flattenedSettings = getFlattenedSettings();
        for (Iterator i = flattenedSettings.iterator(); i.hasNext();) {
            Setting setting = (Setting) i.next();
            if (setting.isAdvanced()) {
                return true;
            }
        }
        return false;
    }
    
    /**
     * Render setting if it's not advanced (hidden) or if show advanced is set
     * @param setting
     * @return true if setting should be rendered
     */
    public boolean renderSetting(Setting setting) {
        return !setting.isAdvanced() || getShowAdvanced();
    }
}
