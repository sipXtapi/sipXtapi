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
 
package com.pingtel.pds.pgs.beans;

import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.pgs.phone.DeviceType;
import com.pingtel.pds.pgs.phone.DeviceTypeHome;
import com.pingtel.pds.pgs.profile.*;

import javax.servlet.jsp.JspTagException;
import java.util.Collection;
import java.util.Iterator;

/**
 * ScriptletUtilityBean contains methods that are used in more than one
 * JSP.   Wherever possible our logic in contained in our custom tag
 * lib or other Javabeans, where we have fallen short of that the code
 * should be migrated into one of those two sources when it is appropriate.
 *
 * This is an appropriate place for utility code used in scriptlets.
 *
 * @author ibutcher
 *
 */
public class ScriptletUtilityBean {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////
    private RefConfigurationSetHome mRefConfigSetHome;
    private DeviceTypeHome mDeviceTypeHome;
    private RefDataAdvocate mRefDataAdvocateEJBObject;


//////////////////////////////////////////////////////////////////////////
// Construction
////
    public ScriptletUtilityBean() throws JspTagException {
        try {
            mRefConfigSetHome = (RefConfigurationSetHome)
                        EJBHomeFactory.getInstance().getHomeInterface(
                                RefConfigurationSetHome.class,
                                "RefConfigurationSet");

            mDeviceTypeHome = (DeviceTypeHome)
                    EJBHomeFactory.getInstance().getHomeInterface(
                            DeviceTypeHome.class, "DeviceType");

            RefDataAdvocateHome refDataAdvocateHome = (RefDataAdvocateHome)
                    EJBHomeFactory.getInstance().getHomeInterface(
                            RefDataAdvocateHome.class, "RefDataAdvocate");

            mRefDataAdvocateEJBObject = refDataAdvocateHome.create();
        }
        catch(Exception e) {
            throw new JspTagException(e.getMessage());
        }

    }


//////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * some of the pgs tag lib tags require a RefConfigurationSetId as
     * one of their attributes.    getCompleteDeviceRcsId encapsulates
     * the common code to find RefConfigurationSet called "Complete Device".
     *
     * @return the (String) PK of the "Complete Device" RefConfigurationSet.
     * @throws JspTagException for all checked errors.
     */
    public String getCompleteDeviceRcsId()
            throws JspTagException {

        String refConfigurationSetId = null;

        try {
            Collection sets = mRefConfigSetHome.findByName("Complete Device");
            Iterator setsIterator = sets.iterator();
            com.pingtel.pds.pgs.profile.RefConfigurationSet refSetProp;
            while( setsIterator.hasNext() ){
                refSetProp = (RefConfigurationSet) setsIterator.next();
                refConfigurationSetId = refSetProp.getID().toString();
            }

        }
        catch (Exception e ) {
            throw new JspTagException(e.getMessage());
        }

        return refConfigurationSetId;
    }


    /**
     * getDeviceTypeId returns a String value of the PK for the given
     * device type (name).
     *
     * @param deviceType name of the device type that you want to get the
     * PK for.  You should the the constants defined in PDSDefinitions
     * for this parameter.
     * @return PK for the given device type.
     * @throws JspTagException for all checked errors.
     * @see com.pingtel.pds.common.PDSDefinitions#MODEL_HARDPHONE_XPRESSA
     *
     */
    public String getDeviceTypeId(String deviceType) throws JspTagException {

        String deviceTypeId = null;

        try {
            Collection sets = mDeviceTypeHome.findByModel(deviceType);

            Iterator setsIterator = sets.iterator();

            while(setsIterator.hasNext()){
                DeviceType deviceTypeProp = (DeviceType) setsIterator.next();
                deviceTypeId = deviceTypeProp.getID().toString();
            }
        }
        catch (Exception ioe ) {
            throw new JspTagException(ioe.getMessage());
        }

        return deviceTypeId;
    }


    /**
     * getXpressaApplicationRefPropId returns the String value of the
     * PK of the RefProperty which represents 'user' applications for
     * Pingtel xpressa devices.
     *
     * @return
     * @throws JspTagException
     */
    public String getXpressaApplicationRefPropId() throws JspTagException {

        String refpropertyid = null;

        try {
            Collection sets =
                    mDeviceTypeHome.findByModel(
                            PDSDefinitions.MODEL_HARDPHONE_XPRESSA);

            Iterator setsIterator = sets.iterator();
            DeviceType deviceTypeProp;
            RefProperty refProperty;

            while( setsIterator.hasNext() ){
                deviceTypeProp = (DeviceType) setsIterator.next();

                Collection rdaCollection =
                        mRefDataAdvocateEJBObject.getRefPropertiesForDeviceTypeAndProfile(
                                deviceTypeProp.getID(),
                                new Integer(PDSDefinitions.PROF_TYPE_APPLICATION_REF));

                Iterator rdaIterator = rdaCollection.iterator();
                while(rdaIterator.hasNext()){
                    refProperty = (RefProperty) rdaIterator.next();
                    refpropertyid = refProperty.getID().toString();
                }
            }
        }
        catch (Exception ioe ) {
            throw new JspTagException(ioe.getMessage());
        }

        return refpropertyid;

    }

//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////



//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}
