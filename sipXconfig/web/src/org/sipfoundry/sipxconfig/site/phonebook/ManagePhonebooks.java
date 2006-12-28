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

import java.util.Collection;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;

public abstract class ManagePhonebooks extends BasePage {
    
    public abstract SelectMap getSelections();
    
    public abstract PhonebookManager getPhonebookManager();

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
    
    public void deletePhonebooks() {
        SelectMap selections = getSelections();
        Collection selected = selections.getAllSelected();
        getPhonebookManager().deletePhonebooks(selected);        
    }
}
