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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.dom4j.Element;

public class Permissions extends DataSetGenerator {
    private static final Log LOG = LogFactory.getLog(Permissions.class);

    /**
     * Adds  
     * <item>
     *     <identity>user_uri</identity>
     *     <permission>permission_name</permission>
     * </item>
     * 
     * to the list of items.
     */
    protected void addItems(Element items_) {
        LOG.warn("Permissions replication not implemented.");
    }

    protected DataSet getType() {
        return DataSet.PERMISSION;
    }
}
