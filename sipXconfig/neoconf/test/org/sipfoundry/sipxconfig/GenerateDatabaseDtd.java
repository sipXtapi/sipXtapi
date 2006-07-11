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
package org.sipfoundry.sipxconfig;

import java.io.FileOutputStream;

import org.dbunit.database.IDatabaseConnection;
import org.dbunit.dataset.xml.FlatDtdDataSet;
import org.dbunit.dataset.xml.XmlDataSet;

/**
 * Generate a DTD that helps writing dbunit files.  You can run this from eclipse or commandline
 * 
 * NOTE: After you run this, replace first DTD entry w/this
 * 
 * <!ELEMENT dataset ANY>
 */
public class GenerateDatabaseDtd {

    public static void main(String[] args) {
        try {
            if (args.length > 0) {
                TestHelper.initSysDirProperties(args[0]);
            }
            generateDbDtd();
            generateDbXml();
            System.exit(0);
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }

    private static void generateDbDtd() throws Exception {
        IDatabaseConnection c = TestHelper.getConnection();

        FlatDtdDataSet.write(c.createDataSet(), new FileOutputStream(
                "test/org/sipfoundry/sipxconfig/sipxconfig-db.dtd"));
    }

    private static void generateDbXml() throws Exception {
        IDatabaseConnection c = TestHelper.getConnection();

        XmlDataSet.write(c.createDataSet(), new FileOutputStream(
                "test/org/sipfoundry/sipxconfig/sipxconfig-db.xml"));
    }

}
