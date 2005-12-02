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

package org.sipfoundry.sipxphone.sys ;

import java.io.* ;

/**
 * AppListItem is a class used by the ApplicationRegistry to store a single
 * application item.
 */
class AppListItem implements Serializable
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /**
     * WARNING: Do NOT add or remove any attributes in this class.  For
     *          backwards compatibility, the serialVersionUID has been fixed
     *          and may blow up phone with old/existing AppListItems.  You
     *          can add/change methods, though.
     */
    public int    iAppType ;
    public String strClassNameOrURL ;
    private static final long serialVersionUID = 7241782694933355383L;        
    

//////////////////////////////////////////////////////////////////////////////
// Construction
////
       
    /**
     * Constructor taking the application type and class name/url
     */
    public AppListItem(int iAppType, String strClassNameOrURL)
    {
        this.iAppType = iAppType ;
        this.strClassNameOrURL = strClassNameOrURL ;                        
    }
    
    
    /** 
     * Default no-argument constructor (required for serialization)
     */
    public AppListItem()
    {
        strClassNameOrURL = null ;
        iAppType = -1 ;
    }
    
    
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Get the type of this item
     */
    public int getType()
    {
        return iAppType ;        
    }
    
    
    /**
     * Get the class name/url of this item
     */
    public String getClassNameOrURL()
    {
        return strClassNameOrURL ;   
    }
    
    
    /**
     * Overloaded equals operator that determines the specified AppItemList is
     * equal to itself.
     */
    public boolean equals(Object obj)
    {
        boolean bEquals = false ;
        
        if ((obj != null) && (obj instanceof AppListItem)) {
            AppListItem item = (AppListItem) obj ;            
            if ((strClassNameOrURL != null) && (item.strClassNameOrURL != null)) {
                bEquals = ((iAppType == item.iAppType) && 
                        strClassNameOrURL.equals(item.strClassNameOrURL)) ;                
            }            
        }        
        return bEquals ;        
    }
 
 
    /**
     * Customized toString representation for debugging purpose
     */
    public String toString()
    {
        return Integer.toString(iAppType) + "/" + strClassNameOrURL ;
    }                
}
