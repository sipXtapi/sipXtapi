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
package org.sipfoundry.sipxconfig.site.vm;

import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;

public class MoveVoicemailAction implements OptionAdapter, IActionListener {
    private String m_folderId;
    private String m_folderLabel;
    
    public MoveVoicemailAction(String folderLabel, String folderId) {
        m_folderLabel = folderLabel;
        m_folderId = folderId;
    }
    
    public void actionTriggered(IComponent arg0, IRequestCycle arg1) {
    }

    public String getLabel(Object option, int index) {
        return m_folderLabel;
    }

    public Object getValue(Object option, int index) {
        return m_folderId;
    }

    public String squeezeOption(Object option, int index) {
        return m_folderId;
    }

}
