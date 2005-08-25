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
package org.sipfoundry.sipxconfig.common;

import java.io.StringReader;

import junit.framework.TestCase;

import org.dom4j.Document;
import org.dom4j.io.SAXReader;
import org.easymock.MockControl;
import org.hibernate.cfg.Configuration;
import org.hibernate.util.DTDEntityResolver;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.audiocodes.MediantGateway;
import org.springframework.beans.factory.ListableBeanFactory;

public class DynamicSessionFactoryBeanTest extends TestCase {

    public void testXmlMapping() throws Exception {
        DynamicSessionFactoryBean factory = new DynamicSessionFactoryBean();
        String mapping = factory.xmlMapping(Gateway.class, MediantGateway.class, "gwMediant1000");
        validateXml(mapping);
    }
    
    public void testBindSubclasses() throws Exception {
        MockControl controlFactory = MockControl.createControl(ListableBeanFactory.class);
        ListableBeanFactory factory = (ListableBeanFactory) controlFactory.getMock();
        factory.getBeanNamesForType(Gateway.class);
        controlFactory.setReturnValue(new String[] {"gwGeneric", "gwMediant1000"});
        factory.getType("gwGeneric");
        controlFactory.setReturnValue(Gateway.class);
        factory.getType("gwMediant1000");
        controlFactory.setReturnValue(MediantGateway.class);
        controlFactory.replay();
        
        ConfigurationMock config = new ConfigurationMock();
                
        DynamicSessionFactoryBean bean = new DynamicSessionFactoryBean();
        bean.setBeanFactory(factory);
        bean.bindSubclasses(config, Gateway.class);
        
        controlFactory.verify();
        config.verify();
    }
    

    public void testBindSubclassesOfBean() throws Exception {
        MockControl controlFactory = MockControl.createControl(ListableBeanFactory.class);
        ListableBeanFactory factory = (ListableBeanFactory) controlFactory.getMock();
        factory.getBeanNamesForType(Gateway.class);
        controlFactory.setReturnValue(new String[] {"gwGeneric", "gwMediant1000"});
        factory.getType("gwGeneric");
        controlFactory.setReturnValue(Gateway.class);
        factory.getType("gwMediant1000");
        controlFactory.setReturnValue(MediantGateway.class);
        controlFactory.replay();
        
        ConfigurationMock config = new ConfigurationMock();
                
        DynamicSessionFactoryBean bean = new DynamicSessionFactoryBean();
        bean.setBeanFactory(factory);
        bean.bindSubclasses(config, "gwGeneric");
        
        controlFactory.verify();
        config.verify();
    }
    
    
    private static void validateXml(String xml) throws Exception {
        SAXReader xmlReader = new SAXReader();
        xmlReader.setEntityResolver(new DTDEntityResolver());
        xmlReader.setValidation(true);
        
        Document document = xmlReader.read(new StringReader(xml));
        
        assertEquals(Gateway.class.getName(), document.valueOf("/hibernate-mapping/subclass/@extends"));
        assertEquals(MediantGateway.class.getName(),
                document.valueOf("/hibernate-mapping/subclass/@name"));
        assertEquals("gwMediant1000",
                document.valueOf("/hibernate-mapping/subclass/@discriminator-value"));        
    }
    
    /**
     * Unfortunately class easy mock does not work for configuration 
     */
    private static class ConfigurationMock extends Configuration {
        boolean m_valid;
        
        public void verify() throws Exception {
            assertTrue(m_valid);
        }
        
        public Configuration addXML(String xml) {
            try {
                validateXml(xml);
            } catch (RuntimeException e) {
                throw e;
            }
            catch (Exception e) {
                throw new RuntimeException(e); 
            }
            m_valid = true;
            return this;
        }
    }
}
