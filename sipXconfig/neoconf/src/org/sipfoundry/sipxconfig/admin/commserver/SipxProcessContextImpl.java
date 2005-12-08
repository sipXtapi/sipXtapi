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

import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLEncoder;
import java.text.MessageFormat;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;
import org.springframework.beans.factory.BeanFactoryAware;

public class SipxProcessContextImpl extends SipxReplicationContextImpl implements
        BeanFactoryAware, SipxProcessContext {
    /** if set to TRUE status operations are only performed on a single server */
    /** commands operations do not use this by default - commands are sent to all servers */
    private static final boolean ONE_SERVER_ONLY = true;

    // for checkstyle compliance, keep only one copy of this string
    private static final String STATUS = "status";

    private static final String HTTP_GET = "GET";
    private static final String ACTION_STATUS = STATUS;
    private static final String COMMAND_URL_FORMAT = "{0}?command={1}";
    private static final String COMMAND_FOR_PROCESS_URL_FORMAT = "{0}?command={1}&process={2}";

    // Constants related to parsing XML output from the process monitor
    private static final String FIND_PROCESS_ELEMENT_XPATH = "//process";
    private static final String PROCESS_NAME_ATTRIB = "name";
    private static final String PROCESS_STATUS_ATTRIB = STATUS;

    /** Read service status values from the process monitor and return them in an array */
    public ServiceStatus[] getStatus() {
        InputStream statusStream = getStatusStream();
        if (statusStream == null) {
            return new ServiceStatus[0];
        }

        try {
            SAXReader reader = new SAXReader();
            Document document = reader.read(statusStream);

            // Create a ServiceStatus for each process (a.k.a. service) entry.
            // Ignore process grouping, not clear whether that means anything important.
            List list = document.selectNodes(FIND_PROCESS_ELEMENT_XPATH);
            ServiceStatus[] statusArray = new ServiceStatus[list.size()];
            for (int i = 0; i < statusArray.length; i++) {
                Element element = (Element) list.get(i);
                String name = element.attribute(PROCESS_NAME_ATTRIB).getValue();
                String status = element.attribute(PROCESS_STATUS_ATTRIB).getValue();

                // Map the status string to a status enum value. For robustness, if the status
                // string is unknown, map it to the special "unknown" status value, in case the
                // process monitor surprises us. We do that here rather than in the getEnum
                // method because the enum itself shouldn't know about the semantics of its
                // values.
                ServiceStatus.Status serviceStatus = ServiceStatus.Status.getEnum(status);
                if (serviceStatus == null) {
                    serviceStatus = ServiceStatus.Status.UNKNOWN;
                }

                statusArray[i] = new ServiceStatus(name, serviceStatus);
            }
            return statusArray;
        } catch (DocumentException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(statusStream);
        }
    }

    /**
     * Get service status and return it as a stream.
     */
    InputStream getStatusStream() {
        // Get a status URL only for the first process monitor
        String[] statusUrls = constructStatusUrls();
        if (statusUrls.length == 0) {
            return null;
        }

        // Get status and return the input stream containing the ouput
        return invokeHttpGetRequest(statusUrls[0]);
    }

    /**
     * Invoke an HTTP GET request at the specified URL. Return an input stream on the output.
     */
    private InputStream invokeHttpGetRequest(String urlString) {
        try {
            URL url = new URL(urlString);
            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.setDoOutput(true);
            connection.setRequestMethod(HTTP_GET);
            connection.connect();
            // *Always* call getInputStream, even when the caller doesn't want the stream.
            // For some reason this is necessary, at least currently.
            return connection.getInputStream();
        } catch (IOException e) {
            // HACK: ignore exception
            // at the moment process.cgi returns 500. Internal Server Error
            LOG.error("Error when invoking HTTP GET: " + e.getMessage());
        }
        return null;
    }

    public void manageServices(Process[] services, Command command) {
        for (int i = 0; i < services.length; i++) {
            manageService(services[i], command);
        }
    }

    public void manageServices(String[] serviceNames, Command command) {
        for (int i = 0; i < serviceNames.length; i++) {
            String serviceName = serviceNames[i];
            manageService(serviceName, command);
        }
    }

    private void manageService(String serviceName, Command command) {
        // Construct command URLs to invoke the specified command on the named service.
        String[] commandUrls = constructCommandUrls(serviceName, command);

        // Invoke commands
        for (int i = 0; i < commandUrls.length; i++) {
            invokeHttpGetRequest(commandUrls[i]);
        }
    }

    public void manageService(Process process, Command command) {
        manageService(process.getName(), command);
    }

    // ================================================================================
    // URL Munging
    // ================================================================================

    /**
     * Construct and return a string array of command URLs, one for each process monitor. Make a
     * GET request to the command URL to apply the specified command to the named service. If
     * firstOnly is true, then return a command URL for the first process monitor only.
     * 
     * @return array of command URL strings
     * @param serviceName named service on which the command will be invoked
     * @param command command to invoke
     */
    String[] constructCommandUrls(String serviceName, Command command) {
        try {
            String serviceId = URLEncoder.encode(serviceName, "UTF-8");
            String[] urls = getProcessMonitorUrls();
            String urlFormat = COMMAND_FOR_PROCESS_URL_FORMAT;
            Object[] params = {
                null, command.getName(), serviceId
            };
            // false means here - send command to all the servers
            return formatUrls(urls, urlFormat, params, false);
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Construct and return a string array of URLs for getting service status.
     */
    String[] constructStatusUrls() {
        String[] urls = getProcessMonitorUrls();
        Object[] params = {
            null, ACTION_STATUS
        };
        String urlFormat = COMMAND_URL_FORMAT;
        return formatUrls(urls, urlFormat, params, ONE_SERVER_ONLY);
    }

    String[] getProcessMonitorUrls() {
        Location[] locations = getLocations();        
        String[] urls = new String[locations.length];
        for (int i = 0; i < locations.length; i++) {
            urls[i] = locations[i].getProcessMonitorUrl();            
        }
        return urls;
    }

    private String[] formatUrls(String[] urls, String urlFormat, Object[] params, boolean oneServerOnly) {
        int len = urls.length;
        if (oneServerOnly && len > 0) {
            len = 1;
        }
        String[] commandUrls = new String[len];
        for (int i = 0; i < len; i++) {
            params[0] = urls[i];
            commandUrls[i] = MessageFormat.format(urlFormat, params);
        }
        return commandUrls;
    }
}
