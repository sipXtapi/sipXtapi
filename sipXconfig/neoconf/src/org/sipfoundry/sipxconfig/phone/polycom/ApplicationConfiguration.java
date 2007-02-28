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
package org.sipfoundry.sipxconfig.phone.polycom;

import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import java.util.regex.Pattern;

import org.apache.commons.io.FileUtils;
import org.sipfoundry.sipxconfig.device.ProfileContext;

/**
 * Velocity model for generating [MAC ADDRESS].cfg, pointer to all other config files. See page 11
 * of Administration guide for more information
 */
public class ApplicationConfiguration extends ProfileContext {
    private List m_staleDirectories = new ArrayList();

    private String m_directory;

    private PolycomPhone m_phone;

    public ApplicationConfiguration(PolycomPhone phone) {
        super(phone);
        m_phone = phone;
    }

    public String getSipBinaryFilename() {
        return "sip.ld";
    }

    public String getAppFilename() {
        return m_phone.getSerialNumber() + ".cfg";
    }

    String getDirectory() {
        if (m_directory == null) {
            String tftpRoot = m_phone.getTftpRoot();
            String endpointDir = m_phone.getSerialNumber();
            m_staleDirectories.clear();
            m_directory = getNextDirectorySequence(tftpRoot, endpointDir, m_staleDirectories);
        }

        return m_directory;
    }

    static String getNextDirectorySequence(String root, String uniqueBase, List stale) {
        File rootDir = new File(root);
        NextDirectoryScanner nextSequence = new NextDirectoryScanner(uniqueBase);
        String[] matches = rootDir.list(nextSequence);
        if (matches != null && matches.length > 0) {
            // mark these for deletion
            stale.addAll(Arrays.asList(matches));
        }

        int seq = nextSequence.getNextSequence();

        String padding = "0000";
        String suffix = padding + Integer.toString(seq);

        // performs a natural modulo at seq = 1000
        int truncateHead = suffix.length() - padding.length();
        return uniqueBase + "." + suffix.substring(truncateHead);
    }

    static class NextDirectoryScanner implements FilenameFilter {

        private Pattern m_basePattern;

        private int m_maxSequence;

        public NextDirectoryScanner(String base) {
            m_basePattern = Pattern.compile(base + "\\.\\d*");
        }

        public boolean accept(File root_, String name) {
            boolean match = m_basePattern.matcher(name).matches();
            if (match) {
                int dot = name.lastIndexOf('.');
                String suffix = name.substring(dot + 1);
                try {
                    int sequence = Integer.parseInt(suffix);
                    m_maxSequence = Math.max(m_maxSequence, sequence);
                } catch (NumberFormatException notARecognizedSuffix) {
                    // should have been caught by regexp
                    notARecognizedSuffix.printStackTrace();
                }
            }
            return match;
        }

        int getNextSequence() {
            return m_maxSequence + 1;
        }
    }

    public String getCoreFilename() {
        return getDirectory() + "/ipmid.cfg";
    }

    public String getSipFilename() {
        return getDirectory() + "/sip.cfg";
    }

    public String getPhoneFilename() {
        return getDirectory() + "/phone.cfg";
    }

    public String getDirectoryFilename() {
        // do not put this in getDirectory() because w/FTP it's not nec. anymore.
        // getDirectory() and respective code should removed
        return m_phone.getSerialNumber() + "-directory.xml";
    }

    public void deleteStaleDirectories() {
        try {
            File tftpRoot = new File(m_phone.getTftpRoot());
            Iterator i = m_staleDirectories.iterator();
            while (i.hasNext()) {
                File stale = new File(tftpRoot, (String) i.next());
                FileUtils.deleteDirectory(stale);
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}
