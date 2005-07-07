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

import junit.framework.TestCase;

import org.apache.tapestry.IMessages;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.easymock.MockControl;

public class LocalizedOptionModelDecoratorTest extends TestCase {
    
    private LocalizedOptionModelDecorator m_localized;
    
    protected void setUp() {
        String[] options = { "a", "b", "c" };
        IPropertySelectionModel model = new StringPropertySelectionModel(options);
        m_localized = new LocalizedOptionModelDecorator();        
        m_localized.setModel(model);
    }
    
    public void testReturnKeyOnNullMessagesObject() {
        assertEquals("a", m_localized.getLabel(0));
    }
    
    public void testLocalization() {
        MockControl messagesControl = MockControl.createStrictControl(IMessages.class);
        IMessages messages = (IMessages) messagesControl.getMock();
        messagesControl.expectAndReturn(messages.getMessage("xyz.a", "a"), "localized a");
        messagesControl.expectAndReturn(messages.getMessage("xyz.b", "b"), "b");
        messagesControl.expectAndReturn(messages.getMessage("c", "c"), "localized c");
        messagesControl.replay();
        
        m_localized.setMessages(messages);
        m_localized.setResourcePrefix("xyz.");
        
        assertEquals("localized a", m_localized.getLabel(0));
        assertEquals("b", m_localized.getLabel(1));
        
        m_localized.setResourcePrefix(null);
        assertEquals("localized c", m_localized.getLabel(2));        

        messagesControl.verify();
    }
}
