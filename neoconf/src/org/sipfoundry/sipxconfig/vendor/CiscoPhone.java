/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.vendor;

import org.sipfoundry.sipxconfig.core.LogicalPhone;
import org.sipfoundry.sipxconfig.core.Phone;

/**
 * Support for Cisco 7960, 7940, 7905 and 7912 SIP phones.
 */
public class CiscoPhone implements Phone {

    /** system-wide plugin id for device */
    public static final String MODEL_7960 = "cisco7960";

    /** system-wide plugin id for device */
    public static final String MODEL_7940 = "cisco7940";

    /** system-wide plugin id for device */
    public static final String MODEL_7905 = "cisco7905";

    /** system-wide plugin id for device */
    public static final String MODEL_7912 = "cisco7912";

    private String m_id;
    
    /**
     * XML filename that describes a particular model's definitions
     * 
     * @param model
     * @return filepath to xml file
     */
    public String getDefinitions() {
        return new StringBuffer().append('/').append(m_id).append("definitions.xml").toString();
    }

    public String getModelId() {
        return m_id;
    }

    public void setModelId(String id) {
        m_id = id;
    }

    public int getProfileCount() {
        return 1;
    }

    public String getProfileFileName(int profileIndexTemp, String macAddress) {
        // Goes into TFTP Root dir, no vendor prefix
        return macAddress + ".lnk";
    }
    
    public String getProfileSubscribeToken(int profileIndexTemp) {
        // not supported
        return null;
    }

    public String getProfileNotifyUrl(LogicalPhone logicalPhoneTemp, int profileIndexTemp) {
        throw new RuntimeException("getDeviceNotifyUrl not implemented yet");
        /*
        StringBuffer returnURL = new StringBuffer();
        returnURL.append ("sip:");

        try {
            if (device.getUserId() != 0) {

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
*/  
    }
    
    public int getProfileSequenceNumber(LogicalPhone logicalPhoneTemp, int profileIndexTemp) {
        return SEQUENCE_NUMBER_NOT_SUPPORTED;
    }
}
