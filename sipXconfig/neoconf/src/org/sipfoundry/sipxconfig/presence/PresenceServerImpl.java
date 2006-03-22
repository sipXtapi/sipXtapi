/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.presence;

import java.net.MalformedURLException;
import java.util.Hashtable;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.commserver.SipxServer;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcProxyFactoryBean;

/**
 * Interaction with PresenceServer
 */
public class PresenceServerImpl implements PresenceServer {    
    public static final String OBJECT_CLASS_KEY = "object-class";
    private CoreContext m_coreContext;
    private SipxServer m_sipxServer;
    private boolean m_enabled;
    
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setSipxServer(SipxServer sipxServer) {
        m_sipxServer = sipxServer;
    }
    
    public boolean isEnabled() {
        return m_enabled;
    }
    
    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public void signIn(User user) {
        signInAction(SignIn.SIGN_IN, user);
    }
    
    public void signOut(User user) {
        signInAction(SignIn.SIGN_OUT, user);
    }   
    
    public PresenceStatus getStatus(User user) {
        Hashtable response = signInAction(SignIn.STATUS, user);
        String status = (String) response.get(SignIn.RESULT_TEXT);
        return PresenceStatus.resolve(status);
    }
    
    private Hashtable signInAction(String action, User user) {        
        XmlRpcProxyFactoryBean factory = new XmlRpcProxyFactoryBean();
        factory.setServiceInterface(SignIn.class);
        factory.setServiceUrl(m_sipxServer.getPresenceServiceUri());
        try {
            factory.afterPropertiesSet();
        } catch (MalformedURLException e) {
            throw new RuntimeException("Could not construct URI to presence server", e);
        }
        SignIn api = (SignIn) factory.getObject();
        return userAction(api, action, user);
    }
    
    Hashtable userAction(SignIn api, String actionId, User user) {
        Hashtable action = new Hashtable();
        String domainName = m_coreContext.getDomainName();
        action.put(OBJECT_CLASS_KEY, SignIn.OBJECT_CLASS_ID);
        action.put(actionId, user.getUri(domainName));
        Hashtable response = api.action(action);
        checkErrorCode(response);
        return response;
    }
    
    static void checkErrorCode(Hashtable response) {
        Integer resultCode = (Integer) response.get(SignIn.RESULT_CODE);
        if (!SignIn.SUCCESS.equals(resultCode)) {
            String rawMessage = (String) response.get(SignIn.RESULT_TEXT);
            String message = StringUtils.defaultString(rawMessage, "Error calling remote api");
            throw new SignInException(message);
        }
    }
    
    static class SignInException extends UserException {
        SignInException(String msg) {
            super(msg);
        }
    }
    
    /**
     * Raw API from presense server. Not very useful outside this context
     */
    public static interface SignIn {
        public static final Integer SUCCESS = new Integer(1);
        public static final String RESULT_CODE = "result-code";
        public static final String RESULT_TEXT = "result-text";
        public static final String OBJECT_CLASS_ID = "login";
        public static final String SIGN_IN = "sign-in";
        public static final String STATUS = "sign-in-status";
        public static final String SIGN_OUT = "sign-out";
        public Hashtable action(Hashtable params);
    };
}
