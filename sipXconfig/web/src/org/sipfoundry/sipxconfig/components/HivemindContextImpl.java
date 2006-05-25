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

public class HivemindContextImpl implements HivemindContext {
    private AssetFactory m_classpathAssetFactory;

    public AssetFactory getClasspathAssetFactory() {
        return m_classpathAssetFactory;
    }

    public void setClasspathAssetFactory(AssetFactory classpathAssetFactory) {
        m_classpathAssetFactory = classpathAssetFactory;
    }
}
