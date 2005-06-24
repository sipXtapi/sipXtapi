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

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

public class GatewayContextImpl extends HibernateDaoSupport implements GatewayContext,
        BeanFactoryAware {

    private DialPlanContext m_dialPlanContext;

    private BeanFactory m_beanFactory;

    private Map m_factoryIds;

    private String m_configDirectory;

    public GatewayContextImpl() {
        super();
        // TODO Auto-generated constructor stub
    }

    public List getGateways() {
        List gateways = getHibernateTemplate().loadAll(Gateway.class);
        return gateways;
    }

    public Gateway getGateway(Integer id) {
        return (Gateway) getHibernateTemplate().load(Gateway.class, id);

    }

    public void storeGateway(Gateway gateway) {
        getHibernateTemplate().saveOrUpdate(gateway);
    }

    public boolean deleteGateway(Integer id) {
        Gateway g = getGateway(id);
        getHibernateTemplate().delete(g);
        return true;
    }

    public void deleteGateways(Collection selectedRows) {
        // remove gateways from rules first
        List rules = m_dialPlanContext.getRules();
        for (Iterator i = rules.iterator(); i.hasNext();) {
            DialingRule rule = (DialingRule) i.next();
            rule.removeGateways(selectedRows);
            m_dialPlanContext.storeRule(rule);
        }
        // remove gateways from the database
        for (Iterator i = selectedRows.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            deleteGateway(id);
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

    /**
     * Loads settings model from XML file
     * 
     * The full path of the model file is: systemEtcDurectory/manufacturere/basename
     * 
     * @return new copy of the settings model
     */
    public Setting loadModelFile(String manufacturer, String basename) {
        File modelDir = new File(m_configDirectory, manufacturer);
        File modelFile = new File(modelDir, basename);
        XmlModelBuilder builder = new XmlModelBuilder(m_configDirectory);
        SettingSet model = builder.buildModel(modelFile);
        return model.copy();
    }

    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    public Gateway newGateway(String factoryId) {
        Gateway gateway = (Gateway) m_beanFactory.getBean(factoryId, Gateway.class);
        return gateway;
    }

    public Map getFactoryIds() {
        return m_factoryIds;
    }

    public void setFactoryIds(Map factoryIds) {
        m_factoryIds = factoryIds;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }
}
