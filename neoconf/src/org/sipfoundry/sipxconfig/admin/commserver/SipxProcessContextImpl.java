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
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLEncoder;
import java.text.MessageFormat;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.dom4j.Attribute;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.Node;
import org.dom4j.io.SAXReader;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSetGenerator;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.ReplicationManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;

public class SipxProcessContextImpl implements BeanFactoryAware, SipxProcessContext {
    private static final Log LOG = LogFactory.getLog(SipxProcessContextImpl.class);
    
    // for checkstyle compliance, keep only one copy of this string
    private static final String STATUS = "status";  
    
    private static final String HTTP_GET = "GET";
    private static final String ACTION_STATUS = STATUS;    
    private static final String COMMAND_URL_FORMAT = "{0}?command={1}";
    private static final String COMMAND_FOR_PROCESS_URL_FORMAT = "{0}?command={1}&process={2}";
    private static final String TOPOLOGY_XML = "topology.xml";

    // Constants related to parsing XML output from the process monitor
    private static final String FIND_PROCESS_ELEMENT_XPATH = "//process";
    private static final String PROCESS_NAME_ATTRIB = "name";
    private static final String PROCESS_STATUS_ATTRIB = STATUS;
    
    /** these are lazily constructed - always use accessors */
    private String[] m_processMonitorUrls;
    private String[] m_replicationUrls;

    private String m_configDirectory;

    private BeanFactory m_beanFactory;

    private ReplicationManager m_replicationManager;
    
