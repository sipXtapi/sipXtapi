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
package org.sipfoundry.sipxconfig.api;

import junit.framework.TestCase;

public class ApiBeanUtilTest extends TestCase {

    public void testCopyProperties() {
        ApiBean1 one = new ApiBean1();
        one.setFreak("homer");
        ApiBean2 two = new ApiBean2();
        ApiBeanUtil.copyProperties(one, two, null);
        assertEquals("homer", two.getFreak());
    }
    
    public static class ApiBean1 {
        
        private String m_freak;
        
        public String getFreak() {        
            return m_freak;
        }
        
        public void setFreak(String freak) {
            m_freak = freak;
        }
    }
    
    public static class ApiBean2 {
        private String m_freak;
        private String m_ball;
        
        public String getFreak() {        
            return m_freak;
        }
        
        public void setFreak(String freak) {
            m_freak = freak;
        }
        
        public String getBall() {            
            return m_ball;
        }
        
        public void setBall(String ball) {
            m_ball = ball;
        }
    }

}
