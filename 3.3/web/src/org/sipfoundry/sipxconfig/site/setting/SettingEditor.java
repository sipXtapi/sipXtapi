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

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.validator.Max;
import org.apache.tapestry.form.validator.MaxLength;
import org.apache.tapestry.form.validator.Min;
import org.apache.tapestry.form.validator.Required;
import org.apache.tapestry.valid.PatternValidator;
import org.sipfoundry.sipxconfig.components.NamedValuesSelectionModel;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.EnumSetting;
import org.sipfoundry.sipxconfig.setting.type.IntegerSetting;
import org.sipfoundry.sipxconfig.setting.type.RealSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;
import org.sipfoundry.sipxconfig.setting.type.StringSetting;

public abstract class SettingEditor extends BaseComponent {
    public abstract Setting getSetting();

    public abstract void setSetting(Setting setting);

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

    /**
     * This is to support new Tapestry4 types of validators
     * 
     * @return list of Validator objects
     */
    public List getValidatorList() {
        SettingType type = getSetting().getType();
        return validatorListForType(type);
    }

    static List validatorListForType(SettingType type) {
        List validators = new ArrayList();
        if (type.isRequired()) {
            validators.add(new Required());
        }
        if (type instanceof IntegerSetting) {
            IntegerSetting integerType = (IntegerSetting) type;
            Min min = new Min();
            min.setMin(integerType.getMin());
            validators.add(min);
            Max max = new Max();
            max.setMax(integerType.getMax());
            validators.add(max);
        }
        if (type instanceof RealSetting) {
            RealSetting realType = (RealSetting) type;
            Min min = new Min();
            min.setMin(realType.getMin());
            validators.add(min);
            Max max = new Max();
            max.setMax(realType.getMax());
            validators.add(max);
        }
        if (type instanceof StringSetting) {
            StringSetting stringType = (StringSetting) type;
            MaxLength maxLen = new MaxLength();
            maxLen.setMaxLength(stringType.getMaxLen());
            validators.add(maxLen);
            String pattern = stringType.getPattern();
            if (StringUtils.isNotEmpty(pattern)) {
                PatternValidator patternValidator = new PatternValidator();
                patternValidator.setPatternString(pattern);
                validators.add(patternValidator);
            }
        }
        return validators;
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

    public boolean isModified() {
        Object val = getSetting().getValue();
        Object def = getSetting().getDefaultValue();
        return val == null ? def != null : !val.equals(def);
    }
}
