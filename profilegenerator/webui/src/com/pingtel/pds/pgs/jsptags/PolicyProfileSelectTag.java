/*
 * 
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
import com.pingtel.pds.pgs.profile.RefConfigurationSetHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;
import java.util.Collection;
import java.util.Iterator;


public class PolicyProfileSelectTag extends ExTagSupport {

   private RefConfigurationSetHome m_refConfSetHome = null;
   private String m_type = null;

   public void setType ( String type ) {
       this.m_type = type;
   }


   public int doStartTag() throws JspException
   {
      try {
         if ( m_refConfSetHome == null ) {
            m_refConfSetHome = ( RefConfigurationSetHome )
                EJBHomeFactory.getInstance().getHomeInterface(  RefConfigurationSetHome.class,
                                                                "RefConfigurationSet" );
         }

         Collection sets = m_refConfSetHome.findByOrganizationID( new Integer( getOrganizationID() ) );
         Iterator setsIterator = sets.iterator();
         StringBuffer m_sBuffer = new StringBuffer();
         com.pingtel.pds.pgs.profile.RefConfigurationSet m_refSetProp;
         while( setsIterator.hasNext() )
         {
            m_refSetProp = ( com.pingtel.pds.pgs.profile.RefConfigurationSet )
               setsIterator.next();
            if ( m_refSetProp.getName().indexOf( m_type ) != -1 )
            {
               m_sBuffer.append( "<option value=\"" +
                                   m_refSetProp.getID() + "\">" +
                                   m_refSetProp.getName() +
                                   "\n");
            }
         }

         pageContext.getOut().println( "<select name=\"refconfigsetid\" size=\"" );
         pageContext.getOut().println( sets.size() < 20 ? sets.size() : 20 );
         pageContext.getOut().println( "\">" );
         pageContext.getOut().println( m_sBuffer );
         pageContext.getOut().println( "</select>" );
      }
      catch (Exception ioe ) {
         throw new JspTagException( ioe.getMessage() );
      }

      return SKIP_BODY;
   }

   protected void clearProperties()
   {
      m_type = null;
      super.clearProperties();
   }
}






