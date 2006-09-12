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

import java.util.List;

import org.dom4j.Element;
import org.sipfoundry.sipxconfig.common.User;

public class Extensions extends DataSetGenerator {

    /** Add all numeric aliases as extension elements in an XML document */
    protected void addItems(Element items) {
        String domainName = getSipDomain();
        List<User> users = getCoreContext().loadUsers();
        for (User user : users) {
            List<String> numericAliases = user.getNumericAliases();
            if (numericAliases.size() > 0) {
                Element item = items.addElement("item");
                item.addElement("uri").setText(user.getUri(domainName));
                // add first found numeric alias
                item.addElement("extension").setText(numericAliases.get(0));
            }
        }
    }

    protected DataSet getType() {
        return DataSet.EXTENSION;
    }
}
