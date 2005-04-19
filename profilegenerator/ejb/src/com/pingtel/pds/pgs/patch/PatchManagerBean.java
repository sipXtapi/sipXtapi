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

package com.pingtel.pds.pgs.patch;

import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.StringTokenizer;

import javax.ejb.EJBException;
import javax.ejb.SessionBean;
import javax.ejb.SessionContext;
import javax.transaction.SystemException;
import javax.transaction.UserTransaction;

import org.jdom.Document;
import org.jdom.Element;
import org.jdom.input.SAXBuilder;

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;

public class PatchManagerBean extends JDBCAwareEJB implements SessionBean, PatchManagerBusiness {

    private static final String PATCH_XML_NAME = "/patches.xml";

    private SAXBuilder m_sax = new SAXBuilder();

    // returns null if no upgrade needed
    public String upgradeNeeded() throws PDSException {


        try {
            String targetVersion = null;
            Element root = getUpgradeInfo();
            Integer version = getDBVersion();

            Collection versions = root.getChildren("version");
            for (Iterator iV = versions.iterator(); iV.hasNext();) {
                Element versionElement = (Element) iV.next();
                Integer versionNumber = Integer.valueOf(versionElement
                        .getAttributeValue("number"));

                if (versionNumber.intValue() >= version.intValue()) {
                    if (targetVersion != null) {
                        if (Integer.valueOf(targetVersion).intValue() < version.intValue())
                            targetVersion = versionNumber.toString();
                    } else {
                        targetVersion = versionNumber.toString();
                    }
                }
            }
            return targetVersion;
        } catch (Exception ex) {
            logError(ex.toString(), ex);
            throw new PDSException("exception in ProcessPatches", ex);
        }
    }

    public Element getUpgradeInfo() {

        try {
            Document document = m_sax.build(getClass().getResourceAsStream(PATCH_XML_NAME));

            Element root = document.getRootElement();
            return root;
        } catch (Exception ex) {
            logFatal(ex.toString(), ex);
            throw new EJBException("exception in ProcessPatches", ex);
        }
    }

    public void processPatches(Integer targetVersion) throws PDSException {
        try {
            Element root = getUpgradeInfo();
            Integer version = getDBVersion();
            logDebug("existing database version is: " + version);
            logDebug("target version is: " + targetVersion);

            Collection versions = root.getChildren("version");
            for (Iterator iV = versions.iterator(); iV.hasNext();) {
                Element versionElement = (Element) iV.next();
                Integer versionNumber = Integer.valueOf(versionElement
                        .getAttributeValue("number"));
                logDebug("looking at patches for version: " + versionNumber);

                String status = null;

                if (versionNumber.intValue() <= targetVersion.intValue()) {
                    boolean allPatchesApplied = true;
                    Collection patches = versionElement.getChildren();

                    for (Iterator iP = patches.iterator(); iP.hasNext();) {
                        Element patchElement = (Element) iP.next();

                        Integer patchNumber = new Integer(patchElement.getAttributeValue("number"));

                        if (versionNumber.intValue() <= version.intValue()) {
                            logDebug("no need to run patch: " + patchNumber);
                            setPatchStatus("A", patchNumber);
                        } else {
                            status = getPatchStatus(patchNumber);

                            if (status.equals("N")) {
                                String className = patchElement.getAttributeValue("class");
                                String args = patchElement.getAttributeValue("args");
                                logDebug("applying patch: " + patchNumber);

                                String runStatus = runPatch(className, args, patchNumber);

                                logDebug("got status of " + runStatus + " for patch# "
                                        + patchNumber);

                                if (runStatus.equals("F"))
                                    allPatchesApplied = false;

                            } else if (status.equals("F")) {
                                allPatchesApplied = false;
                                logDebug("patch: " + patchNumber + " failed");
                            } else if (status.equals("S") || status.equals("A")) {

                            }
                        }
                    } // for

                    logDebug("visited all patches for version " + versionNumber.intValue()
                            + " allPatchesApplied=" + allPatchesApplied);

                    if (allPatchesApplied && versionNumber.intValue() > version.intValue()) {

                        logDebug("all patches applied about to set db version ");
                        setDatabaseVersion(versionNumber);
                        logDebug("all patches applied set db version ");
                    }

                } // if
            } // for

            logTransaction(m_ctx, "Upgraded the database");
        } catch (Exception ex) {
            logError(ex.toString(), ex);
            throw new PDSException("exception in ProcessPatches", ex);
        }
    }

