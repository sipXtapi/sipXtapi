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
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

/**
 * DialingRule
 */
public class DialingRule extends BeanWithId implements IDialingRule {
    private boolean m_enabled;
    private String m_name;
    private String m_description;
    private List m_gateways = new ArrayList();
    private List m_permissions = new ArrayList();

    DialingRule(Integer id) {
        super(id);
    }

    public DialingRule() {
        // intentionally empty
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public List getGateways() {
        return m_gateways;
    }

    public void setGateways(List gateways) {
        m_gateways = gateways;
    }

    public String[] getPatterns() {
        return new String[] {};
    }

    public Transform[] getTransforms() {
        return new Transform[] {};
    }

    public List getPermissions() {
        return m_permissions;
    }

    public void setPermissions(List permissions) {
        m_permissions = permissions;
    }
    
    /**
     * @return list of Gateway objects representing source hosts
     */
    public List getHosts() {
        return null;
    }

    public boolean addGateway(Gateway gateway) {
        if (!m_gateways.remove(gateway)) {
            m_gateways.add(gateway);
            return true;
        }
        return false;
    }

    public void removeGateways(Collection selectedGateways) {
        for (Iterator i = selectedGateways.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            m_gateways.remove(new Gateway(id));
        }
    }
}
