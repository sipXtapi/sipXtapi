/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;

public class DistributionListsTest extends TestCase {

    private DistributionListsReader m_reader;
    private DistributionListsWriter m_writer;
    
    protected void setUp() {
        m_reader = new DistributionListsReader();
        m_writer = new DistributionListsWriter();
        m_writer.setVelocityEngine(TestHelper.getVelocityEngine());
    }
    
    public void testReadPreferences() {
        InputStream in = getClass().getResourceAsStream("200/distribution.xml");
        DistributionList[] lists = m_reader.readObject(new InputStreamReader(in));
        IOUtils.closeQuietly(in);      
        assertEquals(2, lists.length);
    }    
    
    public void testWritePreferences() throws IOException {
        DistributionList[] lists = new DistributionList[] {
                new DistributionList(),
                new DistributionList()
        };
        StringWriter actual = new StringWriter();
        lists[0].setPosition(1);
        lists[0].setExtensions(new String[] {"300", "900"});
        lists[1].setPosition(3);
        m_writer.writeObject(lists, actual);
        StringWriter expected = new StringWriter();
        InputStream expectedIn = getClass().getResourceAsStream("expected-distribution.xml");
        IOUtils.copy(expectedIn, expected);
        IOUtils.closeQuietly(expectedIn);
        assertEquals(expected.toString(), actual.toString());
    }
}
