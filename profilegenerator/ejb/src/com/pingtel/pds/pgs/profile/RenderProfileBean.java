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

package com.pingtel.pds.pgs.profile;

import java.io.ByteArrayInputStream;
import java.rmi.RemoteException;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.Properties;

import javax.ejb.EJBException;
import javax.ejb.FinderException;
import javax.ejb.SessionBean;
import javax.ejb.SessionContext;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;

import org.jdom.Document;
import org.jdom.Element;
import org.jdom.input.SAXBuilder;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.common.MasterDetailsMap;
import com.pingtel.pds.pgs.common.RMIConnectionManager;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.phone.CSProfileDetail;
import com.pingtel.pds.pgs.phone.CSProfileDetailHome;
import com.pingtel.pds.pgs.phone.Device;
import com.pingtel.pds.profilewriter.ProfileWriter;
import org.sipfoundry.sipxconfig.core.Phone;
import org.sipfoundry.sipxconfig.core.LogicalPhone;
import org.sipfoundry.sipxconfig.core.PhoneFactory;
import org.sipfoundry.sipxconfig.core.SipxConfig;
import org.sipfoundry.sipxconfig.core.CoreDao;
import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.beans.factory.access.SingletonBeanFactoryLocator;

/**
 *  RenderProfileBean is the EJ Bean implementation of the RenderProile
 *  auxialliary session bean. It is used as part of all of the profile
 *  generation Use Cases.
 */
