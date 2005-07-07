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
package org.sipfoundry.sipxconfig.common;

import org.apache.commons.lang.enum.Enum;

/**
 * All the keys on the phone key pad
 */
public final class DialPad extends Enum {
    
    public static final DialPad NUM_0 = new DialPad("0"); 
    
    public static final DialPad NUM_1 = new DialPad("1"); 
    
    public static final DialPad NUM_2 = new DialPad("2"); 
    
    public static final DialPad NUM_3 = new DialPad("3"); 
    
    public static final DialPad NUM_4 = new DialPad("4"); 
    
    public static final DialPad NUM_5 = new DialPad("5"); 
    
    public static final DialPad NUM_6 = new DialPad("6"); 
    
    public static final DialPad NUM_7 = new DialPad("7"); 
    
    public static final DialPad NUM_8 = new DialPad("8"); 
    
    public static final DialPad NUM_9 = new DialPad("9"); 
    
    public static final DialPad STAR = new DialPad("*"); 
    
    public static final DialPad POUND = new DialPad("#");
    
    public static final DialPad[] KEYS = (DialPad[]) getEnumList(DialPad.class).toArray(new DialPad[0]);
    
    private DialPad(String name) {
        super(name);        
    }
    
    /**
     * Used for Hibernate type translation
     */
    public static class UserType extends EnumUserType {
        public UserType() {
            super(DialPad.class);
        }
    }
    
    public static final DialPad getByName(String name) {
        return (DialPad) getEnumMap(DialPad.class).get(name);
    }
}
