/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig;

import java.io.FileOutputStream;
import java.io.InputStream;
import java.sql.Connection;
import java.util.Properties;

import javax.sql.DataSource;

import org.apache.velocity.app.VelocityEngine;
import org.dbunit.database.DatabaseConfig;
import org.dbunit.database.DatabaseConnection;
import org.dbunit.database.IDatabaseConnection;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.xml.FlatDtdDataSet;
import org.dbunit.dataset.xml.FlatXmlDataSet;
import org.dbunit.dataset.xml.XmlDataSet;
import org.dbunit.operation.DatabaseOperation;
import org.sipfoundry.sipxconfig.common.TestUtil;
import org.springframework.context.ApplicationContext;
import org.springframework.context.support.ClassPathXmlApplicationContext;

/**
 * For unittests that need spring instantiated
 */
public final class TestHelper {

    private static Properties s_sysDirProps;

    private static ApplicationContext s_appContext;

    private static DatabaseConnection s_dbunitConnection;

    static {
        // default XML parser (crimson) cannot resolve relative DTDs, google for bug
        System.setProperty("org.xml.sax.driver", "org.apache.xerces.parsers.SAXParser");
    }

    public static ApplicationContext getApplicationContext() {
        if (s_appContext == null) {
            getSysDirProperties();
            s_appContext = new ClassPathXmlApplicationContext(TestUtil.APPLICATION_CONTEXT_FILE);
        }

        return s_appContext;
    }

    public static String getClasspathDirectory() {
        return TestUtil.getClasspathDirectory(TestHelper.class);
    }

    public static VelocityEngine getVelocityEngine() throws Exception {
        Properties sysdir = getSysDirProperties();

        String etcDir = sysdir.getProperty("sysdir.etc");

        VelocityEngine engine = new VelocityEngine();
        engine.setProperty("resource.loader", "file");
        engine.setProperty("file.resource.loader.path", etcDir);
        engine.init();

        return engine;
    }

    public static String getTestDirectory() {
        return getClasspathDirectory() + "/test-output";
    }

    public static Properties getSysDirProperties() {
        if (s_sysDirProps == null) {
            String etcDir = TestUtil.getProjectDirectory() + "/etc";
            String outDir = getTestDirectory();
            s_sysDirProps = TestUtil.getSysDirProperties(getClasspathDirectory(), etcDir, outDir);
        }
        return s_sysDirProps;
    }

    public static IDatabaseConnection getConnection() throws Exception {
        if (null != s_dbunitConnection) {
            return s_dbunitConnection;
        }
        Class.forName("com.p6spy.engine.spy.P6SpyDriver");

        DataSource ds = (DataSource) getApplicationContext().getBean("dataSource");
        Connection jdbcConnection = ds.getConnection();
        s_dbunitConnection = new DatabaseConnection(jdbcConnection);
        DatabaseConfig config = s_dbunitConnection.getConfig();
        config.setFeature("http://www.dbunit.org/features/batchedStatements", true);

        return s_dbunitConnection;
    }

    public static void main(String[] args) {
        try {
            generateDbDtd();
            generateDbXml();
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }

    private static void generateDbDtd() throws Exception {
        IDatabaseConnection c = getConnection();

        FlatDtdDataSet.write(c.createDataSet(), new FileOutputStream(
                "test/org/sipfoundry/sipxconfig/sipxconfig-dataset.dtd"));
    }

    private static void generateDbXml() throws Exception {
        IDatabaseConnection c = getConnection();

        XmlDataSet.write(c.createDataSet(), new FileOutputStream(
                "test/org/sipfoundry/sipxconfig/sipxconfig-dataset.xml"));
    }

    public static IDataSet loadDataSet(String fileResource) throws Exception {
        InputStream datasetStream = TestHelper.class.getResourceAsStream(fileResource);
        return new XmlDataSet(datasetStream);
    }

    public static IDataSet loadDataSetFlat(String resource) throws Exception {
        InputStream datasetStream = TestHelper.class.getResourceAsStream(resource);
        return new FlatXmlDataSet(datasetStream);
    }

    public static void cleanInsert(String resource) throws Exception {
        DatabaseOperation.CLEAN_INSERT.execute(getConnection(), loadDataSet(resource));
    }

    public static void cleanInsertFlat(String resource) throws Exception {
        DatabaseOperation.CLEAN_INSERT.execute(getConnection(), loadDataSetFlat(resource));
    }

    public static void insertFlat(String resource) throws Exception {
        DatabaseOperation.INSERT.execute(getConnection(), loadDataSetFlat(resource));
    }

    public static void update(String resource) throws Exception {
        DatabaseOperation.UPDATE.execute(getConnection(), loadDataSet(resource));
    }
}
