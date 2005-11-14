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
package org.sipfoundry.sipxconfig.site.search;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;

public abstract class SearchPanel extends BaseComponent {

    public abstract String getQuery();

    public void search(IRequestCycle cycle) {
        String query = getQuery();
        if (StringUtils.isEmpty(query)) {
            return;
        }
        SearchPage page = (SearchPage) cycle.getPage(SearchPage.PAGE);
        page.setQuery(query);
        cycle.activate(page);
    }

}
