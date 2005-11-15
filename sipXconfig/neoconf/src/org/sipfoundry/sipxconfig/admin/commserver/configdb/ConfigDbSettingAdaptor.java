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
package org.sipfoundry.sipxconfig.admin.commserver.configdb;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Map;

import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;

public class ConfigDbSettingAdaptor {
    private ConfigDbParameter m_configDbParameter;

    public boolean set(Setting setting) {
        Hashtable params = new Hashtable();
        setting.acceptVisitor(new ParamsCollector(params));
        int set = m_configDbParameter.set(setting.getProfileName(), params);
        return set == params.size();
    }

    public boolean set(String dbName, Collection settings) {
        Hashtable params = new Hashtable();
        for (Iterator i = settings.iterator(); i.hasNext();) {
            Setting setting = (Setting) i.next();
            setting.acceptVisitor(new ParamsCollector(params));
        }
        int set = m_configDbParameter.set(dbName, params);
        return set == params.size();
    }

    public boolean get(Setting setting) {
        Collection names = new ArrayList();
        setting.acceptVisitor(new NamesCollector(names));
        String[] paramNames = (String[]) names.toArray(new String[names.size()]);
        Hashtable results = m_configDbParameter.get(setting.getProfileName(), paramNames);
        setting.acceptVisitor(new ResultRetriever(results));
        return results.size() == paramNames.length;
    }

    public void setConfigDbParameter(ConfigDbParameter configDbParameter) {
        m_configDbParameter = configDbParameter;
    }

    private static class ParamsCollector extends AbstractSettingVisitor {
        private final Map m_parameters;

        public ParamsCollector(Map parameters) {
            m_parameters = parameters;
        }

        public void visitSetting(Setting setting) {
            Object value = setting.getTypedValue();
            if (value != null) {
                // ignore null values - do not set them
                m_parameters.put(setting.getProfileName(), value);
            }
        }
    }

    private static class NamesCollector extends AbstractSettingVisitor {
        private final Collection m_names;

        public NamesCollector(Collection names) {
            m_names = names;
        }

        public void visitSetting(Setting setting) {
            m_names.add(setting.getProfileName());
        }
    }

    private static class ResultRetriever extends AbstractSettingVisitor {
        private final Map m_parameters;

        public ResultRetriever(Map parameters) {
            m_parameters = parameters;
        }

        public void visitSetting(Setting setting) {
            String profileName = setting.getProfileName();
            Object value = m_parameters.get(profileName);
            if (value != null) {
                setting.setValue(value.toString());
            }
        }
    }
}
