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


package com.pingtel.pds.jbossauth;

import org.apache.xml.utils.WrappedRuntimeException;
import org.jboss.security.SimpleGroup;
import org.jboss.security.SimplePrincipal;
import org.jboss.security.auth.spi.AbstractServerLoginModule;

import com.pingtel.pds.common.MD5Encoder;
import com.pingtel.pds.common.PathLocatorUtil;

import javax.naming.InitialContext;
import javax.naming.NamingException;
import javax.security.auth.Subject;
import javax.security.auth.callback.*;
import javax.security.auth.login.FailedLoginException;
import javax.security.auth.login.LoginException;
import javax.sql.DataSource;

import java.io.FileInputStream;
import java.io.IOException;
import java.security.Principal;
import java.security.acl.Group;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLSession;

/**
 * SIPxchangeLoginModule is a Pingtel modified version of a JBoss
 * authentication module.   Most of the generic JAAS type code is theirs,
 * I added the database handling stuff and our password digesting scheme.
 * This code is specific to JBoss 2.X so I have not spent a large amount
 * of time polishing it as we hope to move up to a more recent revision.
 *
 * @author ibutcher.
 */
public class SIPxchangeLoginModule extends AbstractServerLoginModule  {

//////////////////////////////////////////////////////////////////////////
// Constants
////
    private static final String PRINCIPALS_QUERY =
        "   SELECT password, display_id " +
        "   FROM users " +
        "   WHERE display_id = ? " +
        "   OR extension = ?";

    private static final String ROLES_QUERY =
        "   SELECT  roles_name, " +
        "           NULL " +
        "   FROM    users, " +
        "           user_roles " +
        "   WHERE   users.id = user_roles.usrs_id " +
        "   AND     users.display_id=? ";

    private static final String DNS_DOMAIN_COUNT_QUERY =
        "SELECT COUNT(*) FROM ORGANIZATIONS";

    private static final String DNS_DOMAIN_QUERY =
        "SELECT DNS_DOMAIN FROM ORGANIZATIONS WHERE ID = 1";


    private static Properties PGS_PROPERTIES = loadPgsProperties();
    
    private static Properties loadPgsProperties()
    {
        try {
            Properties props = new Properties();
            String configFolder =
                PathLocatorUtil.getInstance().getPath(
                        PathLocatorUtil.CONFIG_FOLDER,
                        PathLocatorUtil.PGS );

            props.load( new FileInputStream ( configFolder + PathLocatorUtil.PGS_PROPS )  );
        	return props;
        }
        catch ( IOException ex ) {
            throw new WrappedRuntimeException ( ex );
        }
    }
    
//////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** The login identity */
    private Principal mIdentity;

    private String mDataSourceJndiName;


//////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
    * Initialize this LoginModule.
    */
    public void initialize( Subject subject,
                            CallbackHandler callbackHandler,
                            Map sharedState,
                            Map options ) {

        super.initialize(subject, callbackHandler, sharedState, options);

        mDataSourceJndiName = (String) options.get("dsJndiName");

        if( mDataSourceJndiName == null )
            mDataSourceJndiName = "java:/DefaultDS";

        relaxHttps();
    }

    /**
     * WARNING: Relaxing server names is ok only because https
     * connection HAS to be localhost and localhost probably doens't
     * match certificate's server name. When servers are on different
     * machines as exhibited in topology.xml.in file, then this
     * shouldn't be relaxed.
     */
    public void relaxHttps()
    {
        // This is the normal way to override and works in a testbed
        HostnameVerifier hv = new HostnameVerifier() 
        {
            public boolean verify(String urlHostName, SSLSession session) 
            {
                log.info("Warning: URL Host: " + urlHostName + " vs. " + session.getPeerHost());
                return true;
            }
        };

        // This seems to be the bit of code that actually works.  My suspition is that
        // Jboss https was compiled against and old JVM and somehow causes JVM to
        // use this verifier
        com.sun.net.ssl.HostnameVerifier oldhv = new com.sun.net.ssl.HostnameVerifier()
        {
            public boolean verify(String hostName, SSLSession session)
            {
                log.info("Warning: URL Host: " + hostName + " vs. " + session.getPeerHost());
                return true;
            }

            public boolean verify(String urlHostname, String certHostname)
            {
                log.info("Warning: URL Host: " + urlHostname + " vs. " + certHostname);
                return true;
            }
        };

        HttpsURLConnection.setDefaultHostnameVerifier(hv);
        com.sun.net.ssl.internal.www.protocol.https.HttpsURLConnectionOldImpl.setDefaultHostnameVerifier(oldhv);
    }

