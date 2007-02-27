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
package org.sipfoundry.sipxconfig.acd.stats;

import java.sql.ResultSet;
import java.sql.Timestamp;
import java.util.Date;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.acd.stats.AcdHistoricalStatsImpl.ColumnTransformer;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class ColumnTransformerTest extends TestCase {

    public void testUtcDateConversion() throws Exception {
        Date signinTime = TestUtil.localizeDateTime("12/19/06 1:40:50 AM GMT0:00");
        Timestamp timeOriginal = new Timestamp(signinTime.getTime());
        Timestamp timeInUtc = new Timestamp(signinTime.getTime());
        
        IMocksControl rsControl = EasyMock.createControl();
        ResultSet rs = rsControl.createMock(ResultSet.class);
        rs.getObject(0);
        rsControl.andReturn(timeOriginal);
        rs.getTimestamp(0, ColumnTransformer.UTC);
        rsControl.andReturn(timeInUtc);        
        rsControl.replay();

        ColumnTransformer t = new ColumnTransformer();
        Object o = t.getColumnValue(rs, 0);
        assertSame(o, timeInUtc);

        rsControl.verify();        
    }
}
