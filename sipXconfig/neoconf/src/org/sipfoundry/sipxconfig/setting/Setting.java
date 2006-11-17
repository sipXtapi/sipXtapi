/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.Collection;

import org.sipfoundry.sipxconfig.setting.type.SettingType;
import org.springframework.context.MessageSource;

/**
 * Base class for all items describing and using setting.
 */
public interface Setting extends Cloneable {

    // public static final String NULL_VALUE = StringUtils.EMPTY;

    public static final char PATH_DELIM = '/';

    public Setting getParent();

    public void setParent(Setting parent);

    public String getPath();

    public Setting addSetting(Setting setting);

    public Setting getSetting(String name);

    @Deprecated
    public String getLabel();

    public String getLabelKey();

    public String getName();

    public void setName(String name);

    public String getProfileName();

    /**
     * Full profile path of the setting - including profile names of all parents and profile name
     * of this setting
     */
    public String getProfilePath();

    /**
     * what would value be if it wasn't set., most implementation this is your the value from the
     * setting you decorate. NOTE: no setter because this is a "computed" value based on chain of
     * decorated setting values.
     */
    public String getDefaultValue();

    public String getValue();

    /**
     * @return the value of this setting coerced to the proper type
     */
    public Object getTypedValue();

    public void setTypedValue(Object value);

    public void setValue(String value);

    public SettingType getType();

    public void setType(SettingType type);

    @Deprecated
    public String getDescription();

    public String getDescriptionKey();

    public Collection<Setting> getValues();

    public void acceptVisitor(SettingVisitor visitor);

    public boolean isAdvanced();

    public boolean isHidden();

    public Setting copy();

    public MessageSource getMessageSource();
}
