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

import java.io.IOException;
import java.sql.Date;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.StringTokenizer;

import javax.ejb.EJBException;
import javax.ejb.SessionBean;
import javax.ejb.SessionContext;
import javax.transaction.HeuristicMixedException;
import javax.transaction.HeuristicRollbackException;
import javax.transaction.NotSupportedException;
import javax.transaction.RollbackException;
import javax.transaction.SystemException;
import javax.transaction.UserTransaction;

import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;

public class PatchManagerBean extends JDBCAwareEJB implements SessionBean, PatchManagerBusiness {

    private static final String PATCH_XML_NAME = "/patches.xml";

    private SAXBuilder m_sax = new SAXBuilder();

    public Element getUpgradeInfo() {
        try {
            Document document = m_sax.build(getClass().getResourceAsStream(PATCH_XML_NAME));
            return document.getRootElement();
        } catch (JDOMException e) {
            logFatalAndRethrow("", e);
        } catch (IOException e) {
            logFatalAndRethrow("", e);
        }
        return null;
    }

    public void processPatches(Integer targetVersion) {
        Integer version = getDatabaseVersion();
        logDebug("existing database version is: " + version);
        logDebug("target version is: " + targetVersion);        
        processPatches(version.intValue(), targetVersion.intValue());
    }

    
    /**
     * Process all the patches that have versions bigger than current (non-inclusive)
     * but smaller or equal the target version (inclusive)
     * @param currentVersion
     * @param targetVersion
     */
    private void processPatches(int currentVersion, int targetVersion) {
        Element root = getUpgradeInfo();
        Collection versions = root.getChildren("version");
        for (Iterator iV = versions.iterator(); iV.hasNext();) {
            Element versionElement = (Element) iV.next();
            Integer versionNumber = Integer.valueOf(versionElement.getAttributeValue("number"));

            if (versionNumber.intValue() > targetVersion) {
                // ignore all patches for versions higher than our target version
                continue;
            }
            
            boolean allPatchesApplied = true;
            Collection patches = versionElement.getChildren();

            for (Iterator iP = patches.iterator(); iP.hasNext();) {
                Element patchElement = (Element) iP.next();

                Integer patchNumber = Integer.valueOf(patchElement.getAttributeValue("number"));

                // ignore patches for earlier versions and current version
                if (versionNumber.intValue() <= currentVersion) {
                    logDebug("no need to run patch: " + patchNumber);
                    setPatchStatus(Patch.ALREADY_APPLIED, patchNumber);
                } else {
                    String status = getPatchStatus(patchNumber);

                    if (status.equals(Patch.NOT_YET_APPLIED)) {
                        String className = patchElement.getAttributeValue("class");
                        String args = patchElement.getAttributeValue("args");

                        status = runPatch(className, args, patchNumber);

                        logDebug("got status of " + status + " for patch# " + patchNumber);

                    } 
                    if (status.equals(Patch.FAILURE)) {
                        allPatchesApplied = false;
                        logDebug("patch: " + patchNumber + " failed");
                    }
                }
            }

            logDebug("visited all patches for version " + versionNumber.intValue()
                    + " allPatchesApplied=" + allPatchesApplied);

            if (allPatchesApplied && versionNumber.intValue() > currentVersion) {
                setDatabaseVersion(versionNumber);
                logDebug("all patches applied set db version ");
            }
        }

        logTransaction(m_ctx, "Upgraded the database");        
    }
    
