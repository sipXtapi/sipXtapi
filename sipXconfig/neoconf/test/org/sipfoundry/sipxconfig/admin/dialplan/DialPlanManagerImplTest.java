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

import java.util.Collections;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.easymock.classextension.MockClassControl;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;
import org.springframework.orm.hibernate3.HibernateTemplate;

/**
 * DialPlanManagerImplTest
 */
public class DialPlanManagerImplTest extends TestCase {

    public void testActivateDialPlan() throws Exception {
        DialPlanManagerImpl manager = new MockDialPlanContextImpl(new DialPlan());

        final ConfigGenerator g1 = manager.getGenerator();
        final ConfigGenerator g2 = manager.generateDialPlan();
        final ConfigGenerator g3 = manager.getGenerator();
        assertNotNull(g1);
        assertNotNull(g2);
        assertNotSame(g1, g2);
        assertSame(g2, g3);
    }

    public void testMoveRules() throws Exception {
        MockControl mock = MockClassControl.createNiceControl(DialPlan.class);
        DialPlan plan = (DialPlan) mock.getMock();
        plan.moveRules(Collections.singletonList(new Integer(5)), 3);
        mock.replay();

        MockDialPlanContextImpl manager = new MockDialPlanContextImpl(plan);
        manager.moveRules(Collections.singletonList(new Integer(5)), 3);
        mock.verify();
    }

    private static class MockDialPlanContextImpl extends DialPlanManagerImpl {
        private DialPlan m_plan;

        MockDialPlanContextImpl(DialPlan plan) {
            m_plan = plan;
            MockControl mock = MockClassControl.createNiceControl(HibernateTemplate.class);
            setHibernateTemplate((HibernateTemplate) mock.getMock());
        }

        DialPlan getDialPlan() {
            return m_plan;
        }
        
        public EmergencyRouting getEmergencyRouting() {
            return new EmergencyRouting();
        }
    }
}
