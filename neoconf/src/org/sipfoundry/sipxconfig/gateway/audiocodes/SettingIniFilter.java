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
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import java.io.FilterWriter;
import java.io.IOException;
import java.io.Writer;
import java.text.MessageFormat;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.sipfoundry.sipxconfig.setting.Setting;

public class SettingIniFilter extends FilterWriter {
    private static final Pattern GROUP = Pattern.compile("^\\[(.*?)\\]\\s*$");
    private static final Pattern SETTING = Pattern.compile("^\\s*([^;].*?)\\s*=.*$");
    private static final String FORMAT = "{0} = {1}";

    private Map m_names2groups;
    private Map m_names2settings;

    public SettingIniFilter(Writer out, Setting root) {
        super(out);
        m_names2groups = createNames2Settings(root);
    }

    private Map createNames2Settings(Setting parent) {
        Collection sets = parent.getValues();
        Map names2sets = new HashMap(sets.size());
        for (Iterator iter = sets.iterator(); iter.hasNext();) {
            Setting setting = (Setting) iter.next();
            names2sets.put(setting.getProfileName(), setting);
        }
        return names2sets;
    }

    public void write(String str) throws IOException {
        if (!filterGroup(str) && !filterSetting(str)) {
            super.write(str);
        }
    }

    /**
     * Checks for [group] declaration, if it finds one switches active group to the one that we
     * find
     * 
     * @param str line from ini file
     * @return true if it wrote a line, false if line still has to be written
     */
    private boolean filterGroup(String str) throws IOException {
        Matcher matcher = GROUP.matcher(str);
        if (!matcher.find()) {
            return false;
        }
        // new group - reset settings
        m_names2settings = null;
        String groupName = matcher.group(1);
        Setting groupSetting = (Setting) m_names2groups.get(groupName);
        // we know this setting group
        if (groupSetting != null) {
            m_names2settings = createNames2Settings(groupSetting);
        }
        super.write(str);
        return true;
    }

    /**
     * Checks for setting = value lines. If it finds the one for which we have value, it writes
     * out a new value
     * 
     * @param str line from ini file
     * @return true if it wrote a line, false if line still has to be written
     * @throws IOException
     */
    private boolean filterSetting(String str) throws IOException {
        if (m_names2settings == null) {
            // it's an unknown group - no reason to look for settings
            return false;
        }
        Matcher matcher = SETTING.matcher(str);
        if (!matcher.find()) {
            // not a setting line
            return false;
        }
        String settingName = matcher.group(1);
        Setting setting = (Setting) m_names2settings.get(settingName);
        if (setting == null) {
            // not our setting
            return false;
        }
        String line = MessageFormat.format(FORMAT, new Object[] {
            settingName, setting.getValue()
        });
        super.write(line);
        return true;
    }
}
