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
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.URL;
import java.security.CodeSource;
import java.sql.SQLException;
import java.util.Properties;
import java.util.Stack;

import javax.sql.DataSource;

import net.sf.hibernate.HibernateException;
import net.sf.hibernate.Session;
import net.sf.hibernate.SessionFactory;

import org.apache.velocity.app.VelocityEngine;
import org.dbunit.database.DatabaseConnection;
import org.dbunit.database.IDatabaseConnection;
import org.dbunit.dataset.xml.FlatDtdDataSet;
import org.springframework.context.ApplicationContext;
import org.springframework.context.support.ClassPathXmlApplicationContext;

/**
 * For unittests that need spring instantiated
 */
public final class TestHelper {
    
    private static final String APPLICATION_CONTEXT_FILE = "org/sipfoundry/sipxconfig/applicationContext-sipxconfig.xml";
    
    private static Properties s_sysProps;
    
    private static ApplicationContext s_appContext;
    
    private static Stack m_deleteOnTeardown = new Stack();
    
    public static ApplicationContext getApplicationContext() {
        if (s_appContext == null) {
            getSysDirProperties();
            s_appContext = new ClassPathXmlApplicationContext(
                TestHelper.APPLICATION_CONTEXT_FILE);
        }
        
        return s_appContext;
    }
    
    public static String getTestDirectory() {
        return getSysDirProperties().getProperty("sysdir.var");
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
    
    /**
     * Create a sysdir.properties file in the classpath.  Uses a trick that
     * will only work if unittests are unjar-ed.  This is infavor of doing
     * in ant because it avoids setup and works in IDE's like eclipse where
     * bin.eclipse is the classpath
     */
    public static Properties getSysDirProperties() {
        if (s_sysProps == null) {
	        // create file on classpath
	        CodeSource code = TestHelper.class.getProtectionDomain().getCodeSource();
	        URL classpathUrl = code.getLocation();
	        File classpathDir = new File(classpathUrl.getFile());
	        File sysdirPropsFile = new File(classpathDir, "sysdir.properties");
	        FileOutputStream sysdirPropsStream;
	        try {
	            sysdirPropsStream = new FileOutputStream(sysdirPropsFile);
	        } catch (FileNotFoundException e) {
	            throw new RuntimeException("could not create system dir properties file", e);
	        }
	        s_sysProps = new Properties();
	        String userDir = System.getProperty("user.dir");
	        String etcDir = System.getProperty("basedir", userDir) + "/etc";
	        s_sysProps.setProperty("sysdir.etc", etcDir);
	        s_sysProps.setProperty("sysdir.var", classpathDir.getAbsolutePath());
	        s_sysProps.setProperty("sysdir.log", classpathDir.getAbsolutePath());
	        
	        try {
	            // store them so spring's application context file find it
	            // in classpath
	            s_sysProps.store(sysdirPropsStream, null);
	        } catch (IOException e) {
	            throw new RuntimeException("could not store system dir properties", e);
	        }
        }
        
        return s_sysProps;
    }
    
    public static void deleteOnTearDown(Object o) {
        m_deleteOnTeardown.push(o);
    }
    
    /**
     * straight delete, circumvents business DAO
     */
    public static void tearDown() throws HibernateException {
        if (!m_deleteOnTeardown.empty()) {
            SessionFactory sessionFactory = (SessionFactory) getApplicationContext().getBean("sessionFactory");
            Session session = sessionFactory.openSession();
            while (!m_deleteOnTeardown.empty()) {
                session.delete(m_deleteOnTeardown.pop());
            }
            session.flush();
            session.close();
        }
    }
    
    public static IDatabaseConnection getDbUnitConnection() throws SQLException {
        DataSource dataSource = (DataSource) getApplicationContext().getBean("dataSource");
        return new DatabaseConnection(dataSource.getConnection());
    }
    
    public static void main(String[] args) {
        try {
            generateDbUnitDtd();
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }
    
    private static void generateDbUnitDtd() throws Exception {
        IDatabaseConnection c = getDbUnitConnection();
        
        FlatDtdDataSet.write(c.createDataSet(),
            new FileOutputStream("test/org/sipfoundry/sipxconfig/sipxconfig-dataset.dtd"));
        
    }

}
