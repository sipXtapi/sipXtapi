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

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLEncoder;
import java.text.MessageFormat;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.io.SAXReader;

public class SipxProcessContextImpl implements SipxProcessContext {
    private static final Log LOG = LogFactory.getLog(SipxProcessContextImpl.class);

    private static final String ACTION_RESTART = "restart";
    private static final String URL_FORMAT = "{0}?command={1}&process={2}";
    private static final String TOPOLOGY_XML = "topology.xml";

    private String m_processUrl;

    private String m_configDirectory;

    public void restart(Process process) {
        try {
            String urlText = constructRestartUrl(process);
            URL url = new URL(urlText);
            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.setDoOutput(true);
            connection.setRequestMethod("GET");
            connection.connect();
            connection.getInputStream();
        } catch (IOException e) {
            // HACK: ignore exception
            // at the moment process.cgi returns 500. Internal Server Error whenever services are
            // restarted
            LOG.error("Restarting " + process + ":" + e.getMessage());
        }
    }

    String constructRestartUrl(Process process) {
        try {
            if (null == m_processUrl) {
                m_processUrl = retrieveProcessUrl();
            }
            String processId = URLEncoder.encode(process.getName(), "UTF-8");
            return MessageFormat.format(URL_FORMAT, new Object[] {
                m_processUrl, ACTION_RESTART, processId
            });
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException(e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } catch (DocumentException e) {
            throw new RuntimeException(e);
        }
    }

    String retrieveProcessUrl(InputStream stream) throws DocumentException {
        SAXReader xmlReader = new SAXReader();
        Document topology = xmlReader.read(stream);
        return topology.valueOf("/topology/location/agent_url").trim();
    }

    String retrieveProcessUrl() throws DocumentException, FileNotFoundException {
        File file = new File(m_configDirectory, TOPOLOGY_XML);
        InputStream stream = new FileInputStream(file);
        return retrieveProcessUrl(stream);
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }
}
