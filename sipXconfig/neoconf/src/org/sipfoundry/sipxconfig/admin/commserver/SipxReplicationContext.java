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
package org.sipfoundry.sipxconfig.admin.commserver;

import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;
import org.springframework.context.ApplicationEvent;

public interface SipxReplicationContext {
    void generate(DataSet dataSet);

    void generateAll();

    void replicate(XmlFile xmlFile);

    String getXml(DataSet dataSet);

    /**
     * This function will publish application event - in case the application is done lazily it
     * will publish the even only after everything has been replicated
     * 
     * @param event event to be published
     */
    void publishEvent(ApplicationEvent event);
}
