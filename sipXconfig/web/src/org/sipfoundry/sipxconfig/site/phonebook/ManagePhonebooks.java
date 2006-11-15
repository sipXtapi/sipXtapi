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
package org.sipfoundry.sipxconfig.site.phonebook;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;

public class ManagePhonebooks extends BasePage {
    
    public IPage edit(IRequestCycle cycle, Integer phonebookId) {
        EditPhonebook page = (EditPhonebook) cycle.getPage(EditPhonebook.PAGE);
        page.setPhonebookId(phonebookId);
        page.setReturnPage(this);
        return page;        
    }
    
    public IPage addPhonebook(IRequestCycle cycle) {
        EditPhonebook page = (EditPhonebook) cycle.getPage(EditPhonebook.PAGE);
        page.setReturnPage(this);
        return page;        
    }
}
