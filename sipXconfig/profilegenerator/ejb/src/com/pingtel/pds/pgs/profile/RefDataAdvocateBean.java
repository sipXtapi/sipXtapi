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

/*
 *  error messages updated
 */
package com.pingtel.pds.pgs.profile;

import java.rmi.RemoteException;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

import javax.ejb.CreateException;
import javax.ejb.EJBException;
import javax.ejb.FinderException;
import javax.ejb.RemoveException;
import javax.ejb.SessionBean;
import javax.ejb.SessionContext;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;

import org.jdom.Element;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.PropertyGroupLink;
import com.pingtel.pds.pgs.common.MasterDetailsMap;
import com.pingtel.pds.pgs.common.PGSDefinitions;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.phone.DeviceType;
import com.pingtel.pds.pgs.phone.DeviceTypeHome;
import com.pingtel.pds.pgs.phone.Manufacturer;
import com.pingtel.pds.pgs.phone.ManufacturerHome;

/**
 * Description of the Class
 * 
 * @author ibutcher
 * @created December 13, 2001
 */
public class RefDataAdvocateBean extends JDBCAwareEJB implements SessionBean, PDSDefinitions,
        PGSDefinitions, RefDataAdvocateBusiness {

    // Bean Context
    private SessionContext m_ctx;

    // Home interface objects

    private RefPropertyHome m_rpHome;
    private RefConfigurationSetHome m_rcsHome;
    private DeviceTypeHome m_dtHome;
    private ManufacturerHome m_manuHome;

    // Misc.
    private final static String CREATE_REF_PROP_RCS_ASSOC_SQL = "CREATE_REF_PROP_RCS_ASSOC_SQL";

    private final static String DELETE_REF_PROP_RCS_ASSOC_SQL = "DELETE_REF_PROP_RCS_ASSOC_SQL";

    private String m_createRPRCSAssocSQL;
    private String m_deleteRPRCSAssocSQL;

    private HashMap m_refPropertyEJBObjectMap = new HashMap();
    private HashMap m_RCSEJBObjectMap = new HashMap();
    private MasterDetailsMap m_deviceTypeRPCache = new MasterDetailsMap();

    /**
     * Mask for mask parameter for assignRefPropertytoRCS
     */

    public final static Integer FINAL = new Integer(1);
    /**
     * Description of the Field
     */
    public final static Integer APPENDABLE = new Integer(2);
    /**
     * Description of the Field
     */
    public final static Integer OVERRIDABLE = new Integer(3);

    public RefProperty createRefProperty(String name, String profileType, String code,
            String content) throws PDSException, RemoteException {

        return createRefProperty(name, new Integer(profileType).intValue(), code, content);
    }

    /**
     * Description of the Method
     * 
     * @param name Description of the Parameter
     * @param profileType Description of the Parameter
     * @param content Description of the Parameter
     * @return Description of the Return Value
     * @exception PDSException Description of the Exception
     */
    public RefProperty createRefProperty(String name, int profileType, String code, String content)
            throws PDSException, RemoteException {

        RefProperty rp = null;

        try {
            Collection c = this.m_rpHome.findByCode(code);

            if (c.isEmpty()) {
                rp = m_rpHome.create(name, profileType, code, content);
            } else {
                for (Iterator i = c.iterator(); i.hasNext();) {
                    rp = (RefProperty) i.next();
                }
            }

            // New code to clear the Device type / ref property cache

            /**
             * @todo when we externalize the device type/ref property mapping xml doc we should
             *       put the flushing code in the use case which assigns ref properties to device
             *       types.
             */
            m_deviceTypeRPCache.clear();
        } catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(collateErrorMessages("UC530", "E2013", new Object[] {
                name, new Integer(profileType), content
            }), ex);
        } catch (CreateException ce) {
            m_ctx.setRollbackOnly();

            throw new PDSException(collateErrorMessages("UC530", "E2013", new Object[] {
                name, new Integer(profileType), content
            }), ce);
        }

        return rp;
    }

    /**
     * Description of the Method
     * 
     * @param name Description of the Parameter
     * @param organizationID Description of the Parameter
     * @return Description of the Return Value
     * @exception PDSException Description of the Exception
     */
    public RefConfigurationSet createRefConfigurationSet(String name, String organizationID)
            throws PDSException, RemoteException {

        RefConfigurationSet rcs = null;

        try {
            rcs = m_rcsHome.create(name, new Integer(organizationID));
        } catch (CreateException ce) {
            m_ctx.setRollbackOnly();

            throw new PDSException(collateErrorMessages("UC520", "E2014", new String[] {
                name, organizationID
            }), ce);
        }

        return rcs;
    }

    public void deleteRefConfigSet(String refConfigSetID) throws PDSException, RemoteException {

        try {
            RefConfigurationSet rcs = getRefConfigurationSet(new Integer(refConfigSetID));
            deleteRefConfigSet(rcs);
        } catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(collateErrorMessages("UC540", "E1021", new Object[] {
                refConfigSetID
            }), ex);
        }
    }

    /**
     * Description of the Method
     * 
     * @exception PDSException Description of the Exception
     */
    public void deleteRefConfigSet(RefConfigurationSet rcs) throws PDSException, RemoteException {

        try {
            executePreparedUpdate("DELETE FROM CS_PROPERTY_PERMISSIONS " + "WHERE RCS_ID = ? ",
                    new Object[] {
                        rcs.getID()
                    });

            rcs.remove();
        } catch (SQLException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(collateErrorMessages("UC540", "E6007", new Object[] {
                rcs.getExternalID()
            }), ex);
        } catch (RemoveException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(collateErrorMessages("UC540", "E6008", new Object[] {
                rcs.getExternalID()
            }), ex);
        }
    }

    public void assignRefPropertytoRCS(String refConfigSetID, RefConfigSetAssignment rule)
            throws PDSException, RemoteException {

        try {
            RefConfigurationSet rcs = getRefConfigurationSet(new Integer(refConfigSetID));
            assignRefPropertytoRCS(rcs, rule);
        } catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(collateErrorMessages("UC500", "E1021", new Object[] {
                refConfigSetID
            }), ex);
        }
    }

    /**
     * Description of the Method
     * 
     * @param rule Description of the Parameter
     * @exception PDSException Description of the Exception
     */
    public void assignRefPropertytoRCS(RefConfigurationSet rcs, RefConfigSetAssignment rule)
            throws PDSException, RemoteException {

        RefProperty rp = null;
        // RefConfigurationSet rcs = null;
        logDebug("assignRefPropertyToRCS RCS: " + rcs.getExternalID() + " rule: "
                + rule.getRefPropertyID());

        Integer refPropertyID = rule.getRefPropertyID();

        try {
            rp = m_rpHome.findByPrimaryKey(refPropertyID);
            // logDebug ( "found ref property: " + rp.getExternalID() );
        } catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(collateErrorMessages("UC500", "E1029", new Object[] {
                refPropertyID
            }), ex);
        }

        try {
            // print out complete statement.
            executePreparedUpdate(m_createRPRCSAssocSQL, new Object[] {
                rcs.getID(), rp.getID(), new Integer(rule.getIsReadOnly() ? 1 : 0),
                new Integer(rule.getIsFinal() ? 1 : 0)
            });

        } catch (SQLException se) {
            m_ctx.setRollbackOnly();
            logFatal(se.toString(), se);

            throw new PDSException(collateErrorMessages("UC500", "E5008", new Object[] {
                rp.getExternalID(), rcs.getExternalID()
            }), se);
        }
    }

    /**
     * @param refConfigSetID is the String representation of the PK of the Ref Config Set that you
     *        are trying to remove a Ref Property from.
     * @param refPropertyID is the String representation of the PK of the Ref Property that you
     *        are trying to remove from the Ref Config Set.
     * @exception PDSException is thrown if an application level error occurs.
     */
    public void unassignRefPropertyFromRCS(String refConfigSetID, String refPropertyID)
            throws PDSException, RemoteException {

        Integer rcsID = new Integer(refConfigSetID);
        Integer rpID = new Integer(refPropertyID);
        RefConfigurationSet rcs = null;
        RefProperty rp = null;

        try {
            rp = m_rpHome.findByPrimaryKey(rpID);
        } catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(collateErrorMessages("UC510", "E1029", new Object[] {
                refPropertyID
            }), ex);
        }

        try {
            // rcs = m_rcsHome.findByPrimaryKey(rcsID);
            rcs = getRefConfigurationSet(rcsID);
        } catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(collateErrorMessages("UC510", "E1021", new Object[] {
                refConfigSetID
            }), ex);
        }

        try {
            executePreparedUpdate(m_deleteRPRCSAssocSQL, new Object[] {
                rcs.getID(), rp.getID()
            });
        } catch (SQLException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(collateErrorMessages("UC510", "E5007", new Object[] {
                rp.getExternalID(), rcs.getExternalID()
            }), ex);

        }
    }

    /**
     * getRefConfigSetsProperties returns a Collection of ProjectionRules
     * 
     * @param refConfigSetID the String representation of the PK of the Ref Configuration Set that
     *        you want to find which Reference Properties meta data for.
     * @return a Collection of PDSPropertyMetaData objects.
     * @exception PDSException is thrown for application level exceptions.
     * @see com.pingtel.pds.pgs.profile.PDSPropertyMetaData
     */
    public Collection getRefConfigSetsProperties(String refConfigSetID) throws PDSException {

        try {
            Collection properties = new ArrayList();

            List results = executePreparedQuery("SELECT CS.REF_PROP_ID, "
                    + "       CS.IS_FINAL,  " + "       CS.IS_READ_ONLY  "
                    + "FROM   CS_PROPERTY_PERMISSIONS CS " + "WHERE  CS.RCS_ID = ? ",
                    new Object[] {
                        refConfigSetID
                    }, 3, 1000000);

            for (int i = 0; i < results.size(); ++i) {
                List row = (List) results.get(i);

                Integer refPropertyID = Integer.valueOf((String) row.get(0));
                boolean isFinal = PGSDefinitions.PR_FINAL.equals(row.get(1));
                boolean isReadOnly = PGSDefinitions.PR_READ_ONLY.equals(row.get(2));

                String cardinality = getCardiniality(refPropertyID);

                ProjectionRule pr = new ProjectionRule(refPropertyID, isFinal, isReadOnly,
                        cardinality);

                properties.add(pr);
            }

            return properties;
        } catch (SQLException ex) {
            this.m_ctx.setRollbackOnly();

            throw new PDSException(collateErrorMessages("UC180", "E1030", new Object[] {
                refConfigSetID
            }), ex);
        }
    }

    /**
     * Gets the refConfigSetsProperty attribute of the ReferenceDataAdvocateBean object
     * 
     * @param refConfigSetID Description of the Parameter
     * @param refPropertyID Description of the Parameter
     * @return The refConfigSetsProperty value
     * @exception PDSException Description of the Exception
     */
    public ProjectionRule getRefConfigSetsProperty(String refConfigSetID, String refPropertyID)
            throws PDSException, RemoteException {

        RefConfigurationSet rcs = null;

        try {
            Integer rcsID = new Integer(refConfigSetID);
            Integer rpID = new Integer(refPropertyID);
            rcs = getRefConfigurationSet(rcsID);
            List results = executePreparedQuery("SELECT CS.REF_PROP_ID, "
                    + "       CS.IS_FINAL,  " + "       CS.IS_READ_ONLY,  "
                    + "FROM   CS_PROPERTY_PERMISSIONS CS " + "WHERE  CS.RCS_ID = ? "
                    + "AND    CS.REF_PROP_ID = ?", new Object[] {
                rcsID, rpID
            }, 3, 1);
            ProjectionRule pr = null;
            for (int i = 0; i < results.size(); ++i) {
                List row = (List) results.get(i);

                boolean isFinal = false;
                boolean isReadOnly = false;
                Integer intRefPropertyID = new Integer((String) row.get(0));

                if (row.get(1).equals(PR_FINAL)) {
                    isFinal = true;
                }

                if (row.get(2).equals(PR_READ_ONLY)) {
                    isFinal = true;
                }

                String cardinality = getCardiniality(intRefPropertyID);

                pr = new ProjectionRule(intRefPropertyID, isFinal, isReadOnly, cardinality);

            }

            return pr;
        } catch (SQLException ex) {
            this.m_ctx.setRollbackOnly();

            throw new PDSException(collateErrorMessages("UC180", "E1030", new Object[] {
                rcs.getExternalID()
            }), ex);
        } catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(collateErrorMessages("UC510", "E1021", new Object[] {
                refConfigSetID
            }), ex);
        }

    }

    private String getCardiniality(Integer refPropertyID) {
        return RefPropertyCache.getInstance().getCardinality(refPropertyID);
    }

    public Collection getRefPropertiesForDeviceType(Integer deviceTypeID) throws PDSException,
            RemoteException {

        return getRefPropertiesForDeviceTypeAndProfile(deviceTypeID, null);
    }

    public Collection getRefPropertiesForDeviceTypeAndProfile(Integer deviceTypeID,
            Integer profileType) throws PDSException, RemoteException {

        if (!m_deviceTypeRPCache.contains(deviceTypeID, profileType)) {
            DeviceType devType = null;

            try {
                devType = m_dtHome.findByPrimaryKey(deviceTypeID);
            } catch (FinderException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(collateErrorMessages("E1033", new Object[] {
                    deviceTypeID
                }), ex);
            }

            logDebug("found device type: " + devType.getExternalID());

            Manufacturer manu = null;

            try {
                manu = m_manuHome.findByPrimaryKey(devType.getManufacturerID());
            } catch (FinderException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(collateErrorMessages("E1034", new Object[] {
                    devType.getManufacturerID()
                }), ex);
            }

            logDebug("found manufacturer: " + manu.getExternalID());

            String manufacturerName = manu.getName();
            String model = devType.getModel();

            ArrayList validRefPropIDs = new ArrayList();

            Element root = PropertyGroupLink.getInstance().getMappings();
            Collection manufacturers = root.getChildren("manufacturer");
            for (Iterator iMan = manufacturers.iterator(); iMan.hasNext();) {
                Element manufacturer = (Element) iMan.next();
                String name = manufacturer.getAttributeValue("name");

                if (!name.equalsIgnoreCase(manufacturerName)) {
                    continue;
                }

                Collection deviceTypes = manufacturer.getChildren("device_type");
                for (Iterator iDT = deviceTypes.iterator(); iDT.hasNext();) {
                    Element deviceType = (Element) iDT.next();
                    String dtName = deviceType.getAttributeValue("model");

                    if (!dtName.equalsIgnoreCase("common")
                            && !dtName.equalsIgnoreCase(model)) {
                        continue;
                    }

                    Collection rpgs = deviceType.getChildren("ref_property_group");
                    for (Iterator iRPGS = rpgs.iterator(); iRPGS.hasNext();) {
                        Element refPropertyGroup = (Element) iRPGS.next();

                        String userType = refPropertyGroup.getAttributeValue("usertype");

                        if (userType.equalsIgnoreCase("user")) {
                            continue;
                        }

                        Collection refProperties = refPropertyGroup.getChildren("ref_property");
                        for (Iterator iRP = refProperties.iterator(); iRP.hasNext();) {
                            Element refProperty = (Element) iRP.next();

                            String refPropertyCode = refProperty.getAttributeValue("code");

                            try {
                                Collection rpC = m_rpHome.findByCode(refPropertyCode);
                                // TODO: remove after cleaning device defs file
                                if (rpC.isEmpty()) {
                                    logWarning("Empty reference property collection for device: " + dtName
                                            + " code: " + refPropertyCode);
                                }
                                for (Iterator iCode = rpC.iterator(); iCode.hasNext();) {
                                    RefProperty rp = (RefProperty) iCode.next();

                                    if (profileType == null
                                            || rp.getProfileType() == profileType.intValue()) {
                                        validRefPropIDs.add(rp);
                                    }
                                    break;
                                }
                            } catch (FinderException ex) {
                                logError("ignoring exception", ex);
                            }
                        } // for ref properties in group
                    } // for ref property groups in dev type
                } // for device types in manu

                break;

            } // for manufacturers

            m_deviceTypeRPCache.storeDetail(deviceTypeID, profileType, validRefPropIDs);
        } // if

        return (Collection) m_deviceTypeRPCache.getDetail(deviceTypeID, profileType);
    }

    private RefConfigurationSet getRefConfigurationSet(Integer id) throws FinderException,
            RemoteException {

        RefConfigurationSet returnValue = null;

        if (!m_RCSEJBObjectMap.containsKey(id)) {
            returnValue = m_rcsHome.findByPrimaryKey(id);
            m_refPropertyEJBObjectMap.put(id, returnValue);
        } else {
            returnValue = (RefConfigurationSet) m_RCSEJBObjectMap.get(id);
        }

        return returnValue;
    }

    /**
     * Standard Boiler Plate Session Bean Method Implementation
     */
    public void ejbCreate() {
    }

    /**
     * Standard Boiler Plate Session Bean Method Implementation
     */
    public void ejbRemove() {
    }

    /**
     * Standard Boiler Plate Session Bean Method Implementation
     */
    public void ejbActivate() {
    }

    /**
     * Standard Boiler Plate Session Bean Method Implementation
     */
    public void ejbPassivate() {
    }

    /**
     * Standard Boiler Plate Session Bean Method Implementation
     * 
     * @param ctx The new sessionContext value
     */
    public void setSessionContext(SessionContext ctx) {
        m_ctx = ctx;

        try {
            Context initial = new InitialContext();
            m_createRPRCSAssocSQL = getEnvEntry(CREATE_REF_PROP_RCS_ASSOC_SQL);
            m_deleteRPRCSAssocSQL = getEnvEntry(DELETE_REF_PROP_RCS_ASSOC_SQL);
            m_rpHome = (RefPropertyHome) initial.lookup("RefProperty");
            m_rcsHome = (RefConfigurationSetHome) initial.lookup("RefConfigurationSet");
            m_dtHome = (DeviceTypeHome) initial.lookup("DeviceType");
            m_manuHome = (ManufacturerHome) initial.lookup("Manufacturer");

        } catch (NamingException ne) {
            logFatal(ne.toString(), ne);
            throw new EJBException(ne);
        }
    }

    /**
     * Description of the Method
     */
    public void unsetSessionContext() {
        m_ctx = null;
    }

}