public class RenderProfileBean extends JDBCAwareEJB
        implements SessionBean, RenderProfileBusiness {

//////////////////////////////////////////////////////////////////////////
// Constants
////
    private final static Integer ONE = new Integer(1);

    private final static String METAPROPERTY_MACADDRESS = "MACADDRESS";
    private final static String METAPROPERTY_MANUFACTURER = "MANUFACTURER";
    private final static String METAPROPERTY_MODEL = "MODEL";
    private final static String METAPROPERTY_LEAFFILENAME = "LEAFFILENAME";


//////////////////////////////////////////////////////////////////////////
// Attributes
////
    /**
     *  The Session Context object
     */
    private SessionContext mCTX;

    /*
     *  Home references
     */
    private RefPropertyHome mRefPropertyHome;
    private CSProfileDetailHome mCoreSoftwareProfileDetailHome;
    private ConfigurationSetHome mConfigurationSetHome;

    // URL got from the PGS properties file to use for ProfileWriter service.
    private String mProfileWriterURL;

    private SAXBuilder m_saxBuilder = new SAXBuilder();

    // EJBObject caches
    private MasterDetailsMap mCoreSoftwareProfileDetailsCache = new MasterDetailsMap();


//////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     *  Standard EJB implementation
     */
    public void ejbCreate() { }

    /**
     *  Standard EJB implementation
     */
    public void ejbRemove() { }

    /**
     *  Standard EJB implementation
     */
    public void ejbActivate() { }

    /**
     *  Standard EJB implementation
     */
    public void ejbPassivate() { }

    /**
     *  Standard EJB implementation
     */
    public void setSessionContext(SessionContext ctx) {

        mCTX = ctx;

        try {
            Context initial = new InitialContext();
            mRefPropertyHome = (RefPropertyHome) initial.lookup("RefProperty");
            mCoreSoftwareProfileDetailHome = (CSProfileDetailHome)
                    initial.lookup("CSProfileDetail");

            mConfigurationSetHome = (ConfigurationSetHome)
                    initial.lookup("ConfigurationSet");

        } catch (NamingException ex) {
            logFatal( ex.toString(), ex );
            throw new EJBException(ex);
        }

        mProfileWriterURL = getPGSProperty( "profilewriter.rmi.url" );
    }

    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     */
    public void unsetSessionContext() {
        mCTX = null;
    }


    /**
     *  Sends a PreRenderedProfile object to the RMI service where it is
     *  persisted to disk and made available for download via the SDS
     *
     * @param transformedProfile actual bytestream content of the profile
     * to be written.
     * @param device Device that this profile belongs to
     * @param profileType type of profile this bytestream represents, see
     * PDSDefintions PROF_ values.
     * @exception PDSException is thrown for application errors.
     * @see com.pingtel.pds.common.PDSDefinitions
     */
    public void writeProfile( byte[] transformedProfile, Device device, int profileType )
        throws PDSException {

        Integer deviceID = null;

        try {
            deviceID = device.getID();

            // The ProfileWriter uses these properties to decide how to
            // persist the profile and notify the SDS. For ex the Cisco Phone
            // uses tftp while Pingtel uses http for its content delivery service
            Properties profileMetaProperties =
                getPhoneProfileProperties( device, profileType );

            try {
                logDebug ( "trying to open profilewriter at: " + mProfileWriterURL );

                ProfileWriter profileWriterConnection = (ProfileWriter)
                        RMIConnectionManager.getInstance().getConnection(mProfileWriterURL);

                logDebug ( "opened profilewriter" );

                BeanFactoryLocator bfl = SingletonBeanFactoryLocator.getInstance();
                BeanFactoryReference bf = bfl.useBeanFactory("org.sipfoundry.sipxconfig.core");
                SipxConfig sipx = (SipxConfig) bf.getFactory().getBean("sipxconfig");        
                PhoneFactory phones = sipx.getPhoneFactory();    
                Phone phone = phones.getPhoneByModel(device.getModel());

                CoreDao dao = sipx.getCoreDao();
                LogicalPhone logicalPhone = (LogicalPhone) dao.requireById(LogicalPhone.class,
                        deviceID.intValue());
                String notifyURL = phone.getProfileNotifyUrl(logicalPhone, profileType);
                int profileSequenceNumber = phone.getProfileSequenceNumber(logicalPhone,
                        profileType);

                // Render the Profile Object via the profile writer, the profile
                // writer will attempt to notify the SDS which in turn will
                // send the profile url to the phone
                profileWriterConnection.updateDeviceProfile (
                    profileType,
                    profileMetaProperties.getProperty(METAPROPERTY_MACADDRESS),
                    notifyURL,
                    profileMetaProperties.getProperty(METAPROPERTY_MANUFACTURER),
                    profileMetaProperties.getProperty(METAPROPERTY_MODEL),
                    profileMetaProperties.getProperty(METAPROPERTY_LEAFFILENAME),
                    transformedProfile,
                    profileSequenceNumber );

                logDebug ( "updated profile" );
            }
            catch (Exception ex) {
                mCTX.setRollbackOnly();
                RMIConnectionManager.getInstance().reestablishConnection(mProfileWriterURL);

                throw new PDSException(
                        collateErrorMessages("UC120",
                        "E8001",
                        null),
                        ex);
            }
        }
        catch (Exception ex) {

            throw new PDSException(
                    collateErrorMessages("UC120",
                    "E4034",
                    new Object[]{deviceID}),
                    ex);
        }
    }


    /**
     * deleteProfile sends a request to the ProfileWriter to remove a profile
     * of the given profile type for the given Device from it's persistent
     * store.
     *
     * @param device Device whose profile should be deleted.
     * @param profileType type of profile this bytestream represents, see
     * PDSDefintions PROF_ values.
     * @exception PDSException is thrown for application errors.
     * @see com.pingtel.pds.common.PDSDefinitions
     */
    public void deleteProfile( Device device, int profileType )
        throws PDSException {

        Integer deviceID = null;

        try {
            deviceID = device.getID();

            // The ProfileWriter uses these properties to decide how to
            // persist the profile and notify the SDS. For ex the Cisco Phone
            // uses tftp while Pingtel uses http for its content delivery service
            Properties profileMetaProperties =
                getPhoneProfileProperties( device, profileType );

            logDebug ( "got profileMetaProperties" );

            try {
                logDebug ( "trying to open profilewriter at: " + mProfileWriterURL );

                ProfileWriter pw = (ProfileWriter)
                        RMIConnectionManager.getInstance().getConnection(mProfileWriterURL);

                logDebug ( "opened profilewriter" );

                String notifyURL = null;

                if ( device.getModel().equals( PDSDefinitions.MODEL_HARDPHONE_CISCO_7940 ) ||
                        device.getModel().equals( PDSDefinitions.MODEL_HARDPHONE_CISCO_7960 ) ) {

                    notifyURL = getDeviceNotifyURL ( device );
                }
                // Render the Profile Object via the profile writer, the profile
                // writer will attempt to notify the SDS which in turn will
                // send the profile url to the phone
                pw.removeDeviceProfile( profileType,
                                        profileMetaProperties.getProperty(this.METAPROPERTY_MACADDRESS),
                                        notifyURL,
                                        profileMetaProperties.getProperty(this.METAPROPERTY_MANUFACTURER),
                                        profileMetaProperties.getProperty(this.METAPROPERTY_MODEL),
                                        profileMetaProperties.getProperty(this.METAPROPERTY_LEAFFILENAME) );

                logDebug ( "deleted profile" );
            }
            catch (Exception ex) {
                mCTX.setRollbackOnly();
                RMIConnectionManager.getInstance().reestablishConnection(mProfileWriterURL);

                throw new PDSException(
                    collateErrorMessages(   "UC120",
                                            "E8001",
                                            null),
                    ex);
            }
        }
        catch (Exception ex) {

            throw new PDSException(
                    collateErrorMessages(   "UC120",
                                            "E4034",
                                            new Object[]{deviceID}),
                    ex);
        }
    }

    /**
     * getDeviceNotifyURL returns the URL which is used in the NOTIFY/check-sync
     * message which is sent to the Cisco 79XX devices.
     *
     * @param device Device for whom you want the URL
     * @return URL to be used in the NOTIFY
     * @throws PDSException is thrown for non-system errors.
     */
    public String getDeviceNotifyURL ( Device device )
            throws PDSException {

        StringBuffer returnURL = new StringBuffer();
        returnURL.append ("sip:");

        try {
            if ( (device.getModel().equals( PDSDefinitions.MODEL_HARDPHONE_CISCO_7940 ) ||
                    device.getModel().equals( PDSDefinitions.MODEL_HARDPHONE_CISCO_7960 )) &&
                    device.getUserID() != null ) {

                Document doc = null;

                ConfigurationSet cs = null;
                Collection configSets =
                        mConfigurationSetHome.findByUserID( device.getUserID() );

                for ( Iterator i = configSets.iterator(); i.hasNext(); )
                    cs = (ConfigurationSet) i.next();

                doc = m_saxBuilder.build(
                                    new ByteArrayInputStream ( cs.getContent().getBytes() ) );

                Element profile = doc.getRootElement();

                RefProperty primaryLineRP = getCiscoLine1RP();
                String line1RPID = primaryLineRP.getID().toString();

                Collection userLines = profile.getChildren( );
                for ( Iterator lineI = userLines.iterator(); lineI.hasNext(); ) {
                    Element setting = (Element) lineI.next();
                    String rpID = setting.getAttributeValue( "ref_property_id" );

                    if ( rpID.equals( line1RPID )  ) {
                        Element container = setting.getChild( "container" );
                        Element url = container.getChild( "line1_name" );
                        returnURL.append(url.getText());
                        break;
                    } // if
                } // for all elements in config set
            }
            else {
                // assert
            }

            returnURL.append("@");
            returnURL.append(device.getOrganization().getDNSDomain());
        }
        catch (RemoteException e) {
            logFatal (e.getMessage(), e);
            throw new EJBException (e.getMessage());
        }
        catch (Exception e) {
            mCTX.setRollbackOnly();
            throw new PDSException (e.getMessage());
        }

        return returnURL.toString();
    }



//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    /**
     * Gets the logicalPhonesSeq attribute of the RenderProfileBean object
     *
     * @param logicalPhoneID        Logical Identifier for Phone
     * @param profileType           One of currently 4 profile types
     * @return                      the next sequence number that should
     *                              be sent down with the profile.
     * @exception PDSException      A wrapped SQLException most likely
     *                              caused by a DB error
     */
    private int getLogicalPhonesSeq(Integer logicalPhoneID, int profileType)
            throws PDSException {

        ArrayList results = null;

        try {
            // Select the results sequence number column from the DB
            results =
                    executePreparedQuery("SELECT SEQ_NUMBER FROM PROFILE_DATA WHERE LOG_PN_ID = ? AND TYPE = ?",
                            new Object[]{
                                logicalPhoneID,
                                new Integer(profileType)},
                            1,
                            1);
            if ( results.size() == 1 ) {
                executePreparedUpdate(
                        "UPDATE PROFILE_DATA SET SEQ_NUMBER = SEQ_NUMBER + 1 WHERE LOG_PN_ID = ? AND TYPE = ?",
                        new Object[]{ logicalPhoneID, new Integer(profileType)});

            } else if ( results.size() == 0 ) {
                // First insert returns a sequence number
                // 1 which we hard code here.
                createFirstProfileDataRecord(profileType, logicalPhoneID);
                return 1;
            } else {
                throw new PDSException(
                        collateErrorMessages("UC120",
                                "E1032",
                                new Object[]{logicalPhoneID}));
            }
        } catch (SQLException ex) {
            throw new PDSException(
                    collateErrorMessages("UC120",
                            "E1032",
                            new Object[]{logicalPhoneID}),
                    ex);
        }

        ArrayList row = (ArrayList) results.get(0);
        // @JC Added 1 to the result here as the select finds the old value

        // IB int returnValue = new Integer(((String) row.get(0))).intValue() + 1;
        return Integer.parseInt((String) row.get(0)) + 1;
        // IB return returnValue;
    }


    /**
     *  Description of the Method
     *
     *@param  profileType           Description of the Parameter
     *@param  logicalPhoneID        Description of the Parameter
     *@exception  PDSException  Description of the Exception
     */
    private void createFirstProfileDataRecord(int profileType,
            Integer logicalPhoneID)
             throws PDSException {

        try {
            executePreparedUpdate("INSERT INTO PROFILE_DATA ( TYPE, LOG_PN_ID, SEQ_NUMBER ) VALUES (?,?,? )",
                    new Object[]{new Integer(profileType), logicalPhoneID, ONE});

        } catch (SQLException ex) {
            throw new PDSException(
                    collateErrorMessages("UC120",
                    "E2015",
                    new Object[]{logicalPhoneID}),
                    ex);
        }
    }



    private Properties getPhoneProfileProperties( Device device, int profileType )
        throws PDSException, RemoteException {

        Properties profileMetaProperties = new Properties();
        String deviceDetails = device.getShortName();

        // serial numbers may be blank immediately following a copy operation.
        if (device.getSerialNumber() == null ) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages(   "UC120",
                                            "E3015",
                                            new Object[]{ deviceDetails } ) );
        }

        String model = device.getModel();

        profileMetaProperties.setProperty( METAPROPERTY_MACADDRESS, device.getSerialNumber());
        profileMetaProperties.setProperty( METAPROPERTY_MANUFACTURER, device.getManufaturerName());
        profileMetaProperties.setProperty( METAPROPERTY_MODEL, model );
        String leafFileName = null;
        leafFileName = getProfileFileName(device.getCoreSoftwareDetailsID(), new Integer(profileType));
        profileMetaProperties.setProperty( METAPROPERTY_LEAFFILENAME, leafFileName);

        return profileMetaProperties;
    }


    private RefProperty getCiscoLine1RP () throws PDSException {
        RefProperty rp = null;

        try {
            try {
                Collection c = mRefPropertyHome.findByCode( "cs_1020" );
                for ( Iterator i = c.iterator(); i.hasNext(); )
                    rp = (RefProperty) i.next();
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages(   "E1029",
                                                new Object[]{ "cs_1020" }),
                        ex);
            }
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );
            throw new EJBException ( ex.toString() );
        }

        return rp;
    }


    /**
     * getProfileFileName is responsible for fetching and caching the names of the
     * profile file names for a given version of CoreSoftware.   CoreSoftware
     * (and CoreSoftwareProfileDetails) beans are ummutable so it is safe to
     * cache these values indefinately.
     *
     * @param coreSoftwareId PK of the CoreSoftware you are interested in finding the
     * file name for .
     * @param profileType standard profile type.
     * @return file name for the given CoreSoftware and profile type.
     * @throws PDSException for application errors
     * @throws RemoteException for system errors
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_PHONE
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_USER
     * @see com.pingtel.pds.common.PDSDefinitions#PROF_TYPE_APPLICATION_REF
     */
    private String getProfileFileName (Integer coreSoftwareId, Integer profileType)
            throws PDSException, RemoteException {

        if (!mCoreSoftwareProfileDetailsCache.contains(coreSoftwareId, profileType)){
            CSProfileDetail cspd = null;
            Collection cspdCollection = null;

            try {
                cspdCollection =
                        mCoreSoftwareProfileDetailHome.findByCoreSoftwareAndProfileType(coreSoftwareId, profileType);
            } catch (FinderException ex) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages( "E1014", new Object[]{coreSoftwareId,  profileType}),
                        ex);
            }

            for (Iterator cspdI = cspdCollection.iterator(); cspdI.hasNext(); ) {
                cspd = (CSProfileDetail) cspdI.next();
            }

            mCoreSoftwareProfileDetailsCache.storeDetail(coreSoftwareId, profileType, cspd.getFileName());
        }

        return (String) mCoreSoftwareProfileDetailsCache.getDetail(coreSoftwareId, profileType);
    }



    //////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    ////


    //////////////////////////////////////////////////////////////////////////
    // Native Method Declarations
    ////

}