    /**
     * Looks for javax.security.auth.login.name and
     * javax.security.auth.login.password values in the sharedState map if
     * the useFirstPass option was true and returns true if they exist.
     * If they do not or are null this method returns false.
     *
     * @return
     * @throws LoginException
     */
    public boolean login() throws LoginException {

        if( callbackHandler == null ){
             throw new LoginException(  "Error: no CallbackHandler available " +
                                        "to collect authentication information");
        }

        NameCallback nc = new NameCallback("User name: ", "guest");
        PasswordCallback pc = new PasswordCallback("Password: ", false);
        Callback[] callbacks = {nc, pc};
        String username = null;
        String password = null;

        try  {
             callbackHandler.handle(callbacks);
             username = nc.getName();
             char[] tmpPassword = pc.getPassword();
             if(tmpPassword != null){
                char[] credential = new char[tmpPassword.length];
                System.arraycopy(   tmpPassword,
                                    0,
                                    credential,
                                    0,
                                    tmpPassword.length);

                pc.clearPassword();
                password = new String(credential);
             }
        }
        catch(java.io.IOException ioe) {
            throw new LoginException(ioe.toString());
        }
        catch(UnsupportedCallbackException uce) {
            throw new LoginException("CallbackHandler does not support: " +
                                        uce.getCallback());
        }

        username = normalizeUserName( username );

        this.mIdentity = new SimplePrincipal ( username );
        
        String digestedPassword = null;
        
        String [] arr;
        arr = getUserPasswordFromDB(username);
        
        String realUserID = arr [0];
        String expectedPassword = arr [1];
        
        if ( !username.equals( "installer" ) ) {
            // a new feature that we added for IBM is the ability to use
            // an external authentication source.   This code will create
            // a plug-in (specified in the JBoss startup script) to do
            // the authentication.
            // TODO: this is ugly - external plugin should treated in the same way as "internal authentication pugin"
            String externalAuthPlugin = System.getProperty("pds.external.auth","");
        
            if( externalAuthPlugin.length() != 0 ) {
                SIPxchangeAuthPlugin plugIn =
                        instantiatePlugIn(externalAuthPlugin);
        
                if(plugIn.isValidUser(username, password)){
        
                    // FIXME: this looks like a potential problem
                    // it does not use realm so digests are never going to be the same
                    // and as a result - we always update the password
                    // looks like the intention was to update it only when external plugin authenticates the users 
                    // while our internal passwords do not match
                    String enteredPEK =
                            MD5Encoder.encode(username + ":" + password);
        
                    String existingPEK =
                            getExistingProfileEncryptionKey(username);
        
                    if(!enteredPEK.equals(existingPEK)){
                        // this means that the password has changed in the
                        //external security source.
                        updateProfileEncryptionKey(username, enteredPEK);
                        updatePassword(realUserID, getRealm(), password);
        
                        // refresh the expected password value now we have
                        // updated it.
                        arr = getUserPasswordFromDB(username);
                        realUserID = arr [0];
                        expectedPassword = arr [1];
                    }
        
                } else {
                    throw new LoginException("Username/password does not exist in" +
                            " plugin security source.");
                }
            }
        
        
            // try both with and without DNS domain
            String realm = getRealm().trim();
            String dnsDomain = getDnsDomain().trim();
            digestedPassword = MD5Encoder.digestPassword(realUserID, dnsDomain, realm, password);
        
            if ( !digestedPassword.equals( expectedPassword ) ) {
                digestedPassword =
                    MD5Encoder.digestPassword(realUserID, realm, password);
            }
        }
        
        mIdentity = new SimplePrincipal ( realUserID );
        
        if ( expectedPassword.length() == 32 ) {
            if ( digestedPassword.equals( expectedPassword ) ) {
                return true;
            }
        }
        else {
            // this is an remnant of when during very early development we
            // didn't encrypt passwords!
            if ( password.equals( expectedPassword ) ) {
                return true;
            }
        }
        
        return false;
    }


    
    /**
     * Remove "sip:" prefix and domain name suffix 
     * @param username
     * @return
     */
    String normalizeUserName( String username ) {
        int startIndex = 0;
        int endIndex = username.length();

        if ( username.toLowerCase().startsWith( "sip:") )
            startIndex = 4;

        if ( username.indexOf( '@' ) != -1 ) {
            // need to check realm
            endIndex = username.indexOf( '@' );
        }

        return username.substring( startIndex, endIndex );
    }

