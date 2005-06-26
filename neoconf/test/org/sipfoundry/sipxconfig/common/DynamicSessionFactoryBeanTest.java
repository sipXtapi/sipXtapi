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

import org.custommonkey.xmlunit.XMLTestCase;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.audiocodes.MediantGateway;

public class DynamicSessionFactoryBeanTest extends XMLTestCase {

    public void testXmlMapping() throws Exception {
        DynamicSessionFactoryBean factory = new DynamicSessionFactoryBean();
        String mapping = factory.xmlMapping(Gateway.class, MediantGateway.class, "gwMediant1000");
        assertXpathEvaluatesTo(Gateway.class.getName(), "/hibernate-mapping/subclass/@extends",
                mapping);
        assertXpathEvaluatesTo(MediantGateway.class.getName(),
                "/hibernate-mapping/subclass/@name", mapping);
        assertXpathEvaluatesTo("gwMediant1000",
                "/hibernate-mapping/subclass/@discriminator-value", mapping);
    }

}
