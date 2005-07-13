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
import java.util.Iterator;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Node;
import org.dom4j.io.SAXReader;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSetGenerator;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.ReplicationManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;

public class SipxProcessContextImpl implements BeanFactoryAware, SipxProcessContext {
    private static final Log LOG = LogFactory.getLog(SipxProcessContextImpl.class);

    private static final String ACTION_RESTART = "restart";
    private static final String URL_FORMAT = "{0}?command={1}&process={2}";
    private static final String TOPOLOGY_XML = "topology.xml";

    /** these are lazily constructed - always use accessors */
    private String[] m_processUrls;
    private String[] m_replicationUrls;

    private String m_configDirectory;

    private BeanFactory m_beanFactory;

    private ReplicationManager m_replicationManager;

    public void restart(Process process) {
        try {
            String[] urlTexts = constructRestartUrl(process);
            for (int i = 0; i < urlTexts.length; i++) {
                String urlText = urlTexts[i];

                URL url = new URL(urlText);
                HttpURLConnection connection = (HttpURLConnection) url.openConnection();
                connection.setDoOutput(true);
                connection.setRequestMethod("GET");
                connection.connect();
                connection.getInputStream();
            }
        } catch (IOException e) {
            // HACK: ignore exception
            // at the moment process.cgi returns 500. Internal Server Error whenever services are
            // restarted
            LOG.error("Restarting " + process + ":" + e.getMessage());
        }
    }

    String[] constructRestartUrl(Process process) {
        try {
            String[] processUrls = getProcessUrls();
            String[] restartUrls = new String[processUrls.length];
            for (int i = 0; i < processUrls.length; i++) {
                String processId = URLEncoder.encode(process.getName(), "UTF-8");
                restartUrls[i] = MessageFormat.format(URL_FORMAT, new Object[] {
                    processUrls[i], ACTION_RESTART, processId
                });
            }
            return restartUrls;
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException(e);
        }
    }

    private String[] retrieveUrls(Document topology, String xmlPath) {
        List nodes = topology.selectNodes(xmlPath);
        String[] urls = new String[nodes.size()];
        for (int i = 0; i < urls.length; i++) {
            Node node = (Node) nodes.get(i);
            urls[i] = node.getText().trim();
        }
        return urls;
    }

    private void retrieveUrls() {
        try {
            InputStream stream = getTopologyAsStream();
            SAXReader xmlReader = new SAXReader();
            Document topology = xmlReader.read(stream);

            m_processUrls = retrieveUrls(topology, "/topology/location/agent_url");
            m_replicationUrls = retrieveUrls(topology, "/topology/location/replication_url");
        } catch (IOException e) {
            throw new RuntimeException(e);
        } catch (DocumentException e) {
            throw new RuntimeException(e);
        }
    }

    protected InputStream getTopologyAsStream() throws FileNotFoundException {
        File file = new File(m_configDirectory, TOPOLOGY_XML);
        InputStream stream = new FileInputStream(file);
        return stream;
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    public void generate(DataSet dataSet) {
        String beanName = dataSet.getBeanName();
        DataSetGenerator generator = (DataSetGenerator) m_beanFactory.getBean(beanName,
                DataSetGenerator.class);
        m_replicationManager.replicateData(getReplicationUrls(), generator.generate(), dataSet);
    }

    public void generateAll() {
        for (Iterator i = DataSet.iterator(); i.hasNext();) {
            DataSet dataSet = (DataSet) i.next();
            generate(dataSet);
        }
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public void setReplicationManager(ReplicationManager replicationManager) {
        m_replicationManager = replicationManager;
    }

    String[] getProcessUrls() {
        if (m_processUrls == null) {
            retrieveUrls();
        }
        return m_processUrls;
    }

    String[] getReplicationUrls() {
        if (m_replicationUrls == null) {
            retrieveUrls();
        }
        return m_replicationUrls;
    }
}
