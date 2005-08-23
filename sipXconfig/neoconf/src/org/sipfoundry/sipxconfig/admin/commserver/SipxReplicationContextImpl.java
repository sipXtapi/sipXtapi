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
import java.io.InputStream;
import java.io.Serializable;
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
import org.sipfoundry.sipxconfig.job.JobContext;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;

public class SipxReplicationContextImpl implements BeanFactoryAware, SipxReplicationContext {

    protected static final Log LOG = LogFactory.getLog(SipxProcessContextImpl.class);
    private static final String TOPOLOGY_XML = "topology.xml";
    /** these are lazily constructed - always use accessors */
    private String[] m_processMonitorUrls;
    private String[] m_replicationUrls;
    private String m_configDirectory;
    private BeanFactory m_beanFactory;
    private ReplicationManager m_replicationManager;
    private JobContext m_jobContext;

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    public void generate(DataSet dataSet) {
        Serializable jobId = m_jobContext.schedule("Replication: " + dataSet.getName());
        boolean success = false;
        try {
            m_jobContext.start(jobId);
            String beanName = dataSet.getBeanName();
            DataSetGenerator generator = (DataSetGenerator) m_beanFactory.getBean(beanName,
                    DataSetGenerator.class);
            success = m_replicationManager.replicateData(getReplicationUrls(), generator
                    .generate(), dataSet);
        } finally {
            if (success) {
                m_jobContext.success(jobId);
            } else {
                // there is not really a good info here - advise user to consult log?
                m_jobContext.failure(jobId, null, null);
            }
        }
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

    /** Return the replication URLs, retrieving them on demand */
    protected String[] getReplicationUrls() {
        if (m_replicationUrls == null) {
            retrieveUrls();
        }
        return m_replicationUrls;
    }

    /** Extract URLs from the topology file and save them in data members */
    private void retrieveUrls() {
        try {
            InputStream stream = getTopologyAsStream();
            SAXReader xmlReader = new SAXReader();
            Document topology = xmlReader.read(stream);

            m_processMonitorUrls = retrieveUrls(topology, "/topology/location/agent_url");
            m_replicationUrls = retrieveUrls(topology, "/topology/location/replication_url");
        } catch (FileNotFoundException e) {
            // When running in a test environment, the topology file will not be found
            m_processMonitorUrls = new String[0];
            m_replicationUrls = new String[0];
            LOG.warn("Could not find the file " + TOPOLOGY_XML, e);
        } catch (DocumentException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Given a topology document and the XML path to elements containing URL values, retrieve all
     * such URLs and return them as a string array
     */
    private String[] retrieveUrls(Document topology, String xmlPath) {
        List nodes = topology.selectNodes(xmlPath);
        String[] urls = new String[nodes.size()];
        for (int i = 0; i < urls.length; i++) {
            Node node = (Node) nodes.get(i);
            urls[i] = node.getText().trim();
        }
        return urls;
    }

    /** Open an input stream on the topology file and return it */
    protected InputStream getTopologyAsStream() throws FileNotFoundException {
        File file = new File(m_configDirectory, TOPOLOGY_XML);
        InputStream stream = new FileInputStream(file);
        return stream;
    }

    /** Return the process monitor URLs, retrieving them on demand */
    protected String[] getProcessMonitorUrls() {
        if (m_processMonitorUrls == null) {
            retrieveUrls();
        }
        return m_processMonitorUrls;
    }

    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }
}
