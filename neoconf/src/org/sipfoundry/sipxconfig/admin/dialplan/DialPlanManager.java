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
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

/**
 * DialPlanManager TODO: need interface and hibernate persistence implementation
 */
public class DialPlanManager {
    private List m_dialPlans = new ArrayList();
    private List m_gateways = new ArrayList();

    public List getDialPlans() {
        return m_dialPlans;
    }

    public void setDialPlans(List dialPlans) {
        m_dialPlans = dialPlans;
    }

    public List getGateways() {
        return m_gateways;
    }

    public void setGateways(List gateways) {
        m_gateways = gateways;
    }

    public Gateway getGateway(Integer id) {
        if (null == id) {
            return null;
        }
        Object key = new Gateway(id);
        int i = m_gateways.indexOf(key);
        if (i < 0) {
            return null;
        }
        return (Gateway) m_gateways.get(i);
    }

    public DialPlan getDialPlan(Integer id) {
        if (null == id) {
            return null;
        }
        Object key = new DialPlan(id);
        int i = m_dialPlans.indexOf(key);
        if (i < 0) {
            return null;
        }
        return (DialPlan) m_dialPlans.get(i);
    }

    public boolean updateGateway(Integer id, Gateway gatewayData) {
        Gateway gateway = getGateway(id);
        if (null == gateway) {
            return false;
        }
        gateway.update(gatewayData);
        return true;
    }

    public boolean addGateway(Gateway gateway) {
        if (!m_gateways.remove(gateway)) {
            m_gateways.add(gateway);
            return true;
        }
        return false;
    }

    public boolean addDialPlan(DialPlan dialPlan) {
        if (!m_dialPlans.remove(dialPlan)) {
            m_dialPlans.add(dialPlan);
            return true;
        }
        return false;

    }

    public boolean updateDialPlan(Integer id, DialPlan planData) {
        DialPlan plan = getDialPlan(id);
        if (plan == null) {
            return false;
        }
        plan.update(planData);
        return false;
    }

    public void clear() {
        m_dialPlans.clear();
        m_gateways.clear();
    }

    public boolean deleteGateway(Integer id) {
        return m_gateways.remove(new Gateway(id));
    }

    public void deleteGateways(Collection selectedRows) {
        for (Iterator i = selectedRows.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            deleteGateway(id);
        }
    }

    private void deleteDialPlan(Integer id) {
        m_dialPlans.remove(new DialPlan(id));
    }

    public void deleteDialPlans(Collection selectedRows) {
        for (Iterator i = selectedRows.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            deleteDialPlan(id);
        }
    }

    /**
     * Returns the list of gateways available for a specific dial plan
     * 
     * @param dialPlanId
     * @param emergency if true check emergency gateways, otherwise normal
     *        gateways
     * @return collection of available gateways
     */
    public Collection getAvailableGateways(Integer dialPlanId, boolean emergency) {
        DialPlan plan = getDialPlan(dialPlanId);
        if (null == plan) {
            return Collections.EMPTY_LIST;
        }
        Set gateways = new HashSet(getGateways());
        Set planGateways = emergency ? plan.getEmergencyGateways() : plan.getGateways();
        gateways.removeAll(planGateways);
        return gateways;
    }
}
