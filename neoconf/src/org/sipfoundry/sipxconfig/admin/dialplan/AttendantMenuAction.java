/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.apache.commons.lang.enum.Enum;
import org.sipfoundry.sipxconfig.common.EnumUserType;

/**
 * All possible, supported actions for creation auto attendants. 
 */
public final class AttendantMenuAction extends Enum {
    
    public static final AttendantMenuAction OPERATOR = new AttendantMenuAction("operator");
    
    public static final AttendantMenuAction CANCEL = new AttendantMenuAction("cancel");

    public static final AttendantMenuAction DISCONNECT = new AttendantMenuAction("disconnect");

    public static final AttendantMenuAction AUTO_ATTENDANT = new AttendantMenuAction("autoAttendant");
    
    public static final AttendantMenuAction GOTO_EXTENSION  = new AttendantMenuAction("gotoExtension");
    
    public static final AttendantMenuAction[] ACTIONS = 
            (AttendantMenuAction[]) getEnumList(AttendantMenuAction.class).toArray(new AttendantMenuAction[0]);

    private AttendantMenuAction(String actionId) {
        super(actionId);
    }
    
    /**
     * Used for Hibernate type translation
     */
    public static class UserType extends EnumUserType {
        public UserType() {
            super(AttendantMenuAction.class);
        }
    }
}
