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
package org.sipfoundry.sipxconfig.upload;
import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.common.EnumUserType;

public class UploadSpecification extends Enum {

    public static final UploadSpecification UNMANAGED = new UploadSpecification("unmanagedPhone",
            "Unmanaged TFTP", "tftp");

    private String m_label;
    private String m_preferredDeliveryId;
    private UploadDestination m_fileDelivery;
    public UploadSpecification(String manfacturerId) {
        super(manfacturerId);
    }    
    public UploadSpecification(String manfacturerId, String label, String preferredDeliveryId) {
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
            super(UploadSpecification.class);
        }
    }
    public static UploadSpecification getManufacturerById(String id) {
        return (UploadSpecification) Enum.getEnum(UploadSpecification.class, id);
    }
    public void setFileDelivery(UploadDestination fileDelivery) {
        m_fileDelivery = fileDelivery;        
    }
    public UploadDestination getFileDelivery() {
        return m_fileDelivery;
    }
}
