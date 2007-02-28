/*
 * 
 * 
 * Copyright (C) 2007 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2007 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.Writer;

import org.apache.commons.io.IOUtils;

public abstract class AbstractProfileGenerator implements ProfileGenerator {

    public void generate(ProfileContext context, String templateFileName, Writer out) {
        if (templateFileName == null) {
            return;
        }
        try {
            generateProfile(context, templateFileName, out);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void generate(ProfileContext context, String templateFileName, String outputFileName) {
        generate(context, templateFileName, null, outputFileName);
    }

    public void generate(ProfileContext context, String templateFileName, ProfileFilter filter,
            String outputFileName) {
        if (outputFileName == null) {
            return;
        }

        Writer wtr = null;
        try {
            File file = new File(outputFileName);
            ProfileUtils.makeParentDirectory(file);
            wtr = new FileWriter(file);
            if (filter == null) {
                generateProfile(context, templateFileName, wtr);
            } else {
                Writer unformatted = new StringWriter();
                generateProfile(context, templateFileName, unformatted);
                filter.copy(new StringReader(unformatted.toString()), wtr);
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(wtr);
        }
    }

    protected abstract void generateProfile(ProfileContext context, String templateFileName,
            Writer out) throws IOException;
}