    /**
     * Overriden by subclasses to return the Principal that corresponds to
     * the user primary identity.
     *
     * @return
     */
    protected Principal getIdentity() {
        return mIdentity;
    }


    /**
     * Overriden by subclasses to return the Groups that correspond to the
     * to the role sets assigned to the user. Subclasses should create at
     * least a Group named "Roles" that contains the roles assigned to the user.
     * A second common group is "CallerPrincipal" that provides the application
     * identity of the user rather than the security domain identity.
     *
     * @return Group[] containing the sets of roles
     * @throws LoginException if the authentication fails for any reason.
     */
    protected Group[] getRoleSets() throws LoginException {

        String username = getUsername();
        Connection conn = null;
        HashMap setsMap = new HashMap();
        PreparedStatement ps = null;

        try {
            conn = getConnection();
            // Get the users role names
            ps = conn.prepareStatement(ROLES_QUERY);
            ps.setString(1, username);
            ResultSet rs = ps.executeQuery();

            if( rs.next() == false ) {
                Group[] roleSets = { new SimpleGroup("Roles") };
                return roleSets;
            }

            do {
                String name = rs.getString(1);
                String groupName = rs.getString(2);

                if( groupName == null || groupName.length() == 0 )
                    groupName = "Roles";

                Group group = (Group) setsMap.get(groupName);

                if( group == null ) {
                    group = new SimpleGroup(groupName);
                    setsMap.put(groupName, group);
                }

                group.addMember(new SimplePrincipal(name));
            }
            while( rs.next() );

            rs.close();
        }
        catch(NamingException ex) {
            throw new LoginException(ex.toString(true));
        }
        catch(SQLException ex) {
            log.error("SQL failure", ex);
            throw new LoginException(ex.toString());
        }
        finally {
            if( ps != null ) {
                try {
                    ps.close();
                }
                catch(SQLException e)
                {}
            }
            if( conn != null ) {
                try {
                    conn.close();
                }
                catch (Exception ex)
                {}
            }
        }

        Group[] roleSets = new Group[setsMap.size()];
        setsMap.values().toArray(roleSets);
        return roleSets;
    }


    protected String getUsername() {
        String username = null;
        if( getIdentity() != null )
            username = getIdentity().getName();
        return username;
    }


//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    private String [] getUserPasswordFromDB (String userid)
            throws LoginException {

        String password = null;
        Connection conn = null;
        PreparedStatement ps = null;
        String realUserID = null;

        try {
            conn = getConnection();
            // Get the password
            ps = conn.prepareStatement( PRINCIPALS_QUERY );
            ps.setString(1, userid );
            ps.setString(2, userid );
            ResultSet rs = ps.executeQuery();

            if ( rs.next() ) {
                password = rs.getString( 1 );
                realUserID = rs.getString( 2 );
            }

            rs.close();
        }
        catch(NamingException ex) {
            throw new LoginException(ex.toString(true));
        }
        catch(SQLException ex) {
            log.error("Query failed", ex);
            throw new LoginException(ex.toString());
        }
        finally {
            if( ps != null ) {
                try {
                    ps.close();
                }
                catch(SQLException e) {}
            }

            if( conn != null ) {
                try {
                conn.close();
                }
                catch (SQLException ex) {}
            }
        }

        return new String [] { realUserID,password };
    }


    private String getRealm () {
        return PGS_PROPERTIES.getProperty(PathLocatorUtil.PGS_SIPXCHANGE_REALM,"");
    }

