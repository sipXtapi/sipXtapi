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


package org.sipfoundry.sipxphone.sys.user ;


/**
 * Pingtel user management class. This class allows users to be added and deleted. This class
 * should be enhanced to provide mandatory methods and implementations for dealing with
 * event notifications should the user state change.
 *
 */
public class PUserManager
{      
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    
    
/////////////////////////////////////////////////////////////////////////////
// Implementation
////                
    /**
     * Add a user to the user database
     *
     * @strUser the ID of the user
     * @strPassword the user's password (can be null)
     * @exception IllegalStateException if user already exists
     * @exception IllegalArgumentException if method is called with invalid arguments
     */   
    public void addUser(String strUser, String strPassword) 
            throws IllegalStateException, IllegalArgumentException
    {
        int i = JNI_addUser(strUser, strPassword) ;
        
        if (i == 1) 
            throw new IllegalStateException("User already exists") ;
        else if (i ==2)
            throw new IllegalArgumentException("Incorrect arguments") ;
    }
    
    
     /**
     * Delete a user from the user database
     *
     * @strUser the ID of the user
     * @exception IllegalStateException if user doesn't exist
     * @exception IllegalArgumentException if method is called with invalid arguments     
     */   
    public void deleteUser(String strUser)
            throws IllegalStateException, IllegalArgumentException
    
    {
        int i = JNI_deleteUser(strUser) ;
        
        if (i == 1) 
            throw new IllegalStateException("User doesn't exist") ;
        else if (i ==2)
            throw new IllegalArgumentException("Incorrect arguments") ;
    }   
    
    /**
     * Change the password of a user
     *
     * @strUser the ID of the user
     * @strPassword the user's new password (can be null)
     * @exception IllegalStateException if user doesn't exist
     * @exception IllegalArgumentException if method is called with invalid arguments     
     */   
    public void changeUserPassword(String strUser, String strPassword)
            throws IllegalStateException, IllegalArgumentException

    {
        int i = JNI_changeUserPassword(strUser, strPassword) ;
        
        if (i == 1) 
            throw new IllegalStateException("User doesn't exist") ;
        else if (i ==2)
            throw new IllegalArgumentException("Incorrect arguments") ;
        
    }
    
    /**
     * Ensure the user's name and password match
     *
     * @strUser the ID of the user
     * @strPassword the user's new password (can be null)
     * @exception IllegalStateException if user doesn't exist
     * @exception IllegalArgumentException if method is called with invalid arguments
     */   
    public boolean authenticateUser(String strUser, String strPassword)
            throws IllegalStateException, IllegalArgumentException

    {
        boolean bReturn = false ;
        
        switch (JNI_authenticateUser(strUser, strPassword)) {
            case (0) : bReturn = true ; break ; 
            case (1) : bReturn = false ; break ; 
            case (2) : throw new IllegalStateException("User Not Found") ;
            case (3) : throw new IllegalArgumentException("Incorrect arguments") ;
        }
        
        return bReturn ;
    }    

/////////////////////////////////////////////////////////////////////////////
// Native Methods
////        

    // User Database methods
    public native int       JNI_addUser(String strUser, String strPassword) ;
    public native int       JNI_deleteUser(String strUser) ;
    public native int       JNI_changeUserPassword(String strUser, String strPassword) ;
    public native int       JNI_authenticateUser(String strUser, String strPassworD) ;


}