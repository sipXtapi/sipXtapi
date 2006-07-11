/*
 * $Id$
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

 
package org.sipfoundry.sipxphone.awt ;

/** 
 ** WARNING: Values in org.sipfoundry.app.event.KeyCode must match 
 **          org.sipfoundry.JNIProxyEvent
 **/

public interface KeyCode {
    
    /*
     * Dial Pad Keys
     */
     
    public static final int DIALPAD_0       = 48 ;
    public static final int DIALPAD_1       = 49 ;
    public static final int DIALPAD_2       = 50 ;
    public static final int DIALPAD_3       = 51 ;
    public static final int DIALPAD_4       = 52 ;
    public static final int DIALPAD_5       = 53 ;
    public static final int DIALPAD_6       = 54 ;
    public static final int DIALPAD_7       = 55 ;
    public static final int DIALPAD_8       = 56 ;
    public static final int DIALPAD_9       = 57 ;
    public static final int DIALPAD_STAR    = 42 ;
    public static final int DIALPAD_POUND   = 35 ;             
    
    /* 
     * Soft keys surrounding the display panel
     */    
     
    public static final int SOFT_WEST_1     = 329 ;
    public static final int SOFT_WEST_2     = 323 ;
    public static final int SOFT_WEST_3     = 334 ;
    public static final int SOFT_WEST_4     = 328 ;
    
    public static final int SOFT_EAST_1     = 320 ;
    public static final int SOFT_EAST_2     = 326 ;
    public static final int SOFT_EAST_3     = 332 ;
    public static final int SOFT_EAST_4     = 321 ;
    
    public static final int SOFT_SOUTH_1    = 322 ;
    public static final int SOFT_SOUTH_2    = 333 ;
    public static final int SOFT_SOUTH_3    = 327 ;
                
    /*
     * Core System Keys
     */     
     
    public static final int SYS_PINGTEL     = 330 ;
    public static final int SYS_HOLD        = 262 ;
    public static final int SYS_TRANSFER    = 273 ;
    public static final int SYS_CONFERENCE  = 258 ;
    public static final int SYS_REDIAL      = 268 ;


    /*
     * Temp / Hack Keys
     */         
    public static final int TMP_SCROLL_UP   = 335 ;
    public static final int TMP_SCROLL_DOWN = 324 ;
}
