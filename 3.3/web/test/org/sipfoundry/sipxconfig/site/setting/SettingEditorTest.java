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
package org.sipfoundry.sipxconfig.site.setting;

import java.util.List;

import junit.framework.TestCase;

import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.validator.Max;
import org.apache.tapestry.form.validator.MaxLength;
import org.apache.tapestry.form.validator.Min;
import org.apache.tapestry.form.validator.Pattern;
import org.apache.tapestry.form.validator.Required;
import org.apache.tapestry.test.Creator;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.EnumSetting;
import org.sipfoundry.sipxconfig.setting.type.IntegerSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;
import org.sipfoundry.sipxconfig.setting.type.StringSetting;

public class SettingEditorTest extends TestCase {
    
    private SettingEditor m_editor;

    protected void setUp() throws Exception {
        m_editor = (SettingEditor) new Creator().newInstance(SettingEditor.class);
    }

    public void testValidatorForInteger() {
        IntegerSetting type = new IntegerSetting();
        List validators = SettingEditor.validatorListForType(type);
        assertEquals(2, validators.size());
        assertTrue(validators.get(0) instanceof Min);        
        assertTrue(validators.get(1) instanceof Max);        
    }

    public void testValidatorForString() {
        StringSetting type = new StringSetting();
        type.setMaxLen(15);
        type.setRequired(true);
        List validators = SettingEditor.validatorListForType(type);
        assertEquals(2, validators.size());
        assertTrue(validators.get(0) instanceof Required);        
        assertTrue(validators.get(1) instanceof MaxLength);        
    }

    public void testValidatorForPattern() {
        StringSetting type = new StringSetting();
        type.setMaxLen(15);
        type.setRequired(true);
        type.setPattern("kuku");
        List validators = SettingEditor.validatorListForType(type);
        assertEquals(3, validators.size());
        assertTrue(validators.get(0) instanceof Required);        
        assertTrue(validators.get(1) instanceof MaxLength);        
        assertTrue(validators.get(2) instanceof Pattern);        
    }

    public void testEnumModelForType() {
        final String[][] V2L = {
            {
                "0", "Zero"
            }, {
                "something", "XXX"
            }, {
                "no_label_value", "no_label_value"
            }
        };
        EnumSetting type = new EnumSetting();
        for (int i = 0; i < V2L.length; i++) {
            type.addEnum(V2L[i][0], V2L[i][1]);
        }
        IPropertySelectionModel model = SettingEditor.enumModelForType(type);
        assertEquals(V2L.length, model.getOptionCount());
        assertEquals(V2L[1][0], model.getOption(1));
        assertEquals(V2L[2][1], model.getLabel(2));
    }

    public void testEnumModelForInvalidType() {
        IPropertySelectionModel model = SettingEditor.enumModelForType(new StringSetting());
        assertNull(model);
    }

    
    public void testGetDefaultValue() {
        SettingType type = new IntegerSetting();
        
        MockControl settingCtrl = MockControl.createControl(Setting.class);
        Setting setting = (Setting) settingCtrl.getMock();        
        settingCtrl.expectAndReturn(setting.getDefaultValue(), "bongo");
        settingCtrl.expectAndReturn(setting.getType(), type);        
        settingCtrl.replay();
        
        m_editor.setSetting(setting);
        
        assertEquals("bongo", m_editor.getDefaultValue());

        settingCtrl.verify();
    }
    
    public void testGetDefaultValueForString() {
        StringSetting type = new StringSetting();
        type.setPassword(false);
        
        MockControl settingCtrl = MockControl.createControl(Setting.class);
        Setting setting = (Setting) settingCtrl.getMock();        
        settingCtrl.expectAndReturn(setting.getDefaultValue(), "bongo");
        settingCtrl.expectAndReturn(setting.getType(), type);        
        settingCtrl.replay();
        
        m_editor.setSetting(setting);
        
        assertEquals("bongo", m_editor.getDefaultValue());

        settingCtrl.verify();
    }

    public void testGetDefaultValueForPassword() {
        StringSetting type = new StringSetting();
        type.setPassword(true);
        
        MockControl settingCtrl = MockControl.createControl(Setting.class);
        Setting setting = (Setting) settingCtrl.getMock();        
        settingCtrl.expectAndReturn(setting.getType(), type);        
        settingCtrl.replay();
        
        m_editor.setSetting(setting);
        
        assertNull(m_editor.getDefaultValue());

        settingCtrl.verify();
    }
}
