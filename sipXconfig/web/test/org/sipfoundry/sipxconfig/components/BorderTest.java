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

import javax.servlet.http.HttpServletRequest;

import org.easymock.MockControl;

import junit.framework.TestCase;

public class BorderTest extends TestCase {
    private HttpServletRequest createRequestMock(boolean isSuper, boolean isEndUSer) {
        MockControl control = MockControl.createControl(HttpServletRequest.class);
        control.setDefaultMatcher(MockControl.EQUALS_MATCHER);
        HttpServletRequest request = (HttpServletRequest) control.getMock();
        request.isUserInRole("SUPER");
        control.setReturnValue(isSuper, 2);
        request.isUserInRole("END_USER");
        control.setReturnValue(isEndUSer, 2);
        control.replay();
        return request;
    }

    public void testAdmin() {
        HttpServletRequest request = createRequestMock(true, true);

        Border restricted = new RestrictedBorder();
        assertTrue(restricted.checkAuthorization(request));
        Border normal = new UnrestrictedBorder();
        assertTrue(normal.checkAuthorization(request));
    }

    public void testEndUser() {
        HttpServletRequest request = createRequestMock(false, true);

        Border restricted = new RestrictedBorder();
        assertFalse(restricted.checkAuthorization(request));
        Border normal = new UnrestrictedBorder();
        assertTrue(normal.checkAuthorization(request));
    }

    public void testWithoutRole() {
        HttpServletRequest request = createRequestMock(false, false);

        Border restricted = new RestrictedBorder();
        assertFalse(restricted.checkAuthorization(request));
        Border normal = new UnrestrictedBorder();
        assertFalse(normal.checkAuthorization(request));
    }

    static class RestrictedBorder extends Border {
        public boolean isRestricted() {
            return true;
        }
    }

    static class UnrestrictedBorder extends Border {
        public boolean isRestricted() {
            return false;
        }
    }
}
