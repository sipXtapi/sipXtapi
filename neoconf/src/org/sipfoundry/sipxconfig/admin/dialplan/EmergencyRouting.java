/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.HashSet;
import java.util.Set;

/**
 * EmergencyRouting
 */
public class EmergencyRouting {
    private boolean m_enabled;
    private Gateway m_defaultGateway;
    private String m_externalNumber;

    private Set m_exceptions = new HashSet();

    public void addException(RoutingException exception) {
        m_exceptions.add(exception);
    }

    public void removeException(RoutingException exception) {
        m_exceptions.remove(exception);
    }

    public void removeException(Integer exceptionId) {
        m_exceptions.remove(new BeanWithId(exceptionId));
    }

    public void apply() {
        // TODO: this is when you do something
    }

    // getters and setters
    public Gateway getDefaultGateway() {
        return m_defaultGateway;
    }

    public void setDefaultGateway(Gateway defaultGateway) {
        m_defaultGateway = defaultGateway;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public Set getExceptions() {
        return m_exceptions;
    }

    public void setExceptions(Set exceptions) {
        m_exceptions = exceptions;
    }

    public String getExternalNumber() {
        return m_externalNumber;
    }

    public void setExternalNumber(String externalNumber) {
        m_externalNumber = externalNumber;
    }
}
