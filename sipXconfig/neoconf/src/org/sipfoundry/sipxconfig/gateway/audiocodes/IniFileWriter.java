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

import java.io.PrintWriter;
import java.io.Writer;
import java.text.MessageFormat;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingVisitor;

public class IniFileWriter implements SettingVisitor {
    private static final String SETTING = "{0} = {1}";
    private static final String GROUP = "[{0}{1}]";

    private PrintWriter m_writer;
    
    private boolean m_firstGroup;

    private String m_groupNameSuffix = StringUtils.EMPTY;

    public IniFileWriter(Writer writer) {
        m_writer = new PrintWriter(writer);
    }

    public void visitSetting(Setting setting) {
        Object[] params = new String[] {
            setting.getProfileName(), setting.getValue()
        };
        m_writer.println(MessageFormat.format(SETTING, params));
    }

    public boolean visitSettingGroup(Setting group) {
        // skip root - empty name
        if (StringUtils.isEmpty(group.getName())) {
            return true;
        }
        Object[] params = new String[] {
            group.getProfileName(), m_groupNameSuffix
        };
        if (m_firstGroup) {
            m_writer.println();
        } else {
            m_firstGroup = true;
        }
        m_writer.println(MessageFormat.format(GROUP, params));
        m_writer.println();
        return true;
    }

    public void setGroupNameSuffix(String groupNameSuffix) {
        m_groupNameSuffix = groupNameSuffix;
    }
}