    private String getDnsDomain () throws LoginException {

        String realm = null;
        Connection conn = null;
        PreparedStatement ps = null;
        PreparedStatement psCount = null;

        try {
            conn = getConnection();

            psCount = conn.prepareStatement( DNS_DOMAIN_COUNT_QUERY );

            ResultSet rsCount = psCount.executeQuery();

            rsCount.next();
            int count = rsCount.getInt(1);

            // This is for the enterprise model
            if (count == 1) {

                ps = conn.prepareStatement( DNS_DOMAIN_QUERY );

                ResultSet rs = ps.executeQuery();
                if( rs.next() == false )
                    throw new FailedLoginException(
                            "No matching username found in Principals");

                realm = rs.getString(1);
                rs.close();
            }
            else {  // this if for the SP model; we need to parse the userid
                //assert true : "we have hit more than one domain.";
            }

        }
        catch(NamingException ex) {
            throw new LoginException(ex.toString(true));
        }
        catch(SQLException ex) {
            log.error("Query failed", ex);
            throw new LoginException(ex.toString());
        }
        finally {
            if( ps != null ) {
                try {
                    ps.close();
                }
                catch(SQLException e) {}
            }
            if( conn != null ) {
                try {
                    conn.close();
                }
                catch (SQLException ex) {}
            }
        }

        return realm;
    }


    private String getExistingProfileEncryptionKey(String userName)
            throws LoginException {

        Connection conn = null;
        PreparedStatement ps = null;

        try {
            conn = getConnection();
            ps = conn.prepareStatement( "SELECT PROFILE_ENCRYPTION_KEY " +
                                        "FROM USERS " +
                                        "WHERE DISPLAY_ID = ?");

            ps.setString(1, userName);

            ResultSet rs = ps.executeQuery();
            rs.next();

            String profileEncryptionKey = rs.getString(1);

            return profileEncryptionKey;
        }
        catch(NamingException ex) {
            throw new LoginException(ex.toString(true));
        }
        catch(SQLException ex) {
            log.error("Query failed", ex);
            throw new LoginException(ex.toString());
        }
        finally {
            if( ps != null ) {
                try {
                    ps.close();
                }
                catch(SQLException e) {}
            }
            if( conn != null ) {
                try {
                    conn.close();
                }
                catch (SQLException ex) {}
            }
        }
    }


    private void updateProfileEncryptionKey(    String userName,
                                                String enteredPEK)
            throws LoginException {

        Connection conn = null;
        PreparedStatement ps = null;

        try {
            conn = getConnection();
            ps = conn.prepareStatement( "UPDATE USERS " +
                                        "SET PROFILE_ENCRYPTION_KEY = ? " +
                                        "WHERE DISPLAY_ID = ?");

            ps.setString(1, enteredPEK);
            ps.setString(2, userName);

            ps.executeUpdate();
            conn.commit();
        }
        catch(NamingException ex) {
            throw new LoginException(ex.toString(true));
        }
        catch(SQLException ex) {
            log.error("Query failed", ex);
            throw new LoginException(ex.toString());
        }
        finally {
            if( ps != null ) {
                try {
                    ps.close();
                }
                catch(SQLException e) {}
            }
            if( conn != null ) {
                try {
                    conn.close();
                }
                catch (SQLException ex) {}
            }
        }
    }

    private SIPxchangeAuthPlugin instantiatePlugIn(String plugInClassName)
            throws LoginException {
        try {
            return (SIPxchangeAuthPlugin)
                    Class.forName(plugInClassName).newInstance();
        }
        catch(Exception e) {
            e.printStackTrace();
            throw new LoginException(e.getMessage());
        }

    }


    private void updatePassword(    String userName,
                                    String realm,
                                    String password) throws LoginException {

        String newPassword = 
                MD5Encoder.digestPassword(userName, realm, password);

        Connection conn = null;
        PreparedStatement ps = null;

        try {
            conn = getConnection();
            ps = conn.prepareStatement( "UPDATE USERS " +
                                        "SET PASSWORD = ? " +
                                        "WHERE DISPLAY_ID = ?");

            ps.setString(1, newPassword);
            ps.setString(2, userName);

            ps.executeUpdate();
            conn.commit();
        }
        catch(NamingException ex) {
            throw new LoginException(ex.toString(true));
        }
        catch(SQLException ex) {
            log.error("Query failed", ex);
            throw new LoginException(ex.toString());
        }
        finally {
            if( ps != null ) {
                try {
                    ps.close();
                }
                catch(SQLException e) {}
            }
            if( conn != null ) {
                try {
                    conn.close();
                }
                catch (SQLException ex) {}
            }
        }
    }


    private Connection getConnection()
            throws NamingException, SQLException {

        InitialContext ctx = new InitialContext();
        DataSource ds = (DataSource) ctx.lookup(mDataSourceJndiName);
        Connection conn = ds.getConnection();
        return conn;
    }

//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////



}
