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
 
package com.pingtel.pds.pgs.phone;

import javax.ejb.CreateException;
import javax.ejb.EJBException;
import javax.ejb.FinderException;
import javax.ejb.SessionBean;
import javax.ejb.SessionContext;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.rmi.RemoteException;
import java.net.URL;
import java.net.MalformedURLException;
import java.util.Collection;
import java.util.Iterator;

import org.jdom.Attribute;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.output.XMLOutputter;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.profile.*;
import com.pingtel.pds.pgs.organization.Organization;


/**
 * DeviceTypeAdvocateBean is the EJBean implementation class for
 * DeviceTypeAdvocate.   DeviceTypeAdvocate is the session facade
 * for DeviceType entity.
 *
 *@author IB
 */
public class DeviceTypeAdvocateBean extends JDBCAwareEJB
    implements SessionBean, DeviceTypeAdvocateBusiness {


    /* EJBean Home references */
    private DeviceTypeHome m_deviceTypeHome;
    private CoreSoftwareHome m_cswHome;
    private CSProfileDetailHome m_cspdHome;
    private ManufacturerHome m_manufacturerHome;
    private RefDataAdvocate m_refDataAdvocateEJBObject;
    private RefConfigurationSetHome m_rcsHome;

    /**
     *  The Session Context object
     */
    private SessionContext m_ctx;

    private SAXBuilder m_saxBuilder;

    /**
     * importDeviceType is used during the post-installation process for
     * the config server.  It loads up device type definitions from the
     * various device type XML files and creates the associated entities
     * (ref properties, ref configuration sets, manufacturer details, ...).
     * It *does* use fileIO which is not part of the EJB spec however it
     * does so in a read-only way which means that it should be fine.
     * IMHO the spec is worried about non-TX file writes.
     *
     * @param fileName the name of the device definition XML file.
     * @return EJBObject for the new device type.
     * @throws PDSException is thrown for application level errors.
     */
    public DeviceType importDeviceType( String fileName ) throws PDSException {

        DeviceType pt = null;
        String manufacturerName = null;
        String modelName = null;
        Manufacturer manu = null;

        try {
            Document doc =  null;

            Exception ioe = null;
            try {
                doc = m_saxBuilder.build(new URL("file://" + fileName));
            }
            catch ( JDOMException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC100",
                                            "E3013",
                                            new Object[]{ fileName } ) );
            }
            catch ( java.net.MalformedURLException mfue ) {
                ioe = mfue;
            }
            catch ( java.io.IOException ex ) {
                ioe = ex;
            }

            if (ioe != null)
            {
                logFatal( ioe.toString(), ioe );

                throw new EJBException(
                    collateErrorMessages(   "UC930",
                                            "E4055",
                                            new Object[]{ "Pingtel xpressa"}));
            }

            Element root = doc.getRootElement();
            logDebug ( "goot root element ");

            XMLOutputter xmlOut = new XMLOutputter();

            Element pdsPhoneMeta = root;

            Element deviceType = root.getChild("device_type");


            Element manufacturer = deviceType.getChild("manufacturer");
            manufacturerName = manufacturer.getTextNormalize();
            logDebug ( "manufacturer is: " + manufacturerName );

            Collection cManu = null;

            try {
                cManu = m_manufacturerHome.findByName( manufacturerName );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC100",
                                            "E1034",
                                            new Object[]{ manufacturerName } ) );
            }

            for (Iterator iManu = cManu.iterator(); iManu.hasNext(); ) {
                manu = (Manufacturer) iManu.next();
            }

            if (manu == null) {
                try {
                    manu = m_manufacturerHome.create(manufacturer.getTextNormalize());
                    logDebug ( "created manufacturer" );
                }
                catch ( CreateException ex ) {
                    m_ctx.setRollbackOnly();

                    throw new PDSException(
                        collateErrorMessages(   "UC100",
                                                "E2023",
                                                new Object[]{ manufacturer.getTextNormalize() } ) );
                }

            }

            Element model = deviceType.getChild("model");
            modelName = model.getTextNormalize();
            logDebug ( "got model: " + modelName );

            try {
                pt = m_deviceTypeHome.create( manu.getID(), modelName );
            }
            catch ( CreateException ex ) {
                    m_ctx.setRollbackOnly();

                    throw new PDSException(
                        collateErrorMessages(   "UC100",
                                                "E2006",
                                                new Object[]{   manu.getExternalID(),
                                                                modelName } ) );
            }

            Collection profileTypes = deviceType.getChildren("profile");



            int profTypeInt = -1;

            for (Iterator iProfTypes = profileTypes.iterator();
                    iProfTypes.hasNext(); ) {

                Element profType = (Element) iProfTypes.next();
                String type = profType.getAttributeValue("type");
                logDebug ( "loading up profile: " + type );

                if (type.equalsIgnoreCase("phone")) {
                    profTypeInt = PDSDefinitions.PROF_TYPE_PHONE;
                } else if (type.equalsIgnoreCase("user")) {
                    profTypeInt = PDSDefinitions.PROF_TYPE_USER;
                } else if (type.equalsIgnoreCase("application-ref")) {
                    profTypeInt =PDSDefinitions.PROF_TYPE_APPLICATION_REF;
                }

                Collection refPropsList = profType.getChildren("ref_property");

                for (Iterator iRefProps = refPropsList.iterator();
                        iRefProps.hasNext(); ) {

                    Element refProperty = (Element) iRefProps.next();
                    Element def = refProperty.getChild("definition");
                    String definitionContent = xmlOut.outputString(def);
                    //logDebug ( "about to create refproperty name: " + def.getAttributeValue("name") );

                    m_refDataAdvocateEJBObject.createRefProperty( def.getAttributeValue("name"),
                                                //pt.getID(),
                                                profTypeInt,
                                                refProperty.getAttributeValue( "code"),
                                                definitionContent );

                } // for ref properties
            } // for profile types


            logDebug (  "doing core software types" );
            Element coreSoftwareTypes =
                    pdsPhoneMeta.getChild("core_software_types");

            Collection coreSoftware =
                    coreSoftwareTypes.getChildren("core_software");

            CoreSoftware newCS = null;

            for (Iterator csI = coreSoftware.iterator(); csI.hasNext(); ) {
                Element cs = (Element) csI.next();

                Element description = cs.getChild("description");
                Element version = cs.getChild("version");
                Element name = cs.getChild("name");

                try {
                    logDebug ( "about to create core software" );
                    newCS = m_cswHome.create(    description.getTextNormalize(),
                                                version.getTextNormalize(),
                                                name.getTextNormalize(),
                                                pt.getID().toString() );
                }
                catch ( CreateException ex ) {
                    m_ctx.setRollbackOnly();

                    throw new PDSException(
                        collateErrorMessages(   "UC100",
                                                "E2021",
                                                new Object[]{   name.getTextNormalize(),
                                                                version.getTextNormalize(),
                                                                description.getTextNormalize() } ) );
                }

                Collection profiles = cs.getChildren("core_software_profile");

                for (Iterator profI = profiles.iterator(); profI.hasNext(); ) {

                    Element prof = (Element) profI.next();
                    Attribute type = prof.getAttribute("type");

                    Integer realType = null;

                    if (type.getValue().equals("phone")) {
                        realType = new Integer( PDSDefinitions.PROF_TYPE_PHONE );
                    } else if (type.getValue().equals("user")) {
                        realType = new Integer( PDSDefinitions.PROF_TYPE_USER );
                    } else if (type.getValue().equals("application-ref")) {
                        realType = new Integer( PDSDefinitions.PROF_TYPE_APPLICATION_REF );
                    }

                    Element profileFileName = prof.getChild("profile_file_name");
                    Element renderClass =
                            prof.getChild("profile_renderer_class");
                    Element renderXSLT =
                            prof.getChild("profile_renderer_stylesheet");
                    Element projectionClass =
                            prof.getChild("profile_projection_class");

                    try {
                        logDebug ( "about to create core software profile" );
                        m_cspdHome.create(newCS.getID(),
                                realType,
                                profileFileName.getTextNormalize(),
                                renderClass.getTextNormalize(),
                                renderXSLT.getTextNormalize(),
                                projectionClass.getTextNormalize());
                    }
                    catch ( CreateException ex ) {
                        m_ctx.setRollbackOnly();

                        throw new PDSException(
                            collateErrorMessages(   "UC100",
                                                    "E2022",
                                                    new Object[]{   newCS.getExternalID(),
                                                                    type.getValue() } ) );
                    }
                } // for profiles
            } // for core software definitions

            return pt;
        }

        catch (PDSException ex) {
            m_ctx.setRollbackOnly();
            throw new PDSException( collateErrorMessages ( "UC100" ), ex  );
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                collateErrorMessages(   "UC100",
                                        "E4012",
                                        new Object []{  manufacturerName,
                                                        modelName }));

        }
    }

    /**
     * creates a device specific ref configuration set for the given device type and
     * organization.  These are used to produce a basic RCS for devices.
     * @param organization
     * @param deviceType
     */
    public void createDeviceSpecificRefConfigurationSet ( Organization organization,
                                                          DeviceType deviceType ) {

        logDebug ( "creating device specific rcs" );

        try {

                RefConfigurationSet rcs =
                    m_rcsHome.create(   deviceType.getModel() + " Device",
                                        organization.getID() );

                Collection refProperties =
                    m_refDataAdvocateEJBObject.getRefPropertiesForDeviceTypeAndProfile(
                        deviceType.getID(),
                        new Integer ( PDSDefinitions.PROF_TYPE_PHONE) );

                RefConfigSetAssignment rule = null;

                for (Iterator iRP = refProperties.iterator(); iRP.hasNext(); ) {
                    RefProperty rp = (RefProperty) iRP.next();
                    rule = new RefConfigSetAssignment( rp.getID(), false, false);
                    m_refDataAdvocateEJBObject.assignRefPropertytoRCS( rcs, rule );
                }

        }
        catch ( Exception e) {
            logFatal ( e.getMessage() );
            throw new EJBException ( e.getMessage() );
        }
    }



    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     */
    public void ejbCreate() { }


    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     */
    public void ejbRemove() { }


    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     */
    public void ejbActivate() { }


    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     */
    public void ejbPassivate() { }


    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     *
     *@param  ctx  The new sessionContext value
     */
    public void setSessionContext(SessionContext ctx) {
        m_ctx = ctx;

        try {
            Context initial = new InitialContext();

            m_deviceTypeHome = (DeviceTypeHome) initial.lookup("DeviceType");
            m_cswHome = (CoreSoftwareHome) initial.lookup("CoreSoftware");
            m_manufacturerHome = (ManufacturerHome) initial.lookup("Manufacturer");

            m_cspdHome = (CSProfileDetailHome)
                    initial.lookup("CSProfileDetail");

            RefDataAdvocateHome refDataAdvocateHome =
                    (RefDataAdvocateHome) initial.lookup("RefDataAdvocate");

            m_refDataAdvocateEJBObject = refDataAdvocateHome.create();

            m_rcsHome = (RefConfigurationSetHome)
                    initial.lookup("RefConfigurationSet");

            m_saxBuilder = new SAXBuilder();

        } catch ( Exception e) {
            logFatal( e.toString(), e );
            throw new EJBException( e );
        }
    }


    /**
     *  Description of the Method
     */
    public void unsetSessionContext() {
        m_ctx = null;

    }

}
