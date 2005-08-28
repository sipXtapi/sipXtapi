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
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.commons.digester.Digester;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.RegistrationItem;
import org.xml.sax.SAXException;

public class RegistrationContextImpl implements RegistrationContext {
    public static final Log LOG = LogFactory.getLog(RegistrationContextImpl.class);

    private static final String REGISTRATION_URL = "https://{0}:{1}/sipdb/registration.xml";

    private static final String PATTERN = "items/item";

    private String m_server;

    private String m_port;

    /**
     * @see org.sipfoundry.sipxconfig.admin.commserver.RegistrationContext#getRegistrations()
     */
    public List getRegistrations() {
        try {
            URL url = new URL(getUrl());
            InputStream is = url.openStream();
            return getRegistrations(is);
        } catch (FileNotFoundException e) {
            // we are handling this separately - server returns FileNotFound even if everything is
            // OK but we have no registrations present
            LOG.info("No registrations found on the server" + e.getMessage());
            return Collections.EMPTY_LIST;
        } catch (IOException e) {
            LOG.error("Retrieving active registrations failed", e);
            return Collections.EMPTY_LIST;
        } catch (SAXException e) {
            throw new RuntimeException(e);
        }
    }

    List getRegistrations(InputStream is) throws IOException, SAXException {
        Digester digester = configureDigester();
        return (List) digester.parse(is);
    }

    String getUrl() {
        Object[] params = {
            m_server, m_port
        };
        return MessageFormat.format(REGISTRATION_URL, params);
    }

    private Digester configureDigester() {
        Digester digester = new Digester();
        digester.setValidating(false);
        digester.setNamespaceAware(false);

        digester.push(new ArrayList());
        digester.addObjectCreate(PATTERN, RegistrationItem.class);
        digester.addSetNestedProperties(PATTERN);
        digester.addSetNext(PATTERN, "add");

        return digester;
    }

    public void setPort(String port) {
        m_port = port;
    }

    public void setServer(String server) {
        m_server = server;
    }
}