    private String runPatch(String className, String args, Integer patchNumber)
            throws PDSException {

        logDebug("about to run patch# " + patchNumber + " class " + className + " args " + args);

        Patch newPatch = null;
        String status = "F";
        UserTransaction tx = null;

        try {
            tx = m_ctx.getUserTransaction();
            tx.begin();

            HashMap argsMap = new HashMap();

            if (args != null) {
                StringTokenizer argsTok = new StringTokenizer(args, ",");
                while (argsTok.hasMoreTokens()) {
                    String token = argsTok.nextToken();
                    StringTokenizer nameValueTok = new StringTokenizer(token, "=");
                    String name = nameValueTok.nextToken();
                    String value = nameValueTok.nextToken();
                    argsMap.put(name, value);
                }
            }

            newPatch = (Patch) Class.forName(className).newInstance();
            newPatch.initialize(argsMap);
            status = newPatch.update();
            tx.commit();

            setPatchStatus(status, patchNumber);
        } catch (PDSException ex) {
            status = "A";
            logError(ex.toString(), ex);

            try {
                tx.rollback();
            } catch (SystemException exc) {
                throw new EJBException(exc.toString());
            }
            setPatchStatus(status, patchNumber);

        } catch (Exception ex) {
            logError(ex.toString(), ex);

            try {
                tx.rollback();
            } catch (SystemException exc) {
                throw new EJBException(exc.toString());
            }

            setPatchStatus(status, patchNumber);
        }

        return status;
    }

    private Integer getDBVersion() throws PDSException {

        Integer version = getDatabaseVersion();

        if (version.intValue() == 0) {
            runPatch("com.pingtel.pds.pgs.patch.DatabaseScriptProcessor",
                    "script=/version-tables.sql", Integer.valueOf("0"));

            setDatabaseVersion(new Integer(0));
        }

        return version;
    }

    public Integer getDatabaseVersion() {

        Integer version = null;

        try {
            ArrayList rows = executePreparedQuery("SELECT MAX(VERSION) " + "FROM VERSIONS ",
                    null, 1, 1);

            for (Iterator iRows = rows.iterator(); iRows.hasNext();) {
                ArrayList row = (ArrayList) iRows.next();
                version = Integer.valueOf((String) row.get(0));
            }

        } catch (SQLException ex) {
            logInfo(ex.toString());
            version = new Integer(0);
        }

        return version;
    }

    public void setDatabaseVersion(Integer version) throws PDSException {

        logDebug("in setDatabaseVersion " + version);
        UserTransaction tx = null;

        try {
            tx = m_ctx.getUserTransaction();
            tx.begin();

            executePreparedUpdate("INSERT INTO VERSIONS ( applied, version ) "
                    + "VALUES ( ?, ? ) ", new Object[] {
                new java.sql.Date(new java.util.Date().getTime()), version
            });

            logDebug("in setDatabaseVersion about to commit");
            tx.commit();

            logTransaction(m_ctx, "Updated database to version " + version);
        } catch (SQLException ex) {
            logError(ex.toString(), ex);

            try {
                tx.rollback();
            } catch (SystemException exc) {
                throw new EJBException(exc.toString());
            }

            throw new PDSException("problem with getVersion", ex);
        } catch (Exception ex) {
            try {
                tx.rollback();
            } catch (SystemException exc) {
                throw new EJBException(exc.toString());
            }

            logFatal(ex.toString(), ex);
            throw new EJBException("problem with getVersion: " + ex.toString());
        }
    }

