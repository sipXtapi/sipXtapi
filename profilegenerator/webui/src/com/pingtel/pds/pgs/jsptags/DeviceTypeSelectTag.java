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

import java.util.Collection;
import java.util.Iterator;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;

import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.phone.CoreSoftwareHome;
import com.pingtel.pds.pgs.phone.DeviceTypeHelper;
import com.pingtel.pds.pgs.phone.DeviceTypeHome;
import com.pingtel.pds.pgs.phone.Manufacturer;
import com.pingtel.pds.pgs.phone.ManufacturerHome;

public class DeviceTypeSelectTag extends ExTagSupport {

   private DeviceTypeHome m_deviceTypeHome = null;
   private CoreSoftwareHome m_coreVersionHome = null;
   private ManufacturerHome m_mfgHome = null;
   private Manufacturer mfg = null;

   private String m_refProp = null;
   private String m_coreVersion = null;
   private StringBuffer m_dtBuffer = new StringBuffer();
   private StringBuffer m_rpBuffer = new StringBuffer();
   private int i = 0;
   
   public void setRefProp( String refProp )
   {
      this.m_refProp = refProp;
   }
   public void setCoreVersion( String coreVersion )
   {
      this.m_coreVersion = coreVersion;
   }


   public int doStartTag() throws JspException
   {
      try {
         if ( m_deviceTypeHome == null ) {
            m_deviceTypeHome = ( DeviceTypeHome )
                EJBHomeFactory.getInstance().getHomeInterface(  DeviceTypeHome.class,
                                                                "DeviceType" );

             // @DB - added to get mfg name, for display in select box
             m_mfgHome = ( ManufacturerHome )
                     EJBHomeFactory.getInstance().getHomeInterface( ManufacturerHome.class,
                                                                "Manufacturer");

            if( m_coreVersion.equalsIgnoreCase( "true" )  ) {
                m_coreVersionHome = ( CoreSoftwareHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  CoreSoftwareHome.class,
                                                                "CoreSoftware" );
            }
         }

         Collection deviceTypes = m_deviceTypeHome.findAll();
         Iterator deviceTypeIterator = deviceTypes.iterator();
         com.pingtel.pds.pgs.phone.DeviceType m_deviceTypeProp;
         if( m_coreVersion.equalsIgnoreCase( "true" ) )
         {

            com.pingtel.pds.pgs.phone.CoreSoftware m_coreVersionProp;
            while( deviceTypeIterator.hasNext() )
            {

               m_deviceTypeProp = ( com.pingtel.pds.pgs.phone.DeviceType )
                  deviceTypeIterator.next();


               Collection coreVersionColl =
                  m_coreVersionHome.findByDeviceType( m_deviceTypeProp.getID() );

               Iterator coreVersionIterator = coreVersionColl.iterator();
               if( coreVersionIterator.hasNext() )
               {

                  // only display devicetypes if they have coresoftware
                  m_dtBuffer.append( "form.options[" + i + "] = new Option(" +
                                  "\"" +  m_deviceTypeProp.getModel() +
                                  "\",\"" +  m_deviceTypeProp.getID() +
                                  "\");\n");

                  m_rpBuffer.append( "refList.addOptions(\"" +
                                     m_deviceTypeProp.getID() +
                                     "\"" );

                  while( coreVersionIterator.hasNext() )
                  {
                     m_coreVersionProp = ( com.pingtel.pds.pgs.phone.CoreSoftware )
                        coreVersionIterator.next();
                     m_rpBuffer.append( ",\"" + m_coreVersionProp.getVersion() +
                                        "\",\"" + m_coreVersionProp.getID() +
                                        "\"" );

                  }
                  m_rpBuffer.append( ");\n" );
               }
               i++;
            }

         }
         else
         {
            while( deviceTypeIterator.hasNext() )
            {
               m_deviceTypeProp = ( com.pingtel.pds.pgs.phone.DeviceType )
                  deviceTypeIterator.next();
                  mfg = m_mfgHome.findByPrimaryKey(m_deviceTypeProp.getManufacturerID());
                  String label = DeviceTypeHelper.getLabel(m_deviceTypeProp.getModel(), 
                          mfg.getName());
                  m_dtBuffer.append( "<option value=\"" +
                                     m_deviceTypeProp.getID() + "\">" +
                                     label +
                                     "\n");
            }
         }

         if( m_refProp.equalsIgnoreCase( "true" ) )
         {
            pageContext.getOut().println( "<script language=\"JavaScript\">" );
            pageContext.getOut().println( "var refList = new DynamicOptionList(\"" +
                                          "refpropertyid\",\"devicetypeid\");" );
            pageContext.getOut().println( m_rpBuffer );
            pageContext.getOut().println( "function createMainList() {" );
            pageContext.getOut().println( "var form = document.forms[0].devicetypeid;" );
            pageContext.getOut().println( m_dtBuffer );
            pageContext.getOut().println( "}" );
            pageContext.getOut().println( "</script>" );
         }
          else if( m_coreVersion.equalsIgnoreCase( "true " ) )
          {
            pageContext.getOut().println( "<script language=\"JavaScript\">" );
            pageContext.getOut().println( "var refList = new DynamicOptionList(\"" +
                                          "coresoftwaredetailsid\",\"devicetypeid\");" );
            pageContext.getOut().println( m_rpBuffer );
            pageContext.getOut().println( "function createMainList() {" );
            pageContext.getOut().println( "var form = document.forms[0].devicetypeid;" );
            pageContext.getOut().println( m_dtBuffer );
            pageContext.getOut().println( "}" );
            pageContext.getOut().println( "</script>" );
          }
          else
          {
            pageContext.getOut().println( "<select name=\"devicetypeid\" size=\"" );
            pageContext.getOut().println( deviceTypes.size() < 20 ? deviceTypes.size() : 20 );
            pageContext.getOut().println( "\" onchange=\"preLoadSerialNumber(this.value)\">" );
            pageContext.getOut().println( m_dtBuffer );
            pageContext.getOut().println( "</select>" );
          }

      }
      catch (Exception ioe ) {
         throw new JspTagException( ioe.getMessage() );
      }

      return SKIP_BODY;
   }

   protected void clearProperties()
   {
      m_refProp = null;
      m_coreVersion = null;

      super.clearProperties();
   }
}