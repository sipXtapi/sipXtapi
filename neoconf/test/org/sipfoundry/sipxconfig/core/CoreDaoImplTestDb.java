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
    
    private Endpoint m_teardownEndpoint;
    
    private EndpointAssignment m_teardownAssignment;

    private UnitTestDao m_testData;       
    
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
        
        m_testData = (UnitTestDao) bf.getFactory().getBean("testData");
        assertNotNull(m_testData);
        assertTrue(m_testData.initializeImmutableData());
    }
    
    protected void tearDown() throws Exception {
        if (m_teardownEndpoint != null) {
            m_dao.deleteEndpointAssignment(m_teardownAssignment);
        }
        if (m_teardownEndpoint != null) {
            m_dao.deleteEndpoint(m_teardownEndpoint);
        }
        assertTrue(m_testData.verifyDataUnaltered());
    }
    
    public void testStore() {  
        
        Organization org = m_dao.loadRootOrganization();
        assertEquals(1, org.getId());
                       
        User user = m_dao.loadUser(m_testData.getTestUserId());

        Endpoint endpoint = new Endpoint();
        // assumption that this is unique
        endpoint.setSerialNumber("f34298760024fcc1"); 
        endpoint.setPhoneId("unittest - CoreDaoImplTestDb");
        m_dao.storeEndpoint(endpoint);
        m_teardownEndpoint = endpoint;
        
        EndpointAssignment assignment = new EndpointAssignment();
        assignment.setUser(user);
        assignment.setEndpoint(endpoint);
        assignment.setLabel("work phone");
        m_dao.storeEndpointAssignment(assignment);                
        m_teardownAssignment = assignment;
    }
}
