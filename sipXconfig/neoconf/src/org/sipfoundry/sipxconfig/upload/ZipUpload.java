/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.upload;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.util.Enumeration;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.FileSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;

public class ZipUpload extends Upload {

    @Override
    public void deploy() {
        getSettings().acceptVisitor(new ZipFileDeployer());        
        setDeployed(true);
    }
    
    @Override
    public void undeploy() {
        getSettings().acceptVisitor(new ZipFileUndeployer());        
        setDeployed(false);
    }

    private class ZipFileDeployer extends AbstractSettingVisitor {
        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (type instanceof FileSetting) {
                String filename = setting.getValue();
                if (filename != null) {
                    deployZipFile(new File(getDestinationDirectory()), new File(getUploadDirectory(), filename));
                }
            }
        }
    }

    private class ZipFileUndeployer extends AbstractSettingVisitor {
        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (type instanceof FileSetting) {
                String filename = setting.getValue();
                if (filename != null) {
                    undeployZipFile(new File(getDestinationDirectory()), new File(getUploadDirectory(), filename));
                }
            }
        }
    }
    
    
    /**
     * Uses zip file list and list of files to be deleted
     */
    static void undeployZipFile(File expandedDirectory, File zipFile) {
        try {
            ZipFile zip = new ZipFile(zipFile);
            Enumeration entries = zip.entries();
            while (entries.hasMoreElements()) {
                ZipEntry entry = (ZipEntry) entries.nextElement();
                if (entry.isDirectory()) {
                    // do not clean up directory, no guarontee we created them
                    continue;
                }
                File victum = new File(expandedDirectory, entry.getName());
                victum.delete();
            }
        } catch (ZipException e) {
            throw new RuntimeException(e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }                
    }
    
    /**
     * Expand zip file into destination directory
     */
    static void deployZipFile(File expandDirectory, File zipFile) {        
        try {
            ZipFile zip = new ZipFile(zipFile);
            Enumeration entries = zip.entries();
            while (entries.hasMoreElements()) {               
                ZipEntry entry = (ZipEntry) entries.nextElement();
                File file = new File(expandDirectory, entry.getName());
                if (entry.isDirectory()) {
                    file.mkdirs();
                } else {
                    file.getParentFile().mkdirs();
                    InputStream in = zip.getInputStream(entry);
                    FileWriter out = new FileWriter(file);
                    IOUtils.copy(in, out);
                    IOUtils.closeQuietly(in);
                    IOUtils.closeQuietly(out);
                }
            }
        } catch (ZipException e) {
            throw new RuntimeException(e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }        
    }
}
