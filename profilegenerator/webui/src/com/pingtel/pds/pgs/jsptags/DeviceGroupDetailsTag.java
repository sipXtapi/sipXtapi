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
import com.pingtel.pds.common.PropertyGroupLink;
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import com.pingtel.pds.pgs.phone.*;
import com.pingtel.pds.pgs.profile.*;
import org.jdom.Element;

import javax.ejb.FinderException;
import javax.servlet.jsp.JspException;
import java.rmi.RemoteException;
import java.util.*;


/**
 * <p>Title: ListDevicesTag</p>
 * <p>Description: Tag Library for the DMS JSP Pages</p>
 * <p>Copyright: Copyright (c) 2002</p>
 * <p>Company: Pingtel Corp</p>
 * @author John P. Coffey
 * @version 1.0
 */

public class DeviceGroupDetailsTag extends StyleTagSupport {

    private Integer m_deviceGroupID;
    private String m_refPropertyGroupID;
    private String m_deviceTypeID;
    private String m_mfgID;
    private String m_userType;
    private String m_propName;
    private String m_vcuid;
    private String m_level;
    private String m_visiblefor;
    private String m_detailsType;
    private Element m_mappingsDoc = null;

    private DeviceHome m_deviceHome = null;
    private DeviceGroupHome m_deviceGroupHome = null;
    private RefPropertyHome m_rpHome = null;
    private RefConfigurationSetHome m_rcsHome = null;
    private DeviceTypeHome m_dtHome = null;
    private ConfigurationSetHome m_csHome = null;
    private ManufacturerHome m_manuHome = null;


    public void setDevicegroupid ( String devicegroupid ) {
        this.m_deviceGroupID = new Integer ( devicegroupid );
    }

    public void setRefpropertygroupid ( String refpropertygroupid ) {
        this.m_refPropertyGroupID = refpropertygroupid;
    }

    public void setDevicetypeid ( String devicetypeid ) {
        this.m_deviceTypeID = devicetypeid;
    }

    public void setMfgid ( String mfgid ) {
        this.m_mfgID = mfgid;
    }

    public void setUsertype ( String usertype ) {
        this.m_userType = usertype;
    }

    public void setPropname ( String propname ) {
        this.m_propName = propname;
    }

    public void setVcuid ( String vcuid ) {
        this.m_vcuid = vcuid;
    }

    public void setDetailstype ( String detailstype ) {
        this.m_detailsType = detailstype;
    }

    public void setLevel ( String level ) {
        this.m_level = level;
    }

    public void setVisiblefor ( String visiblefor ) {
        this.m_visiblefor = visiblefor;
    }

