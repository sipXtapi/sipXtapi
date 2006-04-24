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

import java.text.MessageFormat;

import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.StringUtils;

/**
 * Version information is kept in jar manifest file which is created during build process. See
 * manifest task in neoconf/build.xml for more details.
 */
public class VersionInfo {
    /**
     * @return Major and minor version number
     */
    public String getVersion() {
        return getClass().getPackage().getSpecificationVersion();
    }
    
    /**
     * Split version into convienient ids for comparison 
     * @return
     */
    public Integer[] getVersionIds() {
        return versionStringToVersionIds(getVersion());
    }
    
    static Integer[] versionStringToVersionIds(String version) {
        String[] sids = version.split("\\.");
        Integer[] ids = new Integer[sids.length];
        for (int i = 0; i < ids.length; i++) {
            ids[i] = new Integer(sids[i]);
        }
        return ids;
    }
    
    /**
     * @return more specific build information
     */
    public String getBuild() {
        return getClass().getPackage().getImplementationVersion();
    }

    public String getTitle() {
        return getClass().getPackage().getSpecificationTitle();
    }
    
    public String getLongVersionString() {
        Object[] params = {
            getTitle(), getVersion(), getBuild()
        };
        // remove nulls
        for (int i = 0; i < params.length; i++) {
            params[i] = ObjectUtils.defaultIfNull(params[i], StringUtils.EMPTY);
        }
        return MessageFormat.format("{0} {1} ({2})", params);
    }
}
