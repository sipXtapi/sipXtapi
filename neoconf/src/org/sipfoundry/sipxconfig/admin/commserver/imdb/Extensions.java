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
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.dom4j.Element;
import org.sipfoundry.sipxconfig.common.User;

public class Extensions extends DataSetGenerator {
    private static final Pattern PATTERN_NUMERIC = Pattern.compile("\\d+");
    
    /** Add all numeric aliases as extension elements in an XML document */
    protected void addItems(Element items) {
        String domainName = getCoreContext().getDomainName();
        List users = getCoreContext().loadUsers();
        for (Iterator i = users.iterator(); i.hasNext();) {
            User user = (User) i.next();
            String[] aliases = user.getAliases();
            boolean foundNumericAlias = false;
            Element item = null;
            for (int j = 0; j < aliases.length; j++) {
                String alias = aliases[j];
                Matcher m = PATTERN_NUMERIC.matcher(alias);
                if (m.matches()) {
                    if (!foundNumericAlias) {
                        // There is at least one numeric alias, so add an entry for this user
                        item = items.addElement("item");
                        item.addElement("uri").setText(user.getUri(domainName));
                        foundNumericAlias = true;
                    }                    
                    item.addElement("extension").setText(alias);
                }
            }
        }
    }
    
    protected DataSet getType() {
        return DataSet.EXTENSION;
    }
}
