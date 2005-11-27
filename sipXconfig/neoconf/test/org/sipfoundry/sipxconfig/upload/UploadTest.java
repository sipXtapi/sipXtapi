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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.upload.Upload;
import org.sipfoundry.sipxconfig.upload.UploadSpecification;

public class UploadTest extends TestCase {
    
    public void testGetSettingModel() {
        Upload f = new Upload();
        f.setModelFilesContext(TestHelper.getModelFilesContext());
        f.setSpecification(UploadSpecification.UNMANAGED);
        assertNotNull(f.getSettings());
    }
}
