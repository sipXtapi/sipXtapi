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
package org.sipfoundry.sipxconfig.components.selection;

import junit.framework.TestCase;

import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.PropertySelection;
import org.easymock.MockControl;
import org.easymock.classextension.MockClassControl;

public class OptGroupPropertySelectionRendererTest extends TestCase {
    private OptGroupPropertySelectionRenderer m_renderer;

    protected void setUp() throws Exception {
        m_renderer = new OptGroupPropertySelectionRenderer();
    }

    public void testBeginRender() {
        MockControl propertySelectionCtrl = MockClassControl
                .createControl(PropertySelection.class);
        PropertySelection propertySelection = (PropertySelection) propertySelectionCtrl.getMock();
        propertySelection.getName();
        propertySelectionCtrl.setReturnValue("bongo");
        propertySelection.isDisabled();
        propertySelectionCtrl.setReturnValue(false);
        propertySelection.getSubmitOnChange();
        propertySelectionCtrl.setReturnValue(false);

        propertySelectionCtrl.replay();

        MockControl writerCtrl = MockControl.createStrictControl(IMarkupWriter.class);
        IMarkupWriter writer = (IMarkupWriter) writerCtrl.getMock();
        writer.begin("select");
        writer.attribute("name", "bongo");
        writer.println();
        writerCtrl.replay();

        m_renderer.beginRender(propertySelection, writer, null);

        writerCtrl.verify();
        propertySelectionCtrl.verify();
    }

    public void testBeginRenderDisabled() {
        MockControl propertySelectionCtrl = MockClassControl
                .createControl(PropertySelection.class);
        PropertySelection propertySelection = (PropertySelection) propertySelectionCtrl.getMock();
        propertySelection.getName();
        propertySelectionCtrl.setReturnValue("bongo");
        propertySelection.isDisabled();
        propertySelectionCtrl.setReturnValue(true);
        propertySelection.getSubmitOnChange();
        propertySelectionCtrl.setReturnValue(true);

        propertySelectionCtrl.replay();

        MockControl writerCtrl = MockControl.createStrictControl(IMarkupWriter.class);
        IMarkupWriter writer = (IMarkupWriter) writerCtrl.getMock();
        writer.begin("select");
        writer.attribute("name", "bongo");
        writer.attribute("disabled", true);
        writer.attribute("onchange", "javascript:this.form.submit();");
        writer.println();
        writerCtrl.replay();

        m_renderer.beginRender(propertySelection, writer, null);

        writerCtrl.verify();
        propertySelectionCtrl.verify();
    }

    public void testRenderOption() {
        MockControl writerCtrl = MockControl.createStrictControl(IMarkupWriter.class);
        IMarkupWriter writer = (IMarkupWriter) writerCtrl.getMock();
        writer.begin("option");
        writer.attribute("value", "1");
        writer.attribute("label", "kuku");
        writer.print("kuku");
        writer.end();
        writer.println();
        writerCtrl.replay();

        String option = "kuku";

        MockControl modelCtrl = MockControl.createControl(IPropertySelectionModel.class);
        IPropertySelectionModel model = (IPropertySelectionModel) modelCtrl.getMock();
        model.getValue(1);
        modelCtrl.setReturnValue("1");
        model.getLabel(1);
        modelCtrl.setReturnValue("kuku");

        modelCtrl.replay();

        m_renderer.renderOption(null, writer, null, model, option, 1, false);

        modelCtrl.verify();
        writerCtrl.verify();
    }

    public void testRenderDisabledAndSelectedOption() {
        MockControl writerCtrl = MockControl.createStrictControl(IMarkupWriter.class);
        IMarkupWriter writer = (IMarkupWriter) writerCtrl.getMock();
        writer.begin("option");
        writer.attribute("value", "1");
        writer.attribute("selected", true);
        writer.attribute("disabled", true);
        writer.attribute("label", "kuku");

        writer.print("kuku");
        writer.end();
        writer.println();
        writerCtrl.replay();

        MockControl modelCtrl = MockControl.createControl(IPropertySelectionModel.class);
        IPropertySelectionModel model = (IPropertySelectionModel) modelCtrl.getMock();
        model.getValue(1);
        modelCtrl.setReturnValue("1");
        model.getLabel(1);
        modelCtrl.setReturnValue("kuku");

        modelCtrl.replay();

        m_renderer.renderOption(null, writer, null, model, null, 1, true);

        modelCtrl.verify();
        writerCtrl.verify();
    }

    public void testRenderOptGroup() {
        OptGroup group = new OptGroup("bongoGroup");
        MockControl writerCtrl = MockControl.createStrictControl(IMarkupWriter.class);
        IMarkupWriter writer = (IMarkupWriter) writerCtrl.getMock();
        // first call to render
        writer.begin("optgroup");
        writer.attribute("label", "bongoGroup");
        writer.println();

        // second call to render
        writer.end();
        writer.begin("optgroup");
        writer.attribute("label", "bongoGroup");
        writer.println();
        writerCtrl.replay();

        m_renderer.renderOption(null, writer, null, null, group, 1, false);
        m_renderer.renderOption(null, writer, null, null, group, 2, false);

        writerCtrl.verify();
    }

    public void testEndRender() {
        MockControl propertySelectionCtrl = MockClassControl
                .createControl(PropertySelection.class);
        PropertySelection propertySelection = (PropertySelection) propertySelectionCtrl.getMock();

        propertySelectionCtrl.replay();

        MockControl writerCtrl = MockControl.createStrictControl(IMarkupWriter.class);
        IMarkupWriter writer = (IMarkupWriter) writerCtrl.getMock();
        writer.end();
        writerCtrl.replay();

        m_renderer.endRender(propertySelection, writer, null);

        writerCtrl.verify();
        propertySelectionCtrl.verify();
    }
}
