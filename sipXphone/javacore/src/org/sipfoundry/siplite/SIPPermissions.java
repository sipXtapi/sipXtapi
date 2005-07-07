/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/siplite/SIPPermissions.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */
package org.sipfoundry.siplite;

import javax.siplite.Constants;
/**
 * Permissions.java
 *
 *
 * Created: Mon Apr 15 12:15:31 2002
 *
 * This class controls the methods and headers that can be
 * added using SIPLITE. The values of permitted header and methods
 * can be added/removed as desired by changing this file.
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */

public class SIPPermissions {

    /**
     * restricted methods.
     */
   private String[] m_strArrayRestrictedMethods = { Constants.ACK };

    /**
     * restricted headers.
     */
   private String[] m_strArrayRestrictedHeaders = {};

   /**
    *Singleton instance of SIPPermissions.
    */
   private static SIPPermissions s_permissions = new SIPPermissions();

   /**
    * private instance of SIPPermissions so that it can't be instantiated
    * outside this class using the default constructor.
    */
   private SIPPermissions(){
   }

   /**
    * method to get the singleton instance of SIPPermissions.
    */
   public static SIPPermissions getInstance(){
      return s_permissions;
   }


   /**
    * Is this method permitted to be added in a SIP message?
    */
   public boolean isMethodPermitted( String strMethod ){
     boolean bRet = true;
     for( int i = 0; i<m_strArrayRestrictedMethods.length; i++ ){
        if( m_strArrayRestrictedMethods[i].equals(strMethod)){
            bRet = false;
            break;
        }
     }
     return bRet;
   }

    /**
     * Is this header permitted to be added in a SIP message ?
     */
   public boolean isHeaderPermitted( String strHeader ){
     boolean bRet = true;
     for( int i = 0; i<m_strArrayRestrictedHeaders.length; i++ ){
        if( m_strArrayRestrictedHeaders[i].equals(strHeader)){
            bRet = false;
            break;
        }
     }
     return bRet;
   }

}// Permissions
