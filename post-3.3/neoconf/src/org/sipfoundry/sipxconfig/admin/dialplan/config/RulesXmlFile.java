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
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.io.File;
import java.util.Iterator;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

public abstract class RulesXmlFile extends XmlFile {
    public static final Log LOG = LogFactory.getLog(RulesXmlFile.class);

    private String m_externalRulesFileName;

    public void setExternalRulesFileName(String externalRulesFileName) {
        m_externalRulesFileName = externalRulesFileName;
    }

    /** called before generating rules */
    public abstract void begin();

    public abstract void generate(IDialingRule rule);

    /** called after last rule is generated */
    public abstract void end();

    /**
     * Insert mapping rules from external mapping rules file
     * 
     * @param mappings - root element of the document
     */
    protected void addExternalRules(Element mappings) {
        if (m_externalRulesFileName == null) {
            return;
        }
        File externalRulesFile = new File(m_externalRulesFileName);
        if (!externalRulesFile.canRead()) {
            LOG.warn("Cannot read from external mapping rules file: " + m_externalRulesFileName);
            return;
        }
        SAXReader reader = new SAXReader();
        try {
            Document externalRules = reader.read(externalRulesFile);
            Element rootElement = externalRules.getRootElement();
            for (Iterator i = rootElement.elementIterator(); i.hasNext();) {
                Element hostMatch = (Element) i.next();
                mappings.add(hostMatch.detach());
            }
        } catch (DocumentException e) {
            LOG.error("Cannot parse external rules file", e);
        }
    }

    protected void addRuleDescription(Element userMatch, IDialingRule rule) {
        String descriptionText = rule.getDescription();
        if (!StringUtils.isBlank(descriptionText)) {
            Element description = userMatch.addElement("description");
            description.setText(descriptionText);
        }
    }

    protected void addRuleNameComment(Element hostMatch, IDialingRule rule) {
        String nameText = rule.getName();
        if (!StringUtils.isBlank(nameText)) {
            hostMatch.addComment(nameText);
        }
    }
}