    // TODO: Generalize this to report status for multiple servers instead of just the first one.
    /** Read service status values from the process monitor and return them in an array */
    public ServiceStatus[] getStatus() {
        ServiceStatus[] statusArray = null; 
        InputStream statusStream = getStatusStream();
        if (statusStream == null) {
            return new ServiceStatus[0];
        }
        
        SAXReader reader = new SAXReader();
        try {
            Document document = reader.read(statusStream);
            
            // Create a ServiceStatus for each process (a.k.a. service) entry.
            // Ignore process grouping, not clear whether that means anything important.
            List list = document.selectNodes(FIND_PROCESS_ELEMENT_XPATH);
            statusArray = new ServiceStatus[list.size()];
            int statusArrayIndex = 0;
            for (Iterator iter = list.iterator(); iter.hasNext();) {
                Element element = (Element) iter.next();
                Attribute name = element.attribute(PROCESS_NAME_ATTRIB);
                Attribute status = element.attribute(PROCESS_STATUS_ATTRIB);
                String serviceName = name.getValue();
                
                // Map the status string to a status enum value.  For robustness, if the status
                // string is unknown, map it to the special "unknown" status value, in case the
                // process monitor surprises us.  We do that here rather than in the getEnum
                // method because the enum itself shouldn't know about the semantics of its values.
                ServiceStatus.Status serviceStatus = ServiceStatus.Status.getEnum(status.getValue());
                if (serviceStatus == null) {
                    serviceStatus = ServiceStatus.Status.UNKNOWN;
                }
                
                statusArray[statusArrayIndex++] = new ServiceStatus(serviceName, serviceStatus);
            }
        } catch (DocumentException e) {
            throw new RuntimeException(e);
        } finally {
            try {
                statusStream.close();
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
        }

        return statusArray;
    }

    /**
     * Get service status and return it as a stream.
     * For now, call only the first process monitor.
     */
    InputStream getStatusStream() {
        // Get a status URL only for the first process monitor
        String[] statusUrls = constructStatusUrls(true);
        if (statusUrls.length == 0) {
            return null;
        }
        
        // Get status and return the input stream containing the ouput
        URL statusUrl = createUrl(statusUrls[0]);
        return invokeHttpGetRequest(statusUrl);
    }
    
    /**
     * Invoke an HTTP GET request at the specified URL.  Return an input stream on the output.
     */
    private InputStream invokeHttpGetRequest(URL url) {
        InputStream inputStream = null;
        try {
            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.setDoOutput(true);
            connection.setRequestMethod(HTTP_GET);
            connection.connect();
            
            // *Always* call getInputStream, even when the caller doesn't want the stream.
            // For some reason this is necessary, at least currently.
            inputStream = connection.getInputStream();
        } catch (IOException e) {
            // HACK: ignore exception
            // at the moment process.cgi returns 500. Internal Server Error
            LOG.error("Error when invoking HTTP GET: " + e.getMessage());
        }
        return inputStream;
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

    public void manageServices(String[] serviceNames, Command command) {
        for (int i = 0; i < serviceNames.length; i++) {
            String serviceName = serviceNames[i];
            manageService(serviceName, command);
        }
    }

    public void manageService(String serviceName, Command command) {
        // Construct command URLs to invoke the specified command on the named service.
        // Pass in "true" to do this for the first process monitor only, for now.
        String[] commandUrls = constructCommandUrls(serviceName, command, true);        
        
        // Invoke commands
        for (int j = 0; j < commandUrls.length; j++) {
            URL commandUrl = createUrl(commandUrls[j]);
            invokeHttpGetRequest(commandUrl);
        }
    }
    
    public void manageService(Process process, Command command) {
        manageService(process.getName(), command);
    }
    
    //================================================================================
    // URL Munging
    //================================================================================

    /**
     * Construct and return a string array of command URLs, one for each process monitor.
     * Make a GET request to the command URL to apply the specified command to the named service.
     * If firstOnly is true, then return a command URL for the first process monitor only.
     * 
     * @return array of command URL strings
     * @param serviceName named service on which the command will be invoked
     * @param command command to invoke
     * @param firstOnly if true, then return a command URL for the first process monitor only
     */
    String[] constructCommandUrls(String serviceName, Command command, boolean firstOnly) {
        try {
            String[] processMonitorUrls = getProcessMonitorUrls();
            if (firstOnly && processMonitorUrls.length > 0) {
                processMonitorUrls = new String[] {processMonitorUrls[0]};
            }
            String[] commandUrls = new String[processMonitorUrls.length];
            for (int i = 0; i < processMonitorUrls.length; i++) {
                String serviceId = URLEncoder.encode(serviceName, "UTF-8");
                commandUrls[i] = MessageFormat.format(COMMAND_FOR_PROCESS_URL_FORMAT, new Object[] {
                    processMonitorUrls[i], command.getName(), serviceId
                });
            }
            return commandUrls;
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException(e);
        }
    }

    String[] constructCommandUrls(String serviceName, Command command) {
        return constructCommandUrls(serviceName, command, false);
    }

    String[] constructCommandUrls(Process process, Command command) {
        return constructCommandUrls(process.getName(), command, false);
    }

    /**
     * Construct and return a string array of URLs for getting service status.
     * If firstOnly is true, then do so only for the first process monitor.
     */
    String[] constructStatusUrls(boolean firstOnly) {
        String[] processMonitorUrls = getProcessMonitorUrls();
        if (firstOnly && processMonitorUrls.length > 0) {
            processMonitorUrls = new String[] {processMonitorUrls[0]};
        }
        String[] commandUrls = new String[processMonitorUrls.length];

        // Construct the request URLs
        for (int i = 0; i < processMonitorUrls.length; i++) {
            String processMonitorUrl = processMonitorUrls[i];
            commandUrls[i] =
                MessageFormat.format(COMMAND_URL_FORMAT,
                                     new Object[] {processMonitorUrl, ACTION_STATUS});
        }
        return commandUrls;
    }
    
    String[] constructStatusUrls() {
        return constructStatusUrls(false);
    }
    
    /** Return the process monitor URLs, retrieving them on demand */
    String[] getProcessMonitorUrls() {
        if (m_processMonitorUrls == null) {
            retrieveUrls();
        }
        return m_processMonitorUrls;
    }

    /** Return the replication URLs, retrieving them on demand */
    String[] getReplicationUrls() {
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
     * Given a topology document and the XML path to elements containing URL values,
     * retrieve all such URLs and return them as a string array
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
    
    //================================================================================
    // Generic Utilities
    // TODO: Consider moving these methods to a more central place
    //================================================================================

    /**
     * Create an URL from text and return it.
     * Wrap MalformedURLExceptions with RuntimeExceptions so that callers don't
     * have to explicitly handle them.
     */
    public URL createUrl(String urlText) {
        URL url;
        try {
            url = new URL(urlText);
        } catch (MalformedURLException e) {
            throw new RuntimeException(e);
        }
        return url;
    }
    
}
