/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig;

import java.util.HashSet;
import java.util.Set;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.easymock.classextension.MockClassControl;

public class EasyMockTest extends TestCase {
    
    public void testBeanWithoutGenerics() {
        MockControl simpleCtrl = MockClassControl.createControl(BeanWithoutGenerics.class);
        simpleCtrl.getMock();        
    }

    public void testBeanWithGenericsDefined() {
        MockControl simpleCtrl = MockClassControl.createControl(BeanWithGenericsDefined.class);
        simpleCtrl.getMock();        
    }
    
    public void testBeanWithGenericsInMethod() {        
        MockControl simpleCtrl = MockClassControl.createControl(BeanWithGenericsInMethod.class);
        simpleCtrl.getMock();        
    }

    public static class BeanWithoutGenerics {
    }
    
    public static class BeanWithGenericsDefined {
        private Set<String> m_set = new HashSet<String>();
        public BeanWithGenericsDefined() {
            m_set = new HashSet<String>();            
        }
        public Object avoidEclipseWarning() {
            return m_set;
        }
    }
    
    public static class BeanWithGenericsInMethod {        
        private Set<String> m_set = new HashSet<String>();
        public BeanWithGenericsInMethod() {
            init();
        }
        protected void init() {
            m_set = new HashSet<String>();            
        }
        public Object avoidEclipseWarning() {
            return m_set;
        }
    }
}
