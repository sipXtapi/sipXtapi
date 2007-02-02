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
package org.sipfoundry.sipxconfig.site.common;

import java.util.Date;
import java.util.Locale;

import junit.framework.TestCase;

import org.apache.tapestry.valid.RenderString;
import org.postgresql.util.PGInterval;
import org.sipfoundry.sipxconfig.common.SqlInterval;

public class DefaultTableValueRendererSourceTest extends TestCase {
    
    public void testGetDefaultRender() {
        DefaultTableValueRendererSource renderer = new DefaultTableValueRendererSource();
        assertNull(renderer.getRender("honey creeper", null));        
    }
    
    public void testSqlIntervalRenderer() {
        DefaultTableValueRendererSource renderer = new DefaultTableValueRendererSource();
        PGInterval pg = new PGInterval();
        pg.setMinutes(1);
        RenderString render = (RenderString) renderer.getRender(new SqlInterval(pg), Locale.ENGLISH);
        // assumes Millis is not localized
        assertEquals("1 minute, 0 seconds", render.getString());
    }

    public void testDateRenderer() {
        DefaultTableValueRendererSource renderer = new DefaultTableValueRendererSource();
        RenderString render = (RenderString) renderer.getRender(new Date(0), Locale.ENGLISH);
        assertEquals("12/31/69 7:00 PM", render.getString());
    }
}