    public String getPatchStatus(Integer patchNumber) throws PDSException {

        String status = "N";

        try {
            ArrayList rows = executePreparedQuery("SELECT status " + "FROM PATCHES "
                    + "WHERE patch_number = ?", new Object[] {
                patchNumber
            }, 1, 1);

            if (!rows.isEmpty()) {

                for (Iterator iRows = rows.iterator(); iRows.hasNext();) {
                    ArrayList row = (ArrayList) iRows.next();
                    String statusString = (String) row.get(0);
                    status = statusString;
                }
            }

        } catch (SQLException ex) {
            logError(ex.toString(), ex);
            throw new PDSException("problem with getVersion", ex);
        }

        return status;
    }

    public String getPatchDescription(Integer patchNumber) throws PDSException {
        String description = null;

        try {
            ArrayList rows = executePreparedQuery("SELECT description " + "FROM PATCHES "
                    + "WHERE patch_number = ?", new Object[] {
                patchNumber
            }, 1, 1);

            if (!rows.isEmpty()) {

                for (Iterator iRows = rows.iterator(); iRows.hasNext();) {
                    ArrayList row = (ArrayList) iRows.next();
                    description = (String) row.get(0);
                    ;
                }
            }

        } catch (SQLException ex) {
            logError(ex.toString(), ex);
            throw new PDSException("problem with getVersion", ex);
        }

        return description;
    }

    public void setPatchStatus(String status, Integer patchNumber) throws PDSException {

        if (patchNumber.intValue() == 0) {
            setPatchStatus(status, patchNumber, "Patch table creation.");
        } else {
            String patchDescription = null;

            Element root = getUpgradeInfo();
            Collection versions = root.getChildren("version");
            for (Iterator iV = versions.iterator(); iV.hasNext();) {
                Element version = (Element) iV.next();
                Collection patches = version.getChildren("patch");
                for (Iterator iP = patches.iterator(); iP.hasNext();) {
                    Element patch = (Element) iP.next();
                    Integer number = Integer.valueOf(patch.getAttributeValue("number"));
                    if (number.equals(patchNumber)) {
                        patchDescription = patch.getAttributeValue("description");
                    }
                }
            }

            if (patchDescription == null)
                throw new EJBException("No description found for patch# " + patchNumber);

            setPatchStatus(status, patchNumber, patchDescription);
        }
    }

    private void setPatchStatus(String status, Integer patchNumber, String description)
            throws PDSException {

        UserTransaction tx = null;

        try {
            tx = m_ctx.getUserTransaction();
            tx.begin();

            executePreparedUpdate("  DELETE FROM PATCHES " + "   WHERE patch_number = ? ",
                    new Object[] {
                        patchNumber
                    });

            executePreparedUpdate(
                    "  INSERT INTO PATCHES (    patch_number, date_applied, status, description )"
                            + "  VALUES ( ?, ?, ?, ? )  ", new Object[] {
                        patchNumber, new java.sql.Date(new java.util.Date().getTime()), status,
                        description
                    });
            tx.commit();
        } catch (SQLException ex) {
            logError("problem in setStatus: " + ex.toString(), ex);

            try {
                tx.rollback();
            } catch (Exception exc) {
                logError(exc.toString(), ex);
                throw new EJBException(exc.toString());
            }

            throw new PDSException("problem in setStatus", ex);
        } catch (Exception ex) {
            try {
                tx.rollback();
            } catch (Exception exc) {
                logError(exc.toString(), ex);
                throw new EJBException(exc.toString());
            }

            throw new EJBException("problem in setStatus: " + ex.toString());
        }
    }

    public void ejbCreate() {
    }

    public void ejbActivate() {
    }

    public void ejbPassivate() {
    }

    public void ejbRemove() {
    }

    public void setSessionContext(SessionContext ctx) {
        this.m_ctx = ctx;
    }

    public void unsetSessionContext() {
        this.m_ctx = null;
    }

    private SessionContext m_ctx;
}