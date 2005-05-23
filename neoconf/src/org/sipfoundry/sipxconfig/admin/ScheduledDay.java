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
package org.sipfoundry.sipxconfig.admin;

import org.apache.commons.lang.enum.Enum;
import org.sipfoundry.sipxconfig.common.EnumUserType;

public final class ScheduledDay extends Enum {
    
    public static final ScheduledDay EVERYDAY = new ScheduledDay("Everyday");    
    public static final ScheduledDay SUNDAY = new ScheduledDay("Sunday");
    public static final ScheduledDay MONDAY = new ScheduledDay("Monday");
    public static final ScheduledDay TUESDAY = new ScheduledDay("Tuesday");
    public static final ScheduledDay WEDNESDAY = new ScheduledDay("Wednesday");
    public static final ScheduledDay THURSDAY = new ScheduledDay("Thursday");
    public static final ScheduledDay FRIDAY = new ScheduledDay("Friday");
    public static final ScheduledDay SATURDAY = new ScheduledDay("Saturday");

    private ScheduledDay(String id) {
        super(id);
    }

    /**
     * Used for Hibernate type translation
     */
    public static class UserType extends EnumUserType {
        public UserType() {
            super(ScheduledDay.class);
        }
    }    
}
