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
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringWriter;
import java.io.Writer;

import org.apache.commons.lang.StringUtils;
import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.dom4j.Element;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.XMLWriter;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

/**
 * ConfigFile
 */
public abstract class XmlFile {
    protected static final DocumentFactory FACTORY = DocumentFactory.getInstance();

    public abstract Document getDocument();

    /**
     * Writes document to specified writer
     * 
     * @param writer
     * @throws IOException
     */
    public void write(Writer writer) throws IOException {
        Document document = getDocument();
        OutputFormat format = new OutputFormat();
        format.setNewlines(true);
        format.setIndent(true);
        XMLWriter xmlWriter = new XMLWriter(writer, format);
        xmlWriter.write(document);
    }

    /**
     * Retrieves configuration file content as string
     * 
     * Use only for preview, use write function to dump it to the file.
     * 
     */
    public String getFileContent() {
        try {
            StringWriter writer = new StringWriter();
            write(writer);
            writer.close();
            return writer.toString();
        } catch (IOException e) {
            // ignore when writing to string
            // TODO: log
            return "";
        }
    }

    /**
     * Creates a bakup copy of a generated file, and writes a new file
     * 
     * @param configDir File object representing a directory in which files are created
     * @param filename xml file name
     * @throws IOException
     */
    public void writeToFile(File configDir, String filename) throws IOException {
        File configFile = new File(configDir, filename);
        // make a backup copy of the file if it exist
        boolean created = configFile.createNewFile();
        if (!created) {
            // FIXME: this is a naive generation of backup files - we should not have more than n
            // backups
            File backup = new File(configDir, filename + ".~");
            backup.delete();
            configFile.renameTo(backup);
            configFile = new File(configDir, filename);
            configFile.createNewFile();
        }
        FileWriter writer = new FileWriter(configFile);
        write(writer);
        writer.close();
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
