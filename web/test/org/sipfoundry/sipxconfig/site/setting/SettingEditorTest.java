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

import junit.framework.TestCase;

import org.apache.tapestry.valid.IValidator;
import org.apache.tapestry.valid.PatternValidator;
import org.apache.tapestry.valid.StringValidator;
import org.sipfoundry.sipxconfig.setting.type.IntegerSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;
import org.sipfoundry.sipxconfig.setting.type.StringSetting;

public class SettingEditorTest extends TestCase {

    public void testValidatorForInteger() {
        SettingType type = new IntegerSetting();
        assertNull(SettingEditor.validatorForType(type));        
    }

    public void testValidatorForString() {
        StringSetting type = new StringSetting();
        type.setMaxLen(15);
        type.setRequired(true);
        IValidator validator = SettingEditor.validatorForType(type);
        assertTrue(validator instanceof StringValidator);
        StringValidator stringValidator = (StringValidator) validator;
        assertEquals(0,stringValidator.getMinimumLength());
        assertTrue(stringValidator.isRequired());
    }

    public void testValidatorForPattern() {
        StringSetting type = new StringSetting();
        type.setMaxLen(15);
        type.setRequired(true);
        type.setPattern("kuku");
        IValidator validator = SettingEditor.validatorForType(type);
        assertTrue(validator instanceof PatternValidator);
        PatternValidator patternValidator = (PatternValidator) validator;
        assertEquals("kuku",patternValidator.getPatternString());
        assertTrue(patternValidator.isRequired());
    }
}
