/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;

public class ConferenceBridgeContextImplTestDb extends TestHelper.TestCaseDb  {

    private ConferenceBridgeContext m_context;
    private CoreContext m_coreContext;

    protected void setUp() throws Exception {
        m_context = (ConferenceBridgeContext) TestHelper.getApplicationContext().getBean(
                ConferenceBridgeContext.CONTEXT_BEAN_NAME);
        m_coreContext = (CoreContext) TestHelper.getApplicationContext().getBean(
                CoreContext.CONTEXT_BEAN_NAME);

        TestHelper.cleanInsert("ClearDb.xml");
    }
    
    /*
     * Test method for 'org.sipfoundry.sipxconfig.conference.ConferenceBridgeContextImpl.getBridges()'
     */
    public void testGetBridges() {

    }

    /*
     * Test method for 'org.sipfoundry.sipxconfig.conference.ConferenceBridgeContextImpl.store(Bridge)'
     */
    public void testStore() {

    }

    /*
     * Test method for 'org.sipfoundry.sipxconfig.conference.ConferenceBridgeContextImpl.removeBridges(Collection)'
     */
    public void testRemoveBridges() {

    }

    /*
     * Test method for 'org.sipfoundry.sipxconfig.conference.ConferenceBridgeContextImpl.removeConferences(Collection)'
     */
    public void testRemoveConferences() {

    }

    /*
     * Test method for 'org.sipfoundry.sipxconfig.conference.ConferenceBridgeContextImpl.removeParticipants(Collection)'
     */
    public void testRemoveParticipants() {

    }

    /*
     * Test method for 'org.sipfoundry.sipxconfig.conference.ConferenceBridgeContextImpl.addParticipantsToConference(Serializable, Collection)'
     */
    public void testAddParticipantsToConference() {

    }

    /*
     * Test method for 'org.sipfoundry.sipxconfig.conference.ConferenceBridgeContextImpl.loadBridge(Serializable)'
     */
    public void testLoadBridge() {

    }

    /*
     * Test method for 'org.sipfoundry.sipxconfig.conference.ConferenceBridgeContextImpl.loadConference(Serializable)'
     */
    public void testLoadConference() {

    }

    /*
     * Test method for 'org.sipfoundry.sipxconfig.conference.ConferenceBridgeContextImpl.loadParticipant(Serializable)'
     */
    public void testLoadParticipant() {

    }

    /*
     * Test method for 'org.sipfoundry.sipxconfig.conference.ConferenceBridgeContextImpl.clear()'
     */
    public void testClear() {

    }
}
