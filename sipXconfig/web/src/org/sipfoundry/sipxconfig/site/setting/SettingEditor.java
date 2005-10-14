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

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.valid.IValidator;
import org.apache.tapestry.valid.PatternValidator;
import org.apache.tapestry.valid.StringValidator;
import org.sipfoundry.sipxconfig.components.NamedValuesSelectionModel;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.BooleanSetting;
import org.sipfoundry.sipxconfig.setting.type.EnumSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;
import org.sipfoundry.sipxconfig.setting.type.StringSetting;

public abstract class SettingEditor extends BaseComponent {
    public abstract Setting getSetting();

    public abstract void setSetting(Setting setting);

    public boolean getBooleanValue() {
        BooleanSetting type = (BooleanSetting) getSetting().getType();
        return type.getTrueValue().equals(getSetting().getValue());
    }

    public void setBooleanValue(boolean value) {
        BooleanSetting type = (BooleanSetting) getSetting().getType();
        getSetting().setValue(value ? type.getTrueValue() : type.getFalseValue());
    }

    public String getStringValue() {
        return getSetting().getValue();
    }

    public void setStringValue(String value) {
        String cleanValue = value;
        if (StringUtils.isEmpty(value)) {
            cleanValue = null;
        }
        getSetting().setValue(cleanValue);
    }

    public IValidator getValidator() {
        SettingType type = getSetting().getType();
        return validatorForType(type);
    }

    public IPropertySelectionModel getEnumModel() {
        SettingType type = getSetting().getType();
        return enumModelForType(type);
    }

    public String getDefaultValue() {
        SettingType type = getSetting().getType();
        if (type instanceof StringSetting) {
            StringSetting stringType = (StringSetting) type;
            if (stringType.isPassword()) {
                return null;
            }
        }
        return getSetting().getDefaultValue();
    }

    static IPropertySelectionModel enumModelForType(SettingType type) {
        if (!(type instanceof EnumSetting)) {
            return null;
        }
        EnumSetting enumType = (EnumSetting) type;
        return new NamedValuesSelectionModel(enumType.getEnums());
    }

    /**
     * Creates validator object based on information in setting type
     * 
     * Validator is only required for types that use ValidField as editor.
     * 
     * @param type
     * @return newly created validator
     */
    static IValidator validatorForType(SettingType type) {
        if (!(type instanceof StringSetting)) {
            // no validator needed for integer type
            return null;
        }
        StringSetting stringType = (StringSetting) type;
        if (null != stringType.getPattern()) {
            PatternValidator validator = new PatternValidator();
            validator.setPatternString(stringType.getPattern());
            validator.setRequired(stringType.isRequired());
            return validator;
        }
        StringValidator validator = new StringValidator();
        validator.setRequired(stringType.isRequired());
        return validator;
    }
    
    public boolean isModified() {
        Object val = getSetting().getValue();
        Object def = getSetting().getDefaultValue();
        return val == null ? def != null : !val.equals(def);
    }
}
