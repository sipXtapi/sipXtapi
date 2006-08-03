/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Scott Zuk
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.components.PageWithCallback;

public abstract class ReplicationData extends PageWithCallback {

    public static final String PAGE = "ReplicationData";

    public abstract SipxReplicationContext getSipxReplicationContext();

    public abstract String getDataSetName();

    public abstract void setDataSetName(String name);

    public String getXml() {
        return getSipxReplicationContext().getXml(DataSet.getEnum(getDataSetName()));
    }

    @SuppressWarnings("unused")
    public void setXml(String xml_) {
        // ignore xml - read only field
    }
}
