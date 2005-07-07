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
import java.io.IOException;
import java.util.Collection;
import java.util.Iterator;

import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.callgroup.ParkOrbit;

public class Orbits extends XmlFile {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/orbits-00-00";
    private static final String FILENAME = "orbits.xml";

    private String m_configDirectory;

    private String m_audioDirectory;

    private Document m_document;

    public Document getDocument() {
        return m_document;
    }

    public void generate(Collection parkOrbits) {
        m_document = FACTORY.createDocument();
        Element orbits = m_document.addElement("orbits", NAMESPACE);
        File dir = new File(m_audioDirectory);
        for (Iterator i = parkOrbits.iterator(); i.hasNext();) {
            ParkOrbit parkOrbit = (ParkOrbit) i.next();
            // ignore disabled orbits
            if (!parkOrbit.isEnabled()) {
                continue;
            }
            Element orbit = orbits.addElement("orbit");
            orbit.addElement("name").setText(parkOrbit.getName());
            orbit.addElement("extension").setText(parkOrbit.getExtension());
            File audioFile = new File(dir, parkOrbit.getMusicOnHold());
            orbit.addElement("background-audio").setText(audioFile.getAbsolutePath());
            orbit.addElement("description").setText(parkOrbit.getDescription());
        }
    }

    /**
     * Writes to file in a specified directory
     * 
     * @throws IOException
     */
    public void writeToFile() throws IOException {
        File parent = new File(m_configDirectory);
        writeToFile(parent, FILENAME);
    }

    public String getAudioDirectory() {
        return m_audioDirectory;
    }

    public void setAudioDirectory(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }
}
