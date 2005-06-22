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

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.sql.Connection;
import java.sql.SQLException;
import java.util.Properties;

import javax.sql.DataSource;

import junit.framework.TestCase;

import org.apache.commons.io.CopyUtils;
import org.apache.commons.io.IOUtils;
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
            s_appContext = new ClassPathXmlApplicationContext(TestUtil.APPLICATION_CONTEXT_FILES);
        }

        return s_appContext;
    }

    public static String getClasspathDirectory() {
        return TestUtil.getClasspathDirectory(TestHelper.class);
    }

    public static VelocityEngine getVelocityEngine() {
        
        try {
            Properties sysdir = getSysDirProperties();
    
            String etcDir = sysdir.getProperty("sysdir.etc");
    
            VelocityEngine engine = new VelocityEngine();
            engine.setProperty("resource.loader", "file");
            engine.setProperty("file.resource.loader.path", etcDir);
            engine.init();
    
            return engine;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public static String getTestDirectory() {
        return TestUtil.getTestOutputDirectory("neoconf");
    }

    public static Properties getSysDirProperties() {
        if (s_sysDirProps == null) {
            String etcDir = TestUtil.getProjectDirectory() + "/etc";
            String outDir = getTestDirectory();
            s_sysDirProps = new Properties(); 
            TestUtil.setSysDirProperties(s_sysDirProps, etcDir, outDir);
            TestUtil.saveSysDirProperties(s_sysDirProps, getClasspathDirectory());
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
        try {
            DatabaseOperation.CLEAN_INSERT.execute(getConnection(), loadDataSet(resource));
        }
        catch (SQLException e){
            throw e.getNextException();
        }
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

    /**
     * Special TestCase class that catches prints additional info for SQL Exceptions errors that
     * may happed during setUp, testXXX and tearDown.
     * 
     * Alternatively we could just throw e.getNextException, but we may want to preserve the
     * original exception.
     */
    public static class TestCaseDb extends TestCase {
        public void runBare() throws Throwable {
            try {
                super.runBare();
            } catch (SQLException e) {
                System.err.println(e.getNextException().getMessage());
                throw e;

            }
        }
    }
    
    /**
     * Use in test to create copy of example files to be changed by test methods.
     * @param from input stream
     * @param dir destination directory
     * @param filename destination file name
     * @throws IOException
     */
    public static final void copyStreamToDirectory(InputStream from, String dir, String filename) throws IOException {
        FileOutputStream to = new FileOutputStream(new File(dir, filename));
        CopyUtils.copy(from, to);
        IOUtils.closeQuietly(to);
        IOUtils.closeQuietly(from);
    }    
}
