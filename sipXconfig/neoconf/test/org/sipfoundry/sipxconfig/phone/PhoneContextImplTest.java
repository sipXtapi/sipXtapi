package org.sipfoundry.sipxconfig.phone;

import java.util.Arrays;
import java.util.Collection;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.phone.cisco.CiscoModel;
import org.sipfoundry.sipxconfig.phone.grandstream.GrandstreamModel;
import org.sipfoundry.sipxconfig.phone.kphone.KPhone;
import org.sipfoundry.sipxconfig.phone.polycom.PolycomModel;

public class PhoneContextImplTest extends TestCase {
    Collection m_models;
    PhoneContextImpl m_impl;
    
    protected void setUp() {
        m_models = Arrays.asList(new PhoneModel[] { 
                Phone.MODEL, PolycomModel.MODEL_300, CiscoModel.MODEL_7960, 
                GrandstreamModel.MODEL_HT496, KPhone.MODEL_KPHONE
        });        
        m_impl = new PhoneContextImpl();
        m_impl.setAvailablePhoneModels(m_models);
    }
    
    public void testGetAvailablePhones() {
        assertSame(m_models, m_impl.getAvailablePhoneModels());
    }
    
    public void testGetCertifiedPhones() {
        m_impl.setCertifiedPhones("^(polycom.*)$");
        Collection certified = m_impl.getAvailablePhoneModels();
        assertEquals(1, certified.size());
        assertSame(PolycomModel.MODEL_300, certified.iterator().next());
    }
    
    public void testRemoveUnCertifiedPhones() {
        m_impl.setCertifiedPhones("^(?!(grandstream|kphone)).*$");
        Collection certified = m_impl.getAvailablePhoneModels();        
        assertEquals(3, certified.size());
        assertFalse(certified.contains(KPhone.MODEL_KPHONE));
        assertFalse(certified.contains(GrandstreamModel.MODEL_HT496));
        assertTrue(certified.contains(PolycomModel.MODEL_300));
        
    }
}
