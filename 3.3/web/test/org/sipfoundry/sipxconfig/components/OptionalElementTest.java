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
package org.sipfoundry.sipxconfig.components;

import junit.framework.TestCase;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.tapestry.IBinding;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.spec.IComponentSpecification;
import org.apache.tapestry.test.Creator;
import org.easymock.MockControl;

public class OptionalElementTest extends TestCase {
    private Creator m_maker = new Creator();
    private OptionalElement m_oe;

    protected void setUp() throws Exception {
        m_oe = (OptionalElement) m_maker.newInstance(OptionalElement.class);
    }

    public void testRender() throws Exception {
        MockControl mcCycle = MockControl.createControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) mcCycle.getMock();
        cycle.isRewinding();
        mcCycle.setReturnValue(false, MockControl.ONE_OR_MORE);
        mcCycle.replay();
        
        MockControl mcWriter = MockControl.createControl(IMarkupWriter.class);
        IMarkupWriter writer = (IMarkupWriter) mcWriter.getMock();

        mcWriter.replay();

        m_oe.setElement("");
        m_oe.renderComponent(writer, cycle);

        mcCycle.verify();
        mcWriter.verify();
    }

    public void testRenderWithElement() throws Exception {
        MockControl mcBinding = MockControl.createControl(IBinding.class);
        IBinding binding = (IBinding) mcBinding.getMock();
        binding.getObject();
        mcBinding.setDefaultReturnValue("kuku");
        mcBinding.replay();

        MockControl mcCycle = MockControl.createControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) mcCycle.getMock();
        cycle.isRewinding();
        mcCycle.setReturnValue(false, MockControl.ONE_OR_MORE);
        mcCycle.replay();

        MockControl mcWriter = MockControl.createStrictControl(IMarkupWriter.class);
        IMarkupWriter writer = (IMarkupWriter) mcWriter.getMock();
        writer.begin("bongo");
        writer.attribute("attr1", "kuku");
        writer.end("bongo");
        mcWriter.replay();
        
        MockControl mcComponentSpec = MockControl.createNiceControl(IComponentSpecification.class);
        IComponentSpecification componentSpec = (IComponentSpecification) mcComponentSpec.getMock();
        mcComponentSpec.replay();
        // method available on proxy object, See Creator.java
        BeanUtils.setProperty(m_oe, "specification", componentSpec);

        m_oe.setBinding("attr1", binding);
        m_oe.setElement("bongo");
        m_oe.renderComponent(writer, cycle);

        mcBinding.verify();
        mcCycle.verify();
        mcWriter.verify();
        mcComponentSpec.verify();
    }
}
