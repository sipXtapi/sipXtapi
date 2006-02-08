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
package org.sipfoundry.sipxconfig.components;


public class DownloadService /* extends AbstractService */ {

//    public static final String SERVICE_NAME = "DownloadService";
//
//    public String getName() {
//        return SERVICE_NAME;
//    }
//
//    /**
//     * The only parameter is the service parameters[dirName, fileName]
//     */
//    public void service(IEngineServiceView iEngineServiceView_, IRequestCycle cycle,
//            ResponseOutputStream responseOutputStream) throws IOException {
//        File file = getFile(cycle);
//        if (!file.canRead()) {
//            return;
//        }
//
//        responseOutputStream.setContentType(getContentType(cycle));
//        HttpServletResponse response = cycle.getRequestContext().getResponse();
//        response.setHeader("Expires", "0");
//        response.setHeader("Cache-Control", "must-revalidate, post-check=0, pre-check=0");
//        response.setHeader("Pragma", "public");
//        response.setHeader("Content-Disposition", "attachment; filename=\"" + file.getName()
//                + "\"");
//
//        InputStream stream = new FileInputStream(file);
//        IOUtils.copy(stream, responseOutputStream);
//    }
//
//    /**
//     * Retrieves the file object from service parameters (dir, filename)
//     * 
//     * @param cycle
//     * @return nely created file object
//     */
//    private File getFile(IRequestCycle cycle) {
//        Object[] params = getParameters(cycle);
//        String fileName = (String) params[0];
//        return new File(fileName);
//    }
//
//    /**
//     * Retrieves the content type from service parameters
//     * 
//     * @param cycle
//     * @return nely created file object
//     */
//    private String getContentType(IRequestCycle cycle) {
//        Object[] params = getParameters(cycle);
//        return (String) params[1];
//    }
//
//    public ILink getLink(IRequestCycle iRequestCycle, IComponent iComponent_, Object[] params) {
//        return constructLink(iRequestCycle, getName(), null, params, false);
//    }
}
