/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/ConfigurationSetUpdateTag.java#4 $
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
import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.phone.Device;
import com.pingtel.pds.pgs.phone.DeviceGroup;
import com.pingtel.pds.pgs.phone.DeviceGroupHome;
import com.pingtel.pds.pgs.phone.DeviceHome;
import com.pingtel.pds.pgs.profile.ConfigurationSet;
import com.pingtel.pds.pgs.profile.ConfigurationSetHome;
import com.pingtel.pds.pgs.user.User;
import com.pingtel.pds.pgs.user.UserGroup;
import com.pingtel.pds.pgs.user.UserGroupHome;
import com.pingtel.pds.pgs.user.UserHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;
import java.util.Collection;
import java.util.Iterator;


/**
 * ConfigurationSetUpdateTag provides a way to create or update
 * configuration sets from JSPs.
 *
 * Four attributes are available:
 *      entitytype (should be 'user', 'usergroup', 'device' or 'devicegroup'
 *      entityid - the PK of the entity.
 *      cscontent - the string version of the XML to store
 *      profiletype - the PDSDefinitions prof type value of the type
 *                      of profile to create/update.   Only used for users
 *                      and usergroups.
 *
 */
public class ConfigurationSetUpdateTag extends ExTagSupport {

    private ConfigurationSetHome m_csHome = null;
    private DeviceHome m_deviceHome = null;
    private DeviceGroupHome m_deviceGroupHome = null;
    private UserHome m_userHome = null;
    private UserGroupHome m_userGroupHome = null;

    private String m_entityType = null;
    private Integer m_entityID = null;
    private String m_csContent = null;
    private int m_profileType = -1;

    public void setEntitytype ( String entitytype ) {
        this.m_entityType = entitytype;
    }

    public void setEntityid ( String entityid ) {
        this.m_entityID = new Integer ( entityid );
    }

    public void setCscontent ( String cscontent ) {
        this.m_csContent = cscontent;
    }

    public void setProfiletype ( String profiletype ) {
        this.m_profileType = new Integer ( profiletype ).intValue();
    }


    public int doStartTag() throws JspException {
        try {

            if ( m_entityType.equals( "user") || m_entityType.equals( "usergroup") ) {
                if (    new Integer ( m_profileType ).intValue() !=
                            PDSDefinitions.PROF_TYPE_APPLICATION_REF &&
                        new Integer ( m_profileType ).intValue() !=
                            PDSDefinitions.PROF_TYPE_USER ) {

                    throw new JspException ( "Invalid profile type given " );
                }
            }

            if ( m_csHome == null ) {
                m_csHome = ( ConfigurationSetHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  ConfigurationSetHome.class,
                                                                    "ConfigurationSet" );
            }

            ConfigurationSet cs = null;
            Collection csC = null;

            if ( m_entityType.equals( "device") ) {
                csC = m_csHome.findByLogicalPhoneID(  m_entityID );

                if ( csC == null ) {
                    if ( m_deviceHome == null ) {
                        m_deviceHome = ( DeviceHome )
                            EJBHomeFactory.getInstance().getHomeInterface(  DeviceHome.class,
                                                                            "Device" );
                    }

                    Device device = m_deviceHome.findByPrimaryKey( m_entityID );

                    m_csHome.create(    device.getRefConfigSetID(),
                                        PDSDefinitions.PROF_TYPE_PHONE,
                                        device,
                                        m_csContent );
                }
            }
            else if  (  m_entityType.equals( "devicegroup") ) {
                csC = m_csHome.findByDeviceGroupID( m_entityID );

                if ( csC == null ) {
                    if ( m_deviceGroupHome == null ) {
                        m_deviceGroupHome = ( DeviceGroupHome )
                            EJBHomeFactory.getInstance().getHomeInterface(  DeviceGroupHome.class,
                                                                            "DeviceGroup" );
                    }

                    DeviceGroup deviceGroup =
                        m_deviceGroupHome.findByPrimaryKey( m_entityID );

                    m_csHome.create(    deviceGroup.getRefConfigSetID(),
                                        PDSDefinitions.PROF_TYPE_PHONE,
                                        deviceGroup,
                                        m_csContent );
                }
            }
            else if  (  m_entityType.equals( "user") ) {

                csC = m_csHome.findByUserIDAndProfileType( m_entityID.toString(), m_profileType );

                if ( csC == null ) {
                    if ( m_userHome == null ) {
                        m_userHome = ( UserHome )
                            EJBHomeFactory.getInstance().getHomeInterface(  UserHome.class,
                                                                            "User" );
                    }

                    User user =
                        m_userHome.findByPrimaryKey( m_entityID.toString() );

                    m_csHome.create(    user.getRefConfigSetID(),
                                        m_profileType,
                                        user,
                                        m_csContent );
                }
            }
            else if  (  m_entityType.equals( "usergroup") ) {
                csC = m_csHome.findByUserGroupIDAndProfileType( m_entityID, m_profileType );

                if ( csC == null ) {
                    if ( m_userGroupHome == null ) {
                        m_userGroupHome = ( UserGroupHome )
                            EJBHomeFactory.getInstance().getHomeInterface(  UserGroupHome.class,
                                                                            "UserGroup" );
                    }

                    UserGroup userGroup =
                        m_userGroupHome.findByPrimaryKey( m_entityID );

                    m_csHome.create(    userGroup.getRefConfigSetID(),
                                        m_profileType,
                                        userGroup,
                                        m_csContent );
                }
            }
            else {
                throw new JspException ( "entitytype specified " + m_entityType + " is invalid" );
            }

            if ( csC != null ) {
                for ( Iterator i = csC.iterator(); i.hasNext(); )
                        cs = (ConfigurationSet) i.next();

                cs.setContent( m_csContent );
            }
        }
        catch (Exception e) {
            throw new JspTagException(e.getMessage());
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        m_entityType = null;
        m_entityID = null;
        m_csContent = null;
        m_profileType = -1;

        super.clearProperties();
    }
}