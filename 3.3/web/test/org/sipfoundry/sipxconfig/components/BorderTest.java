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
package org.sipfoundry.sipxconfig.components;

import junit.framework.TestCase;

import org.apache.tapestry.IPage;
import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.engine.IEngineService;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.CoreContextImpl;
import org.sipfoundry.sipxconfig.site.Visit;

public class BorderTest extends TestCase {

    public void testLogin() {
        Border restricted = new MockBorder(true, true, new Visit());
        try {
            restricted.pageValidate(null);
            fail("should redirect");
        } catch (PageRedirectException e) {
            assertEquals("LoginPage", e.getTargetPageName());
        }
    }

    public void testLoginNotRequired() {
        Border nologin = new MockBorder(true, false, new Visit());
        nologin.pageValidate(null);
    }

    public void testRestricted() {
        Border restricted = new MockBorder(true, true, new MockVisit(false));

        try {
            restricted.pageValidate(null);
            fail("should redirect to login page");
        } catch (PageRedirectException e) {
            assertEquals("Home", e.getTargetPageName());
        }
    }

    public void testRestrictedAdmin() {
        Border restricted = new MockBorder(true, true, new MockVisit(true));

        try {
            restricted.pageValidate(null);
        } catch (PageRedirectException e) {
            fail("unexpected expected");
        }
    }

    public void testUnrestricted() {
        Border unrestricted = new MockBorder(false, true, new MockVisit(false));

        try {
            unrestricted.pageValidate(null);
        } catch (PageRedirectException e) {
            fail("unexpected expected");
        }
    }

    public void testUnrestrictedAdmin() {
        Border unrestricted = new MockBorder(false, true, new MockVisit(true));

        try {
            unrestricted.pageValidate(null);
        } catch (PageRedirectException e) {
            fail("unexpected expected");
        }
    }

    private static class MockVisit extends Visit {
        private final boolean m_admin;

        MockVisit(boolean admin) {
            m_admin = admin;
        }

        public Integer getUserId() {
            return new Integer(5);
        }

        public boolean isAdmin() {
            return m_admin;
        }
    }

    private static class MockBorder extends Border {
        private final boolean m_restricted;
        private final boolean m_loginRequired;
        private final Visit m_visit;

        MockBorder(boolean restricted, boolean loginRequired, Visit visit) {
            m_restricted = restricted;
            m_loginRequired = loginRequired;
            m_visit = visit;
        }

        public boolean isLoginRequired() {
            return m_loginRequired;
        }

        public boolean isRestricted() {
            return m_restricted;
        }

        protected Visit getVisit() {
            return m_visit;
        }

        protected void redirectToLogin(IPage page) {
            throw new PageRedirectException("LoginPage");
        }
        
        public IEngineService getRestartService() {
            return null;
        }

        public CoreContext getCoreContext() {
            return new CoreContextImpl() {
                public int getUsersCount() {
                    return 1;
                }
            };
        }
    }
}
