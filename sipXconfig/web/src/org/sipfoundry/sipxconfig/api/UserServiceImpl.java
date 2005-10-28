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
package org.sipfoundry.sipxconfig.api;


public class UserServiceImpl implements UserService {
    
    public void hello(String message, String message2) {
        System.out.print("hello message " + message);
        System.out.print("hello message2 " + message2);
    }

    public void goodbye(String message) {
        System.out.print("goodbye message " + message);
    }
}
