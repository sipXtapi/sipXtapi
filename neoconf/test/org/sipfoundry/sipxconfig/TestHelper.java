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
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;
import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.context.ApplicationContext;
import org.springframework.context.access.ContextSingletonBeanFactoryLocator;
import org.springframework.dao.DataIntegrityViolationException;

/**
 * For unit tests that need spring instantiated
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
            BeanFactoryLocator bfl = ContextSingletonBeanFactoryLocator.getInstance();
            BeanFactoryReference bfr = bfl.useBeanFactory("servicelayer-context");
            s_appContext = (ApplicationContext) bfr.getFactory();
        }

        return s_appContext;
    }

    public static Setting loadSettings(String path) {
        String sysdir = getSysDirProperties().getProperty("sysdir.etc");
        XmlModelBuilder builder = new XmlModelBuilder(sysdir);
        Setting settings = builder.buildModel(new File(sysdir + "/" + path));

        return settings;
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
            String classpathDirectory = getClasspathDirectory();
            s_sysDirProps = initSysDirProperties(classpathDirectory);
        }
        return s_sysDirProps;
    }

    public static Properties initSysDirProperties(String dir) {
        String etcDir = TestUtil.getProjectDirectory() + "/etc";
        String outDir = getTestDirectory();
        Properties props = new Properties();
        TestUtil.setSysDirProperties(props, etcDir, outDir);
        TestUtil.saveSysDirProperties(props, dir);
        return props;
    }

    public static IDatabaseConnection getConnection() throws Exception {
        if (s_dbunitConnection != null) {
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

    public static void closeConnection() throws SQLException {
        if (s_dbunitConnection != null && !s_dbunitConnection.getConnection().isClosed()) {
            s_dbunitConnection.close();
            s_dbunitConnection = null;
        }        
    }
    
    public static void main(String[] args) {
        try {
            if (args.length > 0) {
                s_sysDirProps = initSysDirProperties(args[0]);
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
        IDatabaseConnection c = getConnection();

        FlatDtdDataSet.write(c.createDataSet(), new FileOutputStream(
                "test/org/sipfoundry/sipxconfig/sipxconfig-db.dtd"));
    }

    private static void generateDbXml() throws Exception {
        IDatabaseConnection c = getConnection();

        XmlDataSet.write(c.createDataSet(), new FileOutputStream(
                "test/org/sipfoundry/sipxconfig/sipxconfig-db.xml"));
    }

    public static IDataSet loadDataSet(String fileResource) throws Exception {
        InputStream datasetStream = TestHelper.class.getResourceAsStream(fileResource);
        return new XmlDataSet(datasetStream);
    }

    public static IDataSet loadDataSetFlat(String resource) throws Exception {
        InputStream datasetStream = TestHelper.class.getResourceAsStream(resource);
        // we do not want to use metadata since it DBTestUnit resolves relative DTDs incorrectly
        // we are checking XML validity in separate Ant tasks (test-dataset) 
        return new FlatXmlDataSet(datasetStream, false);
    }

    public static void cleanInsert(String resource) throws Exception {
        try {
            DatabaseOperation.CLEAN_INSERT.execute(getConnection(), loadDataSet(resource));
        } catch (SQLException e) {
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
                dumpSqlExceptionMessages(e);
                throw e;
            } catch (DataIntegrityViolationException e) {
                if (e.getCause() instanceof SQLException) {
                    dumpSqlExceptionMessages((SQLException) e.getCause());
                }
                throw e;
            }
        }

        private void dumpSqlExceptionMessages(SQLException e) {
            for (SQLException next = e; next != null; next = next.getNextException()) {
                System.err.println(next.getMessage());
            }
        }
    }
    
    /**
     * Use in test to create copy of example files to be changed by test methods.
     * 
     * @param from input stream
     * @param dir destination directory
     * @param filename destination file name
     * @throws IOException
     */
    public static final void copyStreamToDirectory(InputStream from, String dir, String filename)
            throws IOException {
        FileOutputStream to = new FileOutputStream(new File(dir, filename));
        CopyUtils.copy(from, to);
        IOUtils.closeQuietly(to);
        IOUtils.closeQuietly(from);
    }
}
