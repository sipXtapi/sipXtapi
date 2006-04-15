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
import org.apache.tapestry.IComponent;
import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.validator.Max;
import org.apache.tapestry.form.validator.MaxLength;
import org.apache.tapestry.form.validator.Min;
import org.apache.tapestry.form.validator.Pattern;
import org.apache.tapestry.form.validator.Required;
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
    
    public abstract boolean isRequiredEnabled();

    /**
     * This is to support new Tapestry4 types of validators
     * 
     * @return list of Validator objects
     */
    public List getValidatorList() {
        SettingType type = getSetting().getType();
        return validatorListForType(type, isRequiredEnabled());
    }

    static List validatorListForType(SettingType type, boolean isRequiredEnabled) {
        List validators = new ArrayList();
        if (type.isRequired() && isRequiredEnabled) {
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
            String patternString = stringType.getPattern();
            if (StringUtils.isNotEmpty(patternString)) {
                Pattern pattern = new Pattern();
                pattern.setPattern(patternString);
                validators.add(pattern);
            }
        }
        return validators;
    }

    /**
     * In order to make FieldLabel happy we need IFormComponent instance. In most cases this is
     * actually the widget directly passed to setting editor. However in some cases our widget is
     * a collection of components, and there is no easy way of extracting a usable IFormElement.
     * 
     * I tried searching for IFormComponent among component children: this does not work since
     * there is no guarantee that found component will be actually renderer (it can be inside of
     * if block)
     * 
     * @return IFormComponent or null if labeled component is not IFormComponent
     */
    public IFormComponent getFormComponent() {
        String componentName = getSetting().getType().getName();
        IComponent component = getComponent(componentName + "Field");
        if (component instanceof IFormComponent) {
            return (IFormComponent) component;
        }
        return null;
    }

    public IPropertySelectionModel getEnumModel() {
        SettingType type = getSetting().getType();
        return enumModelForType(type);
    }

    public String getDefaultValue() {
        Setting setting = getSetting();
        SettingType type = setting.getType();
        String label = type.getLabel(setting.getDefaultValue());
        return label;
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
