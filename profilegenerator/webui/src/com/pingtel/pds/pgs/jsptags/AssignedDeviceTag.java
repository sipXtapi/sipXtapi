/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/AssignedDeviceTag.java#4 $
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
import com.pingtel.pds.pgs.phone.DeviceHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;
import java.util.*;


public class AssignedDeviceTag extends ExTagSupport {

   private DeviceHome m_dHome = null;
   private TreeMap m_map1 = new TreeMap();
   private String m_userid = null;

   public void setUserid( String userid ){
        m_userid = userid;
    }


    public int doStartTag() throws JspException {
        try {
            if ( m_dHome == null ) {
                m_dHome = ( DeviceHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  DeviceHome.class,
                                                                    "Device" );
            }

            Collection dCollection = m_dHome.findByUserID( m_userid );
            Iterator dIterator = dCollection.iterator();
            StringBuffer m_sBuffer = new StringBuffer();
            com.pingtel.pds.pgs.phone.Device m_dProp;

            while( dIterator.hasNext() ) {
                m_dProp = ( com.pingtel.pds.pgs.phone.Device )
                dIterator.next();
                m_map1.put( m_dProp.getShortName(), m_dProp.getID() );
            }

            Set set1 = m_map1.entrySet();
            Iterator mapIterator = set1.iterator();
            while( mapIterator.hasNext() ) {
                Map.Entry me = ( Map.Entry )mapIterator.next();
                m_sBuffer.append( "<option value=\"" +
                me.getValue() + "\">" +
                me.getKey() +
                "\n");
            }

            pageContext.getOut().println( "<select name=\"deviceid\" size=\"" );
            pageContext.getOut().println( dCollection.size() < 20 ? dCollection.size() + 1 : 20 );
            pageContext.getOut().println( "\">" );
            pageContext.getOut().println( "<option>Select Device to Unassign");
            pageContext.getOut().println( m_sBuffer );
            pageContext.getOut().println( "</select>" );
      }
      catch (Exception ioe ) {
         throw new JspTagException( ioe.getMessage() );
      }

      return SKIP_BODY;
   }

   protected void clearProperties() {
      m_map1.clear();
      m_userid = null;

      super.clearProperties();
   }
}