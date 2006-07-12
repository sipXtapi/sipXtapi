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
package org.sipfoundry.sipxconfig.device;

import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.collections.Predicate;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.cisco.CiscoModel;
import org.sipfoundry.sipxconfig.phone.grandstream.GrandstreamModel;
import org.sipfoundry.sipxconfig.phone.kphone.KPhone;
import org.sipfoundry.sipxconfig.phone.polycom.PolycomModel;

public class FilteredModelSourceTest extends TestCase {

    private List<PhoneModel> m_models;
    private FilteredModelSource m_modelSource;
    private ModelSource m_mockSource;

    protected void setUp() throws Exception {
        m_models = Arrays.asList(new PhoneModel[] {
            Phone.MODEL, PolycomModel.MODEL_300, CiscoModel.MODEL_7960,
            GrandstreamModel.MODEL_HT496, KPhone.MODEL_KPHONE
        });
        m_mockSource = EasyMock.createMock(ModelSource.class);
        m_mockSource.getModels();
        EasyMock.expectLastCall().andReturn(m_models).anyTimes();
        EasyMock.replay(m_mockSource);

        m_modelSource = new FilteredModelSource();
        m_modelSource.setModelSource(m_mockSource);
    }

    protected void tearDown() throws Exception {
        EasyMock.verify(m_mockSource);
    }

    public void testGetAvailablePhoneModels() {
        assertSame(m_models, m_modelSource.getModels());
    }

    public void testGetCertifiedPhones() {
        m_modelSource.setCertified("^(polycom.*)$");
        Collection certified = m_modelSource.getModels();
        assertEquals(1, certified.size());
        assertSame(PolycomModel.MODEL_300, certified.iterator().next());
    }

    public void testRemoveUnCertifiedPhones() {
        m_modelSource.setCertified("^(?!(grandstream|kphone)).*$");
        Collection certified = m_modelSource.getModels();
        assertEquals(3, certified.size());
        assertFalse(certified.contains(KPhone.MODEL_KPHONE));
        assertFalse(certified.contains(GrandstreamModel.MODEL_HT496));
        assertTrue(certified.contains(PolycomModel.MODEL_300));
    }
    
    public void testSetFilter() {
        Predicate onlyKPhone = new Predicate() {
            public boolean evaluate(Object object) {
                return object.equals(KPhone.MODEL_KPHONE);
            }
        };
        m_modelSource.setFilter(onlyKPhone);
        Collection certified = m_modelSource.getModels();
        assertEquals(1, certified.size());
        assertTrue(certified.contains(KPhone.MODEL_KPHONE));        
    }
}
