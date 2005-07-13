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

import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.common.User;

public class Extensions extends DataSetGenerator {
    protected void addItems(Element items) {
        String domainName = getCoreContext().getDomainName();
        List users = getCoreContext().loadUsers();
        for (Iterator i = users.iterator(); i.hasNext();) {
            User user = (User) i.next();
            String extension = user.getExtension();
            if (StringUtils.isBlank(extension)) {
                continue;
            }
            Element item = items.addElement("item");
            item.addElement("uri").setText(user.getUri(domainName));
            item.addElement("extension").setText(extension);
        }
    }
    
    protected DataSet getType() {
        return DataSet.EXTENSION;
    }
}
