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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IForm;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.setting.FilterRunner;
import org.sipfoundry.sipxconfig.setting.RenderProperties;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingRenderer;

public abstract class SettingsForm extends BaseComponent {

    private static final RenderProperties DEFAULT_RENDER_PROPERTIES = new RenderProperties();

    public abstract SettingRenderer getSettingRenderer();

    public abstract Setting getCurrentSetting();

    public abstract Setting getSettings();

    public abstract IActionListener getAction();

    public abstract IActionListener getCancelListener();

    public Collection getFlattenedSettings() {
        return FilterRunner.filter(SettingFilter.ALL, getSettings());
    }

    public RenderProperties getCurrentRenderProperties() {
        RenderProperties rprops = DEFAULT_RENDER_PROPERTIES;
        SettingRenderer render = getSettingRenderer();
        if (render != null) {
            RenderProperties customprops = render.getRenderProperties(getCurrentSetting());
            if (customprops != null) {
                rprops = customprops;
            }
        }

        return rprops;
    }

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
        if (null == action) {
            return;
        }
        if (action == getCancelListener() || isValid()) {
            action.actionTriggered(this, cycle);
        }
    }

    boolean isValid() {
        IForm form = (IForm) getComponent("settingsForm");
        IValidationDelegate delegate = form.getDelegate();
        return !delegate.getHasErrors();
    }
}
