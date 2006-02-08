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

import java.io.File;

import org.apache.tapestry.AbstractComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;

/**
 * Link that allows for dwonlaoding files from web server
 */
public abstract class DownloadLink extends AbstractComponent {
    public abstract String getFileName();

    public abstract String getDirName();

    public abstract String getContentType();

    protected void renderComponent(IMarkupWriter w, IRequestCycle cycle) {
        if (cycle.isRewinding()) {
            return;
        }
        File file = getFile(getDirName(), getFileName());
        if (null == file) {
            return;
        }

//        IEngineService srv = cycle.getEngine().getService(DownloadService.SERVICE_NAME);
//        ILink lnk = srv.getLink(cycle, this, new Object[] {
//            file.getAbsolutePath(), getContentType()
//        });
        w.begin("a");
        
//        w.attribute("href", lnk.getURL());
        w.attribute("href", "todo");
      
        renderInformalParameters(w, cycle);
        renderBody(w, cycle);
        w.end();
    }

    /**
     * Creates file object from directory name andbase file name.
     * 
     * @param dirName directory name
     * @param fileName base file name
     * @return file object if it's possible to read from the file, null if not
     */
    private static File getFile(String dirName, String fileName) {
        if (null == dirName || null == fileName) {
            return null;
        }
        File file = new File(dirName, fileName);
        return file.canRead() ? file : null;
    }
}
