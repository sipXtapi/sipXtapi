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
package org.sipfoundry.sipxconfig.site.user_portal;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.user.EditPinComponent;
import org.sipfoundry.sipxconfig.site.user.UserForm;
import org.sipfoundry.sipxconfig.vm.VoicemailPreferences;


public abstract class EditMyInformation extends UserBasePage implements EditPinComponent {
    
    public abstract String getPin();
    
    @Bean
    public abstract VoicemailPreferences getVoicemailPreferences();
    
    public void save() {
        if (!getValidator().getHasErrors()) {
            getCoreContext().saveUser(getUser());
        }
    }
    
    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);
        
        if (getPin() == null) {
            UserForm.initializePin(getComponent("pin"), this, getUser());
        } else if (TapestryUtils.isValid(this)) {
            UserForm.updatePin(this, getUser(), getCoreContext().getAuthorizationRealm());
        }
    }
}
