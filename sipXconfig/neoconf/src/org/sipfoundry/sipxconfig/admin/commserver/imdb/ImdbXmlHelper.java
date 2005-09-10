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
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.ArrayList;

import org.apache.commons.digester.Digester;
import org.apache.commons.digester.SetNestedPropertiesRule;

/**
 * Class for parsing IMDB xml
 */
public final class ImdbXmlHelper {
    public static final String PATTERN = "items/item";
    
    private ImdbXmlHelper() {
        // utility class - no instantiation
    }

    public static Digester configureDigester(Class itemClass) {
        Digester digester = new Digester();
        digester.setValidating(false);
        digester.setNamespaceAware(false);

        digester.push(new ArrayList());
        digester.addObjectCreate(ImdbXmlHelper.PATTERN, itemClass);
        SetNestedPropertiesRule rule = new SetNestedPropertiesRule();
        // ignore all properties that we are not interested in
        rule.setAllowUnknownChildElements(true);
        digester.addRule(ImdbXmlHelper.PATTERN, rule);
        digester.addSetNext(ImdbXmlHelper.PATTERN, "add");

        return digester;
    }
}
