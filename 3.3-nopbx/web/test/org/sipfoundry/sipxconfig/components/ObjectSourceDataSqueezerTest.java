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

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.common.DataObjectSource;
import org.sipfoundry.sipxconfig.common.PrimaryKeySource;

public class ObjectSourceDataSqueezerTest extends TestCase {
    static final Integer KEY = new Integer(5);
    private PrimaryKeySource m_object;

    protected void setUp() throws Exception {
        m_object = new PrimaryKeySource() {
            public Object getPrimaryKey() {
                return KEY;
            }
        };
    }

    public void testGetPrimaryKey() {
        MockControl control = MockControl.createStrictControl(DataObjectSource.class);
        DataObjectSource source = (DataObjectSource) control.getMock();
        control.replay();

        ObjectSourceDataSqueezer squeezer = new ObjectSourceDataSqueezer();
        squeezer.setClass(PrimaryKeySource.class);
        squeezer.setSource(source);

        assertEquals(KEY, squeezer.getPrimaryKey(m_object));
        assertNull(squeezer.getPrimaryKey(null));
        control.verify();
    }

    public void testGetValue() {
        MockControl control = MockControl.createStrictControl(DataObjectSource.class);
        control.setDefaultMatcher(MockControl.EQUALS_MATCHER);
        DataObjectSource source = (DataObjectSource) control.getMock();
        
        source.load(PrimaryKeySource.class, KEY);
        control.setReturnValue(m_object);
        control.replay();

        ObjectSourceDataSqueezer squeezer = new ObjectSourceDataSqueezer(source,
                PrimaryKeySource.class);
        
        assertSame(m_object,squeezer.getValue(KEY));
        control.verify();
    }
    
    public void testSetClass() {
        try {
            ObjectSourceDataSqueezer squeezer = new ObjectSourceDataSqueezer();
            squeezer.setClass(Object.class);
            fail("setClass should only support class implementing PrimaryKeySource");            
        }
        catch(IllegalArgumentException e) {
            // ok
        }
    }

}
