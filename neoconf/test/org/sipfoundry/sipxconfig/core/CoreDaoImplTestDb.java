/*
 * Created on Nov 27, 2004
 *
 * TODO To change the template for this generated file go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
package org.sipfoundry.sipxconfig.core;

import junit.framework.TestCase;

import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.beans.factory.access.SingletonBeanFactoryLocator;

/**
 * Requires Database access.
 */
public class CoreDaoImplTestDb extends TestCase {

    private CoreDao m_dao;
    
    private Organization m_testOrganization;
    
    private LogicalPhone m_testPhone;
    
    private User m_testUser;
    
    /*
     * @see TestCase#setUp()
     */
    protected void setUp() throws Exception {
        super.setUp();
        
        BeanFactoryLocator bfl = SingletonBeanFactoryLocator.getInstance();
        BeanFactoryReference bf = bfl.useBeanFactory("unittest-db");
        // now use some bean from factory 
        m_dao = (CoreDao) bf.getFactory().getBean("coreDao");
    }
    
    protected void tearDown() throws Exception {
        if (m_testOrganization != null) {
            m_dao.deleteOrganization(m_testOrganization);
        }
    }

    public void testCreate() {
        assertNotNull(m_dao);
    }
    
    public void testStore() {        
        Organization org = new Organization();
        org.setDnsDomain("coredao.sipfoundry.org");
        org.setName("CoreDaoImplTest");
        m_dao.storeOrganization(org);
        
        m_testOrganization = org;
        
        User user = new User();
        user.setFirstName("First Name");
        user.setOrganization(org);
        user.setPassword("h4ck3rs");        
        m_dao.storeUser(user);
        m_testUser = user;

        LogicalPhone phone = new LogicalPhone();
        phone.setSerialNumber("0000000000000");
        phone.setShortName("CoreDaoImplTest");
        phone.setOrganization(org);
        phone.setUser(user);
        m_dao.storeLogicalPhone(phone);
        m_testPhone = phone;
        
        Line line = new Line();
        line.setName("joe");
        line.setUser(user);
    }
    
    public void _testLoad() {
        
        // create test data
        testStore();
        
        LogicalPhone load = (LogicalPhone)m_dao.loadLogicalPhone(m_testPhone.getId());
        assertNotNull(load);
        assertEquals(load.getId(), m_testPhone.getId());
    }

}
