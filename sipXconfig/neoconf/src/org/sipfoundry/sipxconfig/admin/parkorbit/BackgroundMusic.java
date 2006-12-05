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
package org.sipfoundry.sipxconfig.admin.parkorbit;

import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Setting;

public class BackgroundMusic extends BeanWithGroups {
    private static final String DEFAULT = "default.wav";

    private String m_music = DEFAULT;

    private boolean m_enabled;

    public String getMusic() {
        return m_music;
    }

    public void setMusic(String music) {
        m_music = music;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    @Override
    protected Setting loadSettings() {
        // no settings for Background music
        return null;
    }
}
