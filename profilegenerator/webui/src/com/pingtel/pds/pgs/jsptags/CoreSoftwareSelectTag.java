/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/CoreSoftwareSelectTag.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.jsptags;

import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.phone.CoreSoftwareHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;
import java.util.Collection;
import java.util.Iterator;

public class CoreSoftwareSelectTag extends ExTagSupport {

   private CoreSoftwareHome m_coreSoftwareHome = null;


   public int doStartTag() throws JspException
   {
      try {
         if ( m_coreSoftwareHome == null ) {
            m_coreSoftwareHome = ( CoreSoftwareHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  CoreSoftwareHome.class,
                                                                    "CoreSoftware" );
         }

         Collection software = m_coreSoftwareHome.findAll();
         Iterator coreSoftwareIterator = software.iterator();
         StringBuffer m_sBuffer = new StringBuffer();
         com.pingtel.pds.pgs.phone.CoreSoftware m_coreSoftwareProp;
         while( coreSoftwareIterator.hasNext() )
         {
            m_coreSoftwareProp = ( com.pingtel.pds.pgs.phone.CoreSoftware )
               coreSoftwareIterator.next();
               m_sBuffer.append( "<option value=\"" +
                                   m_coreSoftwareProp.getID() + "\">" +
                                   m_coreSoftwareProp.getVersion() +
                                   "\n");
         }

         pageContext.getOut().println( "<select name=\"coresoftwaredetailsid\" size=\"" );
         pageContext.getOut().println( software.size() < 20 ? software.size() : 20 );
         pageContext.getOut().println( "\">" );
         pageContext.getOut().println( m_sBuffer );
         pageContext.getOut().println( "</select>" );
      }
      catch (Exception ioe) {
         throw new JspTagException( ioe.getMessage() );
      }

      return SKIP_BODY;
   }

   protected void clearProperties()
   {
      super.clearProperties();
   }
}






