/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

import junit.framework.TestCase;

import org.jdom.input.SAXBuilder;
import java.io.FileInputStream;
import java.net.URL;
import com.pingtel.pds.common.PathBuffer;

/**
 * Load device defs XML to make sure they are valid
 */
public class DeviceDefsXmlTest extends TestCase 
{
    /**
     * This tests device files are valid XML, admittingly, you could
     * devise a more more thurough in XML scripts for sure.
     */
    public void testValid()
    {
        String filename = "newxpressa.xml";
        try
        {
            PathBuffer xmlfile = new PathBuffer("file://")
                .append(getBaseDir()).dir("..").dir("meta")
                .dir("devicedefs").slash().append(filename);

            // URL based uses relative resolving of DTD and XML includes
            SAXBuilder builder = new SAXBuilder();
            new SAXBuilder().build(new URL(xmlfile.toString()));
        }
        catch (Exception e)
        {
            e.printStackTrace();
            fail("Error parsing devicedefs file " + filename);
        }
    }

    private String getBaseDir()
    {
        String path = System.getProperty("basedir");
        assertNotNull("-Dbasedir not defined, should point to directory where build.xml is found", 
            path);
        return path;
    }
}
