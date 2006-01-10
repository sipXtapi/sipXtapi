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
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;
import org.springframework.beans.factory.BeanFactoryAware;

public class SipxProcessContextImpl extends SipxReplicationContextImpl implements
        BeanFactoryAware, SipxProcessContext {
    // for checkstyle compliance, keep only one copy of this string
    private static final String STATUS = "status";

    private static final String ACTION_STATUS = STATUS;
    private static final String COMMAND_URL_FORMAT = "{0}?command={1}";
    private static final String COMMAND_FOR_PROCESS_URL_FORMAT = "{0}?command={1}&process={2}";

    // Constants related to parsing XML output from the process monitor
    private static final String PROCESS_STATUS_ATTRIB = STATUS;

    /** Read service status values from the process monitor and return them in an array */
    public ServiceStatus[] getStatus(Location location) {        
        InputStream statusStream = getStatusStream(location);
        if (statusStream == null) {
            return new ServiceStatus[0];
        }

        try {
            SAXReader reader = new SAXReader();
            Document document = reader.read(statusStream);

            // Create a ServiceStatus for each process (a.k.a. service) entry.
            // Ignore process grouping, not clear whether that means anything important.
            List list = document.selectNodes("//process");
            ServiceStatus[] statusArray = new ServiceStatus[list.size()];
            for (int i = 0; i < statusArray.length; i++) {
                Element element = (Element) list.get(i);
                String name = element.attribute("name").getValue();
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
    InputStream getStatusStream(Location location) {
        if(location == null) {
            return null;
        }
        // Get a status URL only for the first process monitor
        String statusUrl = constructStatusUrl(location);

        // Get status and return the input stream containing the ouput
        return invokeHttpGetRequest(statusUrl);
    }

    /**
     * Invoke an HTTP GET request at the specified URL. Return an input stream on the output.
     */
    protected InputStream invokeHttpGetRequest(String urlString) {
        try {
            URL url = new URL(urlString);
            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.setDoOutput(true);
            connection.setRequestMethod("GET");
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

    public void manageServices(Collection processes, Command command) {
        Location[] locations = getLocations();
        for (int i = 0; i < locations.length; i++) {
            Location location = locations[i];
            manageServices(location, processes, command);
        }
    }

    public void manageServices(Location location, Collection processes, Command command) {
        for (Iterator i = processes.iterator(); i.hasNext();) {
            Process process = (Process) i.next();
            manageService(location, process, command);
        }
    }

    public void manageService(Location location, Process process, Command command) {
        String commandUrl = constructCommandUrl(location, process, command);
        invokeHttpGetRequest(commandUrl);
    }

    /**
     * Construct and return a command URL.
     * 
     * Make a GET request to the command URL to apply the specified command to the named service.
     * 
     * @return URL string
     * @param process service on which the command will be invoked
     * @param command command to invoke
     */
    String constructCommandUrl(Location location, Process process, Command command) {
        try {
            final String serviceId = URLEncoder.encode(process.getName(), "UTF-8");
            final Object[] params = {
                location.getProcessMonitorUrl(), command.getName(), serviceId
            };
            return MessageFormat.format(COMMAND_FOR_PROCESS_URL_FORMAT, params);
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Construct and return a string array of URLs for getting service status.
     */
    String constructStatusUrl(Location location) {
        final Object[] params = {
            location.getProcessMonitorUrl(), ACTION_STATUS
        };
        return MessageFormat.format(COMMAND_URL_FORMAT, params);
    }
}