    private String runPatch(String className, String args, Integer patchNumber) {
        final String error = "Problem with running patch: " + patchNumber;

        try {
            UserTransaction tx = m_ctx.getUserTransaction();
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

            Patch newPatch = (Patch) Class.forName(className).newInstance();
            newPatch.initialize(argsMap);
            String status = newPatch.update(patchNumber);
            tx.commit();
                        
            setPatchStatus(status, patchNumber);
            return status;
        } catch (NotSupportedException e) {
            logFatalAndRethrow(error, e);
        } catch (SystemException e) {
            logFatalAndRethrow(error, e);
        } catch (SecurityException e) {
            logFatalAndRethrow(error, e);
        } catch (IllegalStateException e) {
            logFatalAndRethrow(error, e);
        } catch (RollbackException e) {
            logFatalAndRethrow(error, e);
        } catch (HeuristicMixedException e) {
            logFatalAndRethrow(error, e);
        } catch (HeuristicRollbackException e) {
            logFatalAndRethrow(error, e);
        } catch (InstantiationException e) {
            logFatalAndRethrow(error, e);
        } catch (IllegalAccessException e) {
            logFatalAndRethrow(error, e);
        } catch (ClassNotFoundException e) {
            logFatalAndRethrow(error, e);
        }
        return Patch.FAILURE;
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

    public void setDatabaseVersion(Integer version) {
        String error = "Unable to set DB version: " + version;

        UserTransaction tx = null;

        try {
            tx = m_ctx.getUserTransaction();
            tx.begin();

            Date currentTime = new Date(System.currentTimeMillis());
            executePreparedUpdate("INSERT INTO VERSIONS ( applied, version ) "
                    + "VALUES ( ?, ? ) ", new Object[] {
                currentTime, version
            });

            tx.commit();

            logTransaction(m_ctx, "Updated database to version " + version);
        } catch (NotSupportedException e) {
            logFatalAndRethrow(error, e);
        } catch (SystemException e) {
            logFatalAndRethrow(error, e);
        } catch (SecurityException e) {
            logFatalAndRethrow(error, e);
        } catch (IllegalStateException e) {
            logFatalAndRethrow(error, e);
        } catch (RollbackException e) {
            logFatalAndRethrow(error, e);
        } catch (HeuristicMixedException e) {
            logFatalAndRethrow(error, e);
        } catch (HeuristicRollbackException e) {
            logFatalAndRethrow(error, e);
        } catch (SQLException e) {
            logFatalAndRethrow(error, e);
        }
    }

    public String getPatchStatus(Integer patchNumber) {

        String status = Patch.NOT_YET_APPLIED;

        try {
            List rows = executePreparedQuery("SELECT status " + "FROM PATCHES "
                    + "WHERE patch_number = ?", new Object[] {
                patchNumber
            }, 1, 1);

            if (!rows.isEmpty()) {
                List row = (List) rows.get(0);
                status = (String) row.get(0);
            }

        } catch (SQLException ex) {
            logFatalAndRethrow(ex.toString(), ex);
        }

        return status;
    }

    public String getPatchDescription(Integer patchNumber) {
        String description = null;

        try {
            ArrayList rows = executePreparedQuery("SELECT description " + "FROM PATCHES "
                    + "WHERE patch_number = ?", new Object[] {
                patchNumber
            }, 1, 1);

            if (!rows.isEmpty()) {
                List row = (List) rows.get(0);
                description = (String) row.get(0);
            }
        } catch (SQLException ex) {
            logFatalAndRethrow(ex.toString(), ex);
        }
        return description;
    }

    public void initialUpgrade() {
        Element patchRoot = getUpgradeInfo();
        Integer currentVersion = Integer.valueOf( patchRoot.getAttributeValue("currentVersion"));
        setDatabaseVersion(currentVersion);
        processPatches(currentVersion.intValue(), Integer.MAX_VALUE);
    }
    
    
    public void setPatchStatus(String status, Integer patchNumber) {

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

    /**
     * Only throws runtime exception to make sure transaction is rolled back
     */
    private void setPatchStatus(String status, Integer patchNumber, String description) {

        final String error = "Unable to set patch status:" + patchNumber;

        try {
            UserTransaction tx = m_ctx.getUserTransaction();
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
        } catch (SQLException e) {
            logFatalAndRethrow(error, e);
        } catch (NotSupportedException e) {
            logFatalAndRethrow(error, e);
        } catch (SystemException e) {
            logFatalAndRethrow(error, e);
        } catch (SecurityException e) {
            logFatalAndRethrow(error, e);
        } catch (IllegalStateException e) {
            logFatalAndRethrow(error, e);
        } catch (RollbackException e) {
            logFatalAndRethrow(error, e);
        } catch (HeuristicMixedException e) {
            logFatalAndRethrow(error, e);
        } catch (HeuristicRollbackException e) {
            logFatalAndRethrow(error, e);
        }
    }

    public void ejbCreate() {
        // empty
    }

    public void ejbActivate() {
        // empty
    }

    public void ejbPassivate() {
        // empty
    }

    public void ejbRemove() {
        // empty
    }

    public void setSessionContext(SessionContext ctx) {
        this.m_ctx = ctx;
    }

    public void unsetSessionContext() {
        this.m_ctx = null;
    }

    private SessionContext m_ctx;

}
