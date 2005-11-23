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
package org.sipfoundry.sipxconfig.phone;
import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.common.EnumUserType;

public class FirmwareManufacturer extends Enum {
    public static final FirmwareManufacturer UNMANAGED = new FirmwareManufacturer("unmanagedPhone",
            "Unmanaged TFTP", "tftp");

    private String m_label;
    private String m_preferredDeliveryId;
    public FirmwareManufacturer(String manfacturerId) {
        super(manfacturerId);
    }    
    public FirmwareManufacturer(String manfacturerId, String label, String preferredDeliveryId) {
        this(manfacturerId);
        setLabel(label);
        setPreferredDeliveryId(preferredDeliveryId);
    }
    public String getLabel() {
        return m_label;
    }
    public void setLabel(String label) {
        m_label = label;
    }
    public String getManufacturerId() {
        return getName();
    }
    public String getPreferredDeliveryId() {
        return m_preferredDeliveryId;
    }
    public void setPreferredDeliveryId(String preferredDeliveryId) {
        m_preferredDeliveryId = preferredDeliveryId;
    }
    public static class UserType extends EnumUserType {
        public UserType() {
            super(FirmwareManufacturer.class);
        }
    }
}
