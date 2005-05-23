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

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Transformer;
import org.dbunit.dataset.FilteredDataSet;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.filter.IncludeTableFilter;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.springframework.context.ApplicationContext;

/**
 * DialPlanManagerTest
 */
public class DialPlanContextTestDb extends TestCase {
    private DialPlanContext m_context;

    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_context = (DialPlanContext) appContext.getBean(DialPlanContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testAddGateway() {
        Gateway g1 = new Gateway();
        Gateway g2 = new Gateway();

        // add g1
        m_context.storeGateway(g1);

        assertEquals(1, m_context.getGateways().size());
        assertTrue(m_context.getGateways().contains(g1));
        assertFalse(m_context.getGateways().contains(g2));

        // add g2
        m_context.storeGateway(g2);

        assertEquals(2, m_context.getGateways().size());
        assertTrue(m_context.getGateways().contains(g1));
        assertTrue(m_context.getGateways().contains(g2));
    }

    public void testDeleteGateway() {
        Gateway g1 = new Gateway();
        Gateway g2 = new Gateway();
        Gateway g3 = new Gateway();

        // add all
        m_context.storeGateway(g1);
        m_context.storeGateway(g2);
        m_context.storeGateway(g3);

        Integer[] toBeRemoved = {
            g1.getId(), g3.getId()
        };
        m_context.deleteGateways(Arrays.asList(toBeRemoved));

        List gateways = m_context.getGateways();

        assertEquals(1, gateways.size());
        assertFalse(gateways.contains(g1));
        assertTrue(gateways.contains(g2));
        assertFalse(gateways.contains(g3));
    }

    public void testUpdateGateway() throws Exception {
        Gateway g1 = new Gateway();
        g1.setAddress("10.1.1.1");
        m_context.storeGateway(g1);
        g1.setAddress("10.1.1.2");
        m_context.storeGateway(g1);
        assertEquals("10.1.1.2", g1.getAddress());
    }

    public void testDeleteGatewayInUse() {
        Gateway g1 = new Gateway();
        g1.setAddress("10.1.1.1");
        m_context.storeGateway(g1);
        InternationalRule rule = new InternationalRule();
        rule.setName("testRule");
        rule.setInternationalPrefix("011");
        rule.addGateway(g1);

        m_context.storeRule(rule);

        // remove gateway
        m_context.deleteGateways(Collections.singletonList(g1.getId()));

        Integer ruleId = rule.getId();

        rule = (InternationalRule) m_context.getRule(ruleId);
        assertTrue(rule.getGateways().isEmpty());
    }

    public void testAddDeleteRule() {
        // TODO - replace with IDialingRule mocks

        DialingRule r1 = new CustomDialingRule();
        r1.setName("a1");
        DialingRule r2 = new CustomDialingRule();
        r2.setName("a2");

        m_context.storeRule(r1);
        m_context.storeRule(r1);
        assertEquals(1, m_context.getRules().size());
        m_context.storeRule(r2);
        assertEquals(2, m_context.getRules().size());

        Integer id1 = r1.getId();
        m_context.deleteRules(Collections.singletonList(id1));
        Collection rules = m_context.getRules();
        assertTrue(rules.contains(r2));
        assertFalse(rules.contains(r1));
        assertEquals(1, rules.size());
    }

    public void testDefaultRuleTypes() throws Exception {
        m_context.resetToFactoryDefault();

        IncludeTableFilter filter = new IncludeTableFilter();
        filter.includeTable("*dialing_rule");

        IDataSet set = new FilteredDataSet(filter, TestHelper.getConnection().createDataSet());
        ITable table = set.getTable("dialing_rule");
        assertEquals(7, table.getRowCount());
        // FIXME: test agains the real data - need to remove ids...
        // IDataSet reference = new FilteredDataSet(filter, TestHelper
        // .loadDataSet("admin/dialplan/defaultFlexibleDialPlan.xml"));
        // Assertion.assertEquals(set, reference);
    }

    public void testDuplicateRules() throws Exception {
        DialingRule r1 = new CustomDialingRule();
        r1.setName("a1");
        m_context.storeRule(r1);
        assertFalse(r1.isNew());

        m_context.duplicateRules(Collections.singletonList(r1.getId()));

        assertEquals(2, m_context.getRules().size());

        IDataSet set = TestHelper.getConnection().createDataSet();
        ITable table = set.getTable("dialing_rule");
        assertEquals(2, table.getRowCount());
    }

    public void testDuplicateDefaultRules() throws Exception {
        m_context.resetToFactoryDefault();
        
        List rules = m_context.getRules();
        
        Transformer bean2id = new BeanWithId.BeanToId();
        
        Collection ruleIds = CollectionUtils.collect(rules, bean2id);
        
        m_context.duplicateRules(ruleIds);

        assertEquals(ruleIds.size() * 2, m_context.getRules().size());

        IDataSet set = TestHelper.getConnection().createDataSet();
        ITable table = set.getTable("dialing_rule");
        assertEquals(ruleIds.size() * 2, table.getRowCount());
    }    
}
