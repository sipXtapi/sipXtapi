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
import org.springframework.context.ApplicationContext;

/**
 * FlexibleDialPlanTestDb
 */
public class FlexibleDialPlanTestDb extends TestCase {
    private FlexibleDialPlanContext m_plan;

    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_plan = (FlexibleDialPlanContext) appContext
                .getBean(FlexibleDialPlanContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
    }

    public void testAddDeleteRule() {
        // TODO - replace with IDialingRule mocks

        DialingRule r1 = new CustomDialingRule();
        r1.setName("a1");
        DialingRule r2 = new CustomDialingRule();
        r2.setName("a2");

        m_plan.storeRule(r1);
        m_plan.storeRule(r1);
        assertEquals(1, m_plan.getRules().size());
        m_plan.storeRule(r2);
        assertEquals(2, m_plan.getRules().size());

        Integer id1 = r1.getId();
        m_plan.deleteRules(Collections.singletonList(id1));
        Collection rules = m_plan.getRules();
        assertTrue(rules.contains(r2));
        assertFalse(rules.contains(r1));
        assertEquals(1, rules.size());
    }

    public void testDefaultRuleTypes() throws Exception {
        m_plan.resetToFactoryDefault();

        IncludeTableFilter filter = new IncludeTableFilter();
        filter.includeTable("*dialing_rule");

        IDataSet set = new FilteredDataSet(filter, TestHelper.getConnection().createDataSet());
        ITable table = set.getTable("dialing_rule");
        assertEquals(7, table.getRowCount());
        // FIXME: test agains the real data - need to remove ids...
        // IDataSet reference = new FilteredDataSet(filter, TestHelper
        // .loadDataSet("admin/dialplan/dbdata/defaultFlexibleDialPlan.xml"));
        // Assertion.assertEquals(set, reference);
    }

    public void testDuplicateRules() throws Exception {
        DialingRule r1 = new CustomDialingRule();
        r1.setName("a1");
        m_plan.storeRule(r1);
        assertFalse(BeanWithId.UNSAVED_ID.equals(r1.getId()));

        m_plan.duplicateRules(Collections.singletonList(r1.getId()));

        assertEquals(2, m_plan.getRules().size());

        IDataSet set = TestHelper.getConnection().createDataSet();
        ITable table = set.getTable("dialing_rule");
        assertEquals(2, table.getRowCount());
    }

    public void testDuplicateDefaultRules() throws Exception {
        m_plan.resetToFactoryDefault();
        
        List rules = m_plan.getRules();
        
        Transformer bean2id = new BeanWithId.BeanToId();
        
        Collection ruleIds = CollectionUtils.collect(rules, bean2id);
        
        m_plan.duplicateRules(ruleIds);

        assertEquals(ruleIds.size() * 2, m_plan.getRules().size());

        IDataSet set = TestHelper.getConnection().createDataSet();
        ITable table = set.getTable("dialing_rule");
        assertEquals(ruleIds.size() * 2, table.getRowCount());
    }

}
