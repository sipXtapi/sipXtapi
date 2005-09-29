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
package org.sipfoundry.sipxconfig.gateway;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class GatewayContextImpl extends HibernateDaoSupport implements GatewayContext,
        BeanFactoryAware {

    private static final String NAME_PROP_NAME = "name";

    private class DuplicateNameException extends UserException {
        private static final String ERROR = "A gateway with name \"{0}\" already exists.";

        public DuplicateNameException(String name) {
            super(ERROR, name);
        }
    }

    private DialPlanContext m_dialPlanContext;

    private BeanFactory m_beanFactory;

    private List m_availableGatewayModels;

    public GatewayContextImpl() {
        super();
    }

    public List getGateways() {
        List gateways = getHibernateTemplate().loadAll(Gateway.class);
        return gateways;
    }

    public Gateway getGateway(Integer id) {
        return (Gateway) getHibernateTemplate().load(Gateway.class, id);

    }

    public void storeGateway(Gateway gateway) {
        // Before storing the gateway, make sure that it has a unique name.
        // Throw an exception if it doesn't.
        HibernateTemplate hibernate = getHibernateTemplate();
        DaoUtils.checkDuplicates(hibernate, gateway, NAME_PROP_NAME, new DuplicateNameException(
                gateway.getName()));

        // Store the updated gateway
        hibernate.saveOrUpdate(gateway);
    }

    public boolean deleteGateway(Integer id) {
        Gateway g = getGateway(id);
        getHibernateTemplate().delete(g);
        return true;
    }

    public void deleteGateways(Collection selectedRows) {
        // remove gateways from rules first
        m_dialPlanContext.removeGateways(selectedRows);
        // remove gateways from the database
        for (Iterator i = selectedRows.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            deleteGateway(id);
        }
    }

    public void propagateGateways(Collection selectedRows) {
        for (Iterator i = selectedRows.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            Gateway g = getGateway(id);
            g.propagate();
        }
    }

    public void propagateAllGateways() {
        List gateways = getGateways();
        for (Iterator i = gateways.iterator(); i.hasNext();) {
            Gateway g = (Gateway) i.next();
            g.propagate();
        }
    }

    public List getGatewayByIds(Collection gatewayIds) {
        List gateways = new ArrayList(gatewayIds.size());
        for (Iterator i = gatewayIds.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            gateways.add(getGateway(id));
        }
        return gateways;
    }

    /**
     * Returns the list of gateways available for a specific rule
     * 
     * @param ruleId id of the rule for which gateways are checked
     * @return collection of available gateways
     */
    public Collection getAvailableGateways(Integer ruleId) {
        DialingRule rule = m_dialPlanContext.getRule(ruleId);
        if (null == rule) {
            return Collections.EMPTY_LIST;
        }
        List allGateways = getGateways();
        return rule.getAvailableGateways(allGateways);
    }

    public void clear() {
        List gateways = getHibernateTemplate().loadAll(Gateway.class);
        getHibernateTemplate().deleteAll(gateways);
    }

    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    public Gateway newGateway(PhoneModel model) {
        Gateway gateway = (Gateway) m_beanFactory.getBean(model.getBeanId(), Gateway.class);
        gateway.setBeanId(model.getBeanId());
        gateway.setModelId(model.getModelId());
        return gateway;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public void setAvailableGatewayModels(List availableGatewayModels) {
        m_availableGatewayModels = availableGatewayModels;
    }

    public List getAvailableGatewayModels() {
        return m_availableGatewayModels;
    }
}