    /**
     * This list all the devices here
     */
    public int doStartTag() throws JspException {
        try {
            if ( m_deviceHome == null ) {
                m_deviceHome = ( DeviceHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  DeviceHome.class,
                                                                    "Device" );

                m_deviceGroupHome = ( DeviceGroupHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  DeviceGroupHome.class,
                                                                    "DeviceGroup" );

                m_rpHome = ( RefPropertyHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  RefPropertyHome.class,
                                                                    "RefProperty" );

                m_rcsHome = ( RefConfigurationSetHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  RefConfigurationSetHome.class,
                                                                    "RefConfigurationSet" );

                m_dtHome = ( DeviceTypeHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  DeviceTypeHome.class,
                                                                    "DeviceType" );

                m_csHome = ( ConfigurationSetHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  ConfigurationSetHome.class,
                                                                    "ConfigurationSet" );

                m_manuHome = ( ManufacturerHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  ManufacturerHome.class,
                                                                    "Manufacturer" );

            }

            Element documentRootElement = new Element("details");

            DeviceGroup deviceGroup =
                m_deviceGroupHome.findByPrimaryKey( m_deviceGroupID );

            Element attributes = createElementFromObject( "attributes", deviceGroup );

            if ( deviceGroup.getParentID() != null ) {
                DeviceGroup parentGroup =
                    m_deviceGroupHome.findByPrimaryKey( deviceGroup.getParentID() );

                Element parentName = new Element ( "parentname" );
                parentName.setText( parentGroup.getName() );
                attributes.addContent( parentName );
            }
            else {
                Element parentName = new Element ( "parentname" );
                attributes.addContent( parentName );
            }

            ArrayList workingSet = new ArrayList();
            Collection inelligable =
                getInelligableParentGroups (    deviceGroup,
                                                workingSet );

            Element possibleGroups = new Element ( "possible_groups" );
            attributes.addContent( possibleGroups );

            // Need to add parent org's groups at a later date.
            Collection allGroups =
                m_deviceGroupHome.findByOrganizationID( deviceGroup.getOrganizationID() );

            for ( Iterator allGroupsI = allGroups.iterator(); allGroupsI.hasNext(); ) {
                DeviceGroup possGroup = (DeviceGroup) allGroupsI.next();

                if (    !inelligable.contains( possGroup ) &&
                        !possGroup.getID().equals( deviceGroup.getID() ) ) {

                    Element possibleGroupElem = new Element ( "group" );
                    possibleGroups.addContent( possibleGroupElem );
                    possibleGroupElem.addContent(
                        new Element ( "id" ).setText( possGroup.getID().toString() ) );
                    possibleGroupElem.addContent(
                        new Element ( "name" ).setText( possGroup.getName() ) );
                }
            }

            documentRootElement.addContent( attributes );

            Element csElement = new Element ( "configurationset" );
            documentRootElement.addContent( csElement );

            Collection cCS = m_csHome.findByDeviceGroupID( m_deviceGroupID );
            for ( Iterator iCS = cCS.iterator(); iCS.hasNext(); ) {
                ConfigurationSet cs = (ConfigurationSet) iCS.next();
                csElement.addContent( cs.getContent() );
            }

            LinkedList allDeviceTypes = new LinkedList ();
            Collection cAllDeviceTypes = m_dtHome.findAll();

            for ( Iterator iAll = cAllDeviceTypes.iterator(); iAll.hasNext(); ) {
                DeviceType deviceType = (DeviceType) iAll.next();
                allDeviceTypes.add( deviceType );
            }

            LinkedList lightweightDeviceTypes = new LinkedList();

            for (   Iterator iLightweight = allDeviceTypes.iterator();
                    iLightweight.hasNext(); ) {

                DeviceType deviceType = (DeviceType) iLightweight.next();
                Manufacturer manu =
                    m_manuHome.findByPrimaryKey( deviceType.getManufacturerID() );

                lightweightDeviceTypes.add(
                    new LightweightDeviceType ( manu.getName(),
                                                deviceType.getModel() ) );
            }

            Collections.sort(   lightweightDeviceTypes,
                                new LightweightDeviceTypeComparator () );

            Element modelsElement = new Element ( "models" );
            documentRootElement.addContent( modelsElement );

            for (   Iterator iLightweight = lightweightDeviceTypes.iterator();
                    iLightweight.hasNext(); ) {

                LightweightDeviceType ldt =
                    (LightweightDeviceType) iLightweight.next();
                Element modelElement = new Element ( "model" );
                modelElement.addContent( ldt.getModel() );
                modelsElement.addContent( modelElement );
            }

            Element refPropertiesElement = new Element ( "refproperties" );
            documentRootElement.addContent( refPropertiesElement );

            Collection rpC = m_rpHome.findAll();
            for ( Iterator rpI = rpC.iterator(); rpI.hasNext(); ) {
                RefProperty rp = (RefProperty) rpI.next();
                Element rpElement = new Element ( "refproperty" );
                refPropertiesElement.addContent( rpElement );

                Element code = new Element ( "code");
                code.addContent( rp.getCode() );
                rpElement.addContent( code  );
                Element id = new Element ( "id" );
                id.addContent( rp.getID().toString());
                rpElement.addContent( id );
                Element name = new Element ( "name" );
                name.addContent( rp.getName());
                rpElement.addContent( name );
                Element cont = new Element ( "content" );
                cont.addContent(  rp.getContent() );
                rpElement.addContent( cont );
            }

            Element mappingElement = new Element ( "refpropertygroup_rp_mappings" );
            documentRootElement.addContent( mappingElement );

            if ( m_mappingsDoc == null ) {
                m_mappingsDoc = getMappingsDoc();
            }

            mappingElement.addContent( m_mappingsDoc );


            RefConfigurationSet rcs =
                m_rcsHome.findByPrimaryKey( deviceGroup.getRefConfigSetID() );

            Element rcsElement =
                createElementFromObject ( "refconfigurationset", rcs );

            documentRootElement.addContent( rcsElement );

            Collection rcsProperties =
                m_rpHome.findByRefConfigurationSetID( rcs.getID() );

            for ( Iterator iRCS = rcsProperties.iterator(); iRCS.hasNext(); ) {
                RefProperty rp = (RefProperty) iRCS.next();

                Element rpElement = new Element ( "refproperty" );
                rpElement.addContent( rp.getID().toString() );

                rcsElement.addContent( rpElement );
            }

            HashMap stylesheetParameters = new HashMap();
            stylesheetParameters.put(   "refpropertygroupid",
                                        this.m_refPropertyGroupID );
            stylesheetParameters.put(   "devicetypeid",
                                        this.m_deviceTypeID );
            stylesheetParameters.put(   "mfgid",
                                        this.m_mfgID );
            stylesheetParameters.put(   "usertype",
                                        this.m_userType );
            stylesheetParameters.put(   "propname",
                                        this.m_propName );
            stylesheetParameters.put(   "vcuid",
                                        this.m_vcuid );
            stylesheetParameters.put(   "detailstype",
                                        this.m_detailsType );
            stylesheetParameters.put(  "level",
                                        this.m_level );
            stylesheetParameters.put(   "visiblefor",
                                        this.m_visiblefor );

            // Sends the text to the browser in either xml or html format
            // depending on an optional debug tag attribute
            outputTextToBrowser ( documentRootElement, stylesheetParameters );
        }
        catch(Exception ex ) {
             throw new JspException( ex );
        }

        return SKIP_BODY;
    }


    private Element getMappingsDoc() {
        return PropertyGroupLink.getInstance().getMappings();
    }


    private Collection getInelligableParentGroups ( DeviceGroup deviceGroup,
                                                    Collection workingSet )
        throws JspException {

        try {
            Collection children =
                m_deviceGroupHome.findByParentID( deviceGroup.getID() );

            for ( Iterator i = children.iterator(); i.hasNext(); ) {
                DeviceGroup child = (DeviceGroup) i.next();
                workingSet.add( child );
                workingSet.addAll( getInelligableParentGroups ( child, workingSet ) );
            }

            return workingSet;
        }
        catch ( RemoteException ex ) {
            throw new JspException ( ex.toString() );
        }
        catch ( FinderException ex ) {
            throw new JspException ( ex.toString() );
        }
    }

    /**
     * Called by super class method (not by superClass's clearProperties
     * method).
     */
    protected void clearProperties() {
        m_deviceGroupID = null;

        super.clearProperties();
    }

}