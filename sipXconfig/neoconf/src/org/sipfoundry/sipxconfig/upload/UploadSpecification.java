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
import java.util.List;

import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.common.BeanEnum;
import org.sipfoundry.sipxconfig.common.EnumUserType;


/**
 * One UploadSpecification instance for all types of uploads
 */
public class UploadSpecification extends BeanEnum {

    public static final UploadSpecification UNMANAGED = new UploadSpecification("upload", 
            "unmanagedPhone", "Unmanaged TFTP");
    
    public UploadSpecification(String beanId, String label) {
        super(beanId, label);
    }
    
    public UploadSpecification(String beanId, String specificationId, String label) {
        super(beanId, specificationId, label);
    }
    
    public String getSpecificationId() {
        return getEnumId();
    }
    
    public static class UserType extends EnumUserType {
        public UserType() {
            super(UploadSpecification.class);
        }
    }
    
    public static UploadSpecification getSpecificationById(String id) {
        return (UploadSpecification) Enum.getEnum(UploadSpecification.class, id);
    }
    
    public static List getSpecifications() {
        return Enum.getEnumList(UploadSpecification.class);
    }
}
