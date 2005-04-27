/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

public class ObjectSelectionModelTest extends TestCase {
    
    List m_objects;
    
    protected void setUp() {        
        m_objects = new ArrayList();
        m_objects.add(new ExampleBusinessObject("voltar", new Integer(100)));
        m_objects.add(new ExampleBusinessObject("kuku", new Integer(101)));
        m_objects.add(new ExampleBusinessObject("bongo", new Integer(102)));
    }
    
    public void testGetLabel() {
        ObjectSelectionModel model = new ObjectSelectionModel();
        model.setCollection(m_objects);
        model.setLabelExpression("something");
        
        assertEquals("voltar", model.getLabel(0));
        assertEquals("kuku", model.getLabel(1));
        assertEquals("bongo", model.getLabel(2));
    }
    
    public void testGetOption() {
        ObjectSelectionModel model = new ObjectSelectionModel();
        model.setCollection(m_objects);
        model.setValueExpression("somethingElse");
        
        assertEquals(new Integer(100), model.getOption(0));
        assertEquals(new Integer(101), model.getOption(1));
        assertEquals(new Integer(102), model.getOption(2));
    }

    public void testNullCollection() {
        ObjectSelectionModel model = new ObjectSelectionModel();
        assertEquals("", model.getLabel(0));
    }
        
    public void testNullLabel() {
        ObjectSelectionModel model = new ObjectSelectionModel();
        model.setCollection(m_objects);
        assertEquals("", model.getLabel(0));
    }
        
    public void testInvalidLabel() {
        ObjectSelectionModel model = new ObjectSelectionModel();
        model.setCollection(m_objects);
        model.setLabelExpression("bogus");
        try {
            model.getLabel(0);
            fail();
        } catch (RuntimeException e) {
            assertTrue(true);            
        }
    }
    
    static class ExampleBusinessObject {
        
        String m_something;
        
        Integer m_somethingElse;
        
        ExampleBusinessObject(String something, Integer somethingElse) {
            m_something = something;
            m_somethingElse = somethingElse;
        }

        public String getSomething() {
            return m_something;
        }
        
        public Integer getSomethingElse() {
            return m_somethingElse;
        }
    }
}
