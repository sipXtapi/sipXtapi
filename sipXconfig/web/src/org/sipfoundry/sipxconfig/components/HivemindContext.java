/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.asset.AssetFactory;

public interface HivemindContext {
    public AssetFactory getClasspathAssetFactory();

    public void setClasspathAssetFactory(AssetFactory classpathAssetFactory);
}
