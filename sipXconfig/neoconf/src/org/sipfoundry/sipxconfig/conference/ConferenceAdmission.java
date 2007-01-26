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
package org.sipfoundry.sipxconfig.conference;

import java.io.File;
import java.io.IOException;
import java.util.List;

import org.apache.commons.collections.Closure;
import org.apache.commons.collections.CollectionUtils;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.io.OutputFormat;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigFileType;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;

public class ConferenceAdmission extends XmlFile {

    private String m_configDirectory;

    private Document m_document;

    public Document getDocument() {
        return m_document;
    }
    
    public void generate(List conferences) {
        m_document = FACTORY.createDocument();
        Element context = m_document.addElement("context");
        context.addAttribute("name","default");
        CollectionUtils.forAllDo(conferences, new BuildConferenceElem(context));
    }

    private static class BuildConferenceElem implements Closure {
        private final Element m_parent;

        public BuildConferenceElem(Element parent) {
            m_parent = parent;
        }

        public void execute(Object input) {
            Conference conference = (Conference) input;
            if (!conference.isEnabled()) {
                return;
            }
            m_parent.addElement("extension")
              .addAttribute("name",conference.getExtension())
              .addElement("condition")
                .addAttribute("field","destination_number")
                .addAttribute("expression","^"+conference.getName()+"$")
                .addElement("action")
                   .addAttribute("application","conference")
                   .addAttribute("data",conference.getName()+"+["+
            		  conference.getParticipantAccessCode()+"]");
        }
    }

    /**
     * Writes to file in a specified directory
     * 
     * @throws IOException
     */
    public void writeToFile() throws IOException {
        File parent = new File(m_configDirectory);
        writeToFile(parent, getType().getName());
    }
	
    @Override
    public OutputFormat createFormat() {
        OutputFormat format = super.createFormat();
        format.setSuppressDeclaration(true);
        return format;
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    public ConfigFileType getType() {
        return ConfigFileType.CONFERENCES;
    }
}
