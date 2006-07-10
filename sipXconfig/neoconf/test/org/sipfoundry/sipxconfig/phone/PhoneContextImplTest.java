package org.sipfoundry.sipxconfig.phone;

import java.util.Arrays;
import java.util.Collection;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.phone.cisco.CiscoModel;
import org.sipfoundry.sipxconfig.phone.grandstream.GrandstreamModel;
import org.sipfoundry.sipxconfig.phone.kphone.KPhone;
import org.sipfoundry.sipxconfig.phone.polycom.PolycomModel;

public class PhoneContextImplTest extends TestCase {
    private Collection m_models;
    private PhoneContextImpl m_impl;
    private ModelSource m_modelSource;

    protected void setUp() {
        m_models = Arrays.asList(new PhoneModel[] {
            Phone.MODEL, PolycomModel.MODEL_300, CiscoModel.MODEL_7960,
            GrandstreamModel.MODEL_HT496, KPhone.MODEL_KPHONE
        });
        m_modelSource = EasyMock.createMock(ModelSource.class);
        m_modelSource.getModels();
        EasyMock.expectLastCall().andReturn(m_models).anyTimes();
        EasyMock.replay(m_modelSource);
        m_impl = new PhoneContextImpl();
        m_impl.setPhoneModelSource(m_modelSource);
    }

    protected void tearDown() throws Exception {
        EasyMock.verify(m_modelSource);
    }

    // TODO: this is probably a good candidate for removal - all nontrivial tests are ow in
    // FilteredModelSourceTest
    public void testGetAvailablePhones() {
        assertSame(m_models, m_impl.getAvailablePhoneModels());
    }
}
