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

package com.pingtel.pds.pgs.user;

import java.rmi.RemoteException;
import java.sql.SQLException;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import javax.ejb.CreateException;
import javax.ejb.EJBException;
import javax.ejb.FinderException;
import javax.ejb.SessionBean;
import javax.ejb.SessionContext;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.organization.Organization;
import com.pingtel.pds.pgs.organization.OrganizationHome;

/**
 * ExtensionPoolAdvocateBean is the EJ Bean implementation class for the ExtensionPoolAdvocate
 * session bean. It provides the functionality for all of the ExtensionPool features.
 * 
 * @author ibutcher
 * 
 */
public class ExtensionPoolAdvocateBean extends JDBCAwareEJB implements SessionBean,
        ExtensionPoolAdvocateBusiness {
    public SessionContext mCTX;

    // Home references
    private UserHome mUserHome;
    private OrganizationHome mOrganizationHome;

    public ExtensionPoolAdvocateBean() {
    }

    /**
     * Standard EJB implementation
     */
    public void ejbCreate() throws CreateException {
    }

    /**
     * Standard EJB implementation
     */
    public void ejbActivate() throws EJBException {
    }

    /**
     * Standard EJB implementation
     */
    public void ejbPassivate() throws EJBException {
    }

    /**
     * Standard EJB implementation
     */
    public void ejbRemove() throws EJBException {
    }

    /**
     * Standard EJB implementation
     */
    public void setSessionContext(SessionContext ctx) {
        try {
            Context initial = new InitialContext();
            mUserHome = (UserHome) initial.lookup("User");
            mOrganizationHome = (OrganizationHome) initial.lookup("Organization");
        } catch (NamingException ne) {
            logFatalAndRethrow("", ne);
        }
        mCTX = ctx;
    }

    /**
     * createExtensionPool creates a extension pool (a named set of extensions which can then be
     * selected from by users when they want to assign an extension to a user) for an
     * organization.
     * 
     * @param organizationID the PK of the organization that you are creating pool for.
     * @param name the name by which the pool will be known (must be unique within an
     *        organization).
     * @return the PK for the new pool.
     * @throws PDSException for validation errors and application errors
     * @throws RemoteException
     * 
     */
    public Integer createExtensionPool(String organizationID, String name) throws PDSException,
            RemoteException {

        Integer newID = null;
        String externalOrgID = null;

        if (name != null && name.length() > MAX_NAME_LEN) {
            mCTX.setRollbackOnly();

            throw new PDSException(collateErrorMessages("UC621", "E3028", new Object[] {
                "name", new Integer(MAX_NAME_LEN)
            }));
        }

        String errorMessage = collateErrorMessages("UC621", "E4079", new Object[] {
            name, externalOrgID
        });
        try {
            checkExtensionPoolNameUnique(name);

            Organization organization = mOrganizationHome.findByPrimaryKey(Integer
                    .valueOf(organizationID));

            externalOrgID = organization.getExternalID();

            newID = new Integer(getNextSequenceValue("EXTENSION_POOLS_SEQ"));

            executePreparedUpdate("INSERT INTO EXTENSION_POOLS ( NAME, ORG_ID, ID ) "
                    + "VALUES ( ?, ?, ? )", new Object[] {
                name, Integer.valueOf(organizationID), newID
            });
        } catch (FinderException e) {
            logFatalAndRethrow(errorMessage, e);
        } catch (SQLException e) {
            logFatalAndRethrow(errorMessage, e);
        } catch (NumberFormatException e) {
            logFatalAndRethrow(errorMessage, e);
        }

        return newID;
    }

    /**
     * deletes an existing Extension Pool. To delete an extension pool you must unassign any
     * extensions in that pool from any users which may have them. This is to remove the RI
     * constraint. The rest is simple.
     * 
     * @param extensionPoolID the (internal) PK of the pool to delete
     * @throws com.pingtel.pds.common.PDSException for application level errors.
     */
    public void deleteExtensionPool(String extensionPoolID) throws PDSException, RemoteException {
        String errorMessage = collateErrorMessages("UC622", "E2033", new Object[] {
            extensionPoolID
        });

        try {
            List rows = executePreparedQuery(
                    "SELECT EXTENSION_NUMBER FROM EXTENSIONS WHERE EXT_POOL_ID = ?",
                    new Object[] {
                        Integer.valueOf(extensionPoolID)
                    }, 1, 1000000);

            for (Iterator iRows = rows.iterator(); iRows.hasNext();) {
                List row = (List) iRows.next();

                String extension = (String) row.get(0);

                Collection assignedUser = mUserHome.findByExtension(extension);

                for (Iterator iUser = assignedUser.iterator(); iUser.hasNext();) {
                    User user = (User) iUser.next();
                    user.setExtension(null);
                }
            }

            executePreparedUpdate("DELETE FROM EXTENSIONS  " + "WHERE EXT_POOL_ID = ? ",
                    new Object[] {
                        Integer.valueOf(extensionPoolID)
                    });

            executePreparedUpdate("DELETE FROM EXTENSION_POOLS  " + "WHERE ID = ? ",
                    new Object[] {
                        Integer.valueOf(extensionPoolID)
                    });
        } catch (SQLException e) {
            logFatalAndRethrow(errorMessage, e);
        } catch (FinderException e) {
            logFatalAndRethrow(errorMessage, e);
        }
    }

    /**
     * editExtensionPool allows administrators to change attributes for a given extension pool
     * (currently just the name).
     * 
     * @param extensionPoolID the PK of the extension pool that you want to modify
     * @param name the name that you want to change the pool to.
     * @throws com.pingtel.pds.common.PDSException is thrown for application level errors.
     */
    public void editExtensionPool(String extensionPoolID, String name) throws PDSException {
        if (name == null) {
            return;
        }
        String errorMsg = null;
        if (name.trim().length() == 0) {
            errorMsg = collateErrorMessages("E3030", new Object[] {
                "name"
            });
        }
        if (name.length() > MAX_NAME_LEN) {
            errorMsg = collateErrorMessages("E3028", new Object[] {
                "name", new Integer(MAX_NAME_LEN)
            });
        }
        if (errorMsg != null) {
            mCTX.setRollbackOnly();
            throw new PDSException(errorMsg);
        }
        try {
            checkExtensionPoolNameUnique(name);

            executePreparedUpdate("UPDATE EXTENSION_POOLS " + "SET NAME = ? " + "WHERE ID = ?",
                    new Object[] {
                        name, Integer.valueOf(extensionPoolID)
                    });
        } catch (SQLException e) {
            logFatalAndRethrow("", e);
        }
    }

    /**
     * getNextExtension returns the next available extension number for a given pool.
     * 
     * @param extensionPoolID the extension pool ID for the pool that you want to allocate the
     *        extension from.
     * @return next available extension.
     */
    public String getNextExtension(String extensionPoolID) {

        try {
            // ///////////////////////////////////////////////////////////////
            //
            // The following SQL statement looks pretty bizaar. It basically
            // finds the next available extension for the given pool. The
            // whole TO_NUMBER bit is to convert the varchar extension_number
            // values into numbers. Postgres requires that you give a
            // format mask for this function (Oracle does not). FYI to_number
            // is an Oracle function that Postgres copied not ANSI.
            //
            // ///////////////////////////////////////////////////////////////
            List rows = executePreparedQuery(
                    "SELECT MIN( TO_NUMBER( EXTENSION_NUMBER, '999999999999999999999999999999' ) ) "
                            + "FROM EXTENSIONS, EXTENSION_POOLS "
                            + "WHERE EXTENSION_POOLS.ID = ? "
                            + "AND EXTENSION_POOLS.ID = EXTENSIONS.EXT_POOL_ID "
                            + "AND EXTENSIONS.STATUS='A'", new Object[] {
                        Integer.valueOf(extensionPoolID)
                    }, 1, 1);
            if (!rows.isEmpty()) {
                List row = (List) rows.get(rows.size() - 1);
                String nextExtension = (String) row.get(0);
                return nextExtension;
            }
        } catch (SQLException e) {
            String errorMsg = collateErrorMessages("E5020", new Object[] {
                extensionPoolID
            });
            logFatalAndRethrow(errorMsg, e);
        }
        return null;
    }

    /**
     * allocateExtensionsToPool adds a contiguous range or integer extensions to a given extension
     * pool.
     * 
     * @param extensionPoolID the ID/PK of the extension pool that the extensions should be added
     *        to.
     * @param minExtension the lower end value for the extensions.
     * @param maxExtension the upper end value for the extensions.
     * @throws com.pingtel.pds.common.PDSException application errors.
     */
    public void allocateExtensionsToPool(String extensionPoolID, String minExtension,
            String maxExtension) throws PDSException, RemoteException {

        int min = parseInt(minExtension, "UC623", "E3021");
        int max = parseInt(maxExtension, "UC623", "E3021");

        String errorMsg = collateErrorMessages("UC623", "E5021", new Object[] {
            extensionPoolID
        });
        try {
            for (int counter = min; counter <= max; ++counter) {

                // This checks to see that the extension isn't owned by an existing
                // extension pool.
                List rows = executePreparedQuery("SELECT EXTENSION_NUMBER " + "FROM EXTENSIONS "
                        + "WHERE EXTENSION_NUMBER = ? ", new Object[] {
                    String.valueOf(counter)
                }, 1, 1);

                if (rows.isEmpty()) {

                    // Now check to see if a user has already has the extension -
                    // this can happen when a user enters an extension which is
                    // not currently in any pool.

                    Collection existingUsers = mUserHome.findByDisplayIDOrExtension(String
                            .valueOf(counter));

                    if (existingUsers.isEmpty()) {
                        executePreparedUpdate(
                                "INSERT INTO EXTENSIONS ( EXTENSION_NUMBER, EXT_POOL_ID,STATUS ) "
                                        + "VALUES ( ?, ?, 'A' ) ", new Object[] {
                                    new Integer(counter), Integer.valueOf(extensionPoolID)

                                });
                    } else {
                        executePreparedUpdate(
                                "INSERT INTO EXTENSIONS ( EXTENSION_NUMBER, EXT_POOL_ID,STATUS ) "
                                        + "VALUES ( ?, ?, 'U' ) ", new Object[] {
                                    new Integer(counter), Integer.valueOf(extensionPoolID)
                                });
                    }
                }
            }
        } catch (SQLException e) {
            logFatalAndRethrow(errorMsg, e);
        } catch (FinderException e) {
            logFatalAndRethrow(errorMsg, e);
        }
    }

    /**
     * Integer.parseInt that Translates NumberException to application (PDS) exception
     * 
     * @param number to parse
     * @param ucCode - strange code to help identify the error
     * @param eCode - another strange code to help identigy the error
     * @return parsed integer
     * @throws PDSException translated NumberException
     */
    private int parseInt(String number, String ucCode, String eCode) throws PDSException {
        try {
            return Integer.parseInt(number);
        } catch (NumberFormatException e) {
            mCTX.setRollbackOnly();
            String errorMsg = collateErrorMessages(ucCode, eCode, new Object[] {
                number
            });
            throw new PDSException(errorMsg, e);
        }

    }

    /**
     * deleteExtensionRange deletes the given extensions from the system. This can only be done if
     * the extensions are not currently assigned to a user if one is then a PDSException is
     * thrown. Extensions within the range are only deleted if they belong to the extenions pool
     * whose ID is given.
     * 
     * @param extensionPoolID is the extension pool's PK that you want to delete the extensions
     *        from.
     * @param minExtension the min extension that you want to delete.
     * @param maxExtension the max extension that you want to delete.
     * @throws com.pingtel.pds.common.PDSException for application errors.
     */
    public void deleteExtensionRange(String extensionPoolID, String minExtension,
            String maxExtension) throws PDSException, RemoteException {

        int intMinExtension = parseInt(minExtension, "UC624", "E5020");
        int intMaxExtension = parseInt(maxExtension, "UC624", "E5020");

        try {
            for (int count = intMinExtension; count <= intMaxExtension; ++count) {

                Collection allocatedUsers = mUserHome.findByExtension(String.valueOf(count));

                for (Iterator iUser = allocatedUsers.iterator(); iUser.hasNext();) {
                    User user = (User) iUser.next();
                    user.setExtension(null);
                }
            }

            executePreparedUpdate(
                    "DELETE FROM EXTENSIONS "
                            + "WHERE TO_NUMBER ( EXTENSION_NUMBER, '999999999999999999999999999999' ) >= TO_NUMBER ( ?, '999999999999999999999999999999' ) "
                            + "AND TO_NUMBER ( EXTENSION_NUMBER, '999999999999999999999999999999' ) <= TO_NUMBER ( ?, '999999999999999999999999999999' ) "
                            + "AND EXT_POOL_ID = ?", new Object[] {
                        minExtension, maxExtension, Integer.valueOf(extensionPoolID)
                    });
        } catch (FinderException e) {
            String errorMsg = collateErrorMessages("UC624", "E4081", new Object[] {
                minExtension
            });
            logFatalAndRethrow(errorMsg, e);
        } catch (SQLException e) {
            String errorMsg = collateErrorMessages("UC624", "E6024", new Object[] {
                minExtension
            });
            logFatalAndRethrow(errorMsg, e);
        }

    }

    /**
     * getAllExtensionPoolsDetails returns a Map of ID to Name pairs for all of the extension
     * pools in the system.
     * 
     * @return Map of ID to Name pairs
     */
    public Map getAllExtensionPoolsDetails() {

        HashMap returnValues = new HashMap();

        try {
            List rows = executePreparedQuery("SELECT ID, NAME " + "FROM EXTENSION_POOLS ", null,
                    2, 1000000);

            for (Iterator iRows = rows.iterator(); iRows.hasNext();) {
                List row = (List) iRows.next();

                Integer id = (Integer) row.get(0);
                String name = (String) row.get(1);
                returnValues.put(id, name);
            }
        } catch (SQLException e) {
            String errorMsg = collateErrorMessages("E8007", null);
            logFatalAndRethrow(errorMsg, e);
        }

        return returnValues;
    }

    private void checkExtensionPoolNameUnique(String name) throws SQLException, PDSException {
        // check for uniqueness of the extension pool name
        List rows = executePreparedQuery("SELECT NAME FROM EXTENSION_POOLS WHERE NAME =?",
                new Object[] {
                    name
                }, 1, 1);

        if (!rows.isEmpty()) {
            mCTX.setRollbackOnly();

            throw new PDSException(collateErrorMessages("E3031", new Object[] {
                name
            }));
        }
    }
}
