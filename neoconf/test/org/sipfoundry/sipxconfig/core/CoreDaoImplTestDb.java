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
    
    private Endpoint m_testPhone;
    
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
        assertNotNull(m_dao);
    }
    
    public void testStore() {  
        
        Organization org = m_dao.loadOrganization(1);
        assertEquals(1, org.getId());
        
        User user = new User();
        user.setFirstName("First Name");
        user.setOrganization(org);
        user.setPassword("h4ck3rs");        
        m_dao.storeUser(user);

        Endpoint endpoint = new Endpoint();
        endpoint.setSerialNumber("0000000000000");
        endpoint.setName("CoreDaoImplTest");
        endpoint.setUser(user);
        m_dao.storeEndpoint(endpoint);
        
        Line line = new Line();
        line.setName("joe");
        line.setUser(user);
        
        // TODO 
        //m_dao.delete(user);
        //m_dao.delete(endpoint);
        
        // TODO check they are truly deleted
    }
}
