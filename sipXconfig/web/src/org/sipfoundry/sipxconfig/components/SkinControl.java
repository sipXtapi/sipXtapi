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

import java.util.HashMap;
import java.util.Map;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.asset.AssetFactory;

/**
 * UI control such as stylesheet assets to change skin
 */
public class SkinControl {
    public static final String CONTEXT_BEAN_NAME = "skin";
    private static final String ASSET_COLORS = "colors.css";
    private static final String ASSET_LAYOUT = "layout.css";
    private TapestryContext m_tapestryContext;
    // overrideable in skin
    private String m_helpLink = "http://www.sipfoundry.org/doc/sipxhelp/{0}.{1}.html";    
    private String m_copyright;
    private String m_productName;
    private Map<String, String> m_assets = new HashMap();

    public SkinControl() {
        // default skin resources
        m_assets.put("logo.png", "org/sipfoundry/sipxconfig/components/sipxconfig-logo.png");
        m_assets.put(ASSET_LAYOUT, "org/sipfoundry/sipxconfig/components/layout.css");
        m_assets.put(ASSET_COLORS, "org/sipfoundry/sipxconfig/components/colors.css");
    }

    /**
     * Link to online help. Can include 2 placeholders for app major and minor version numbers
     * example http://example.com/help-{0}-{1}.html will become "http://example.com/help-9-0.html
     * for version 9.0 of sipX
     */
    public String getHelpLink() {
        return m_helpLink;
    }

    public void setHelpLink(String helpLink) {
        m_helpLink = helpLink;
    }

    /**
     * Copyright displayed in sipXconfig footer. If not configured default sipFoundry copyright is displayed.
     */
    public String getCopyright() {
        return m_copyright;
    }

    public void setCopyright(String copyright) {
        m_copyright = copyright;
    }
        
    public String getProductName() {
        return m_productName;
    }

    public void setProductName(String productName) {
        m_productName = productName;
    }

    public IAsset[] getStylesheetAssets() {
        IAsset[] assets = new IAsset[2];
        assets[0] = getAsset(ASSET_COLORS);
        assets[1] = getAsset(ASSET_LAYOUT);
        return assets;
    }

    private AssetFactory getAssetFactory() {
        return m_tapestryContext.getHivemindContext().getClasspathAssetFactory();
    }

    public Map<String, String> getAssets() {
        return m_assets;
    }

    public void setAssets(Map<String, String> assets) {
        m_assets.putAll(assets);
    }

    public IAsset getAsset(String path) {
        String resourcePath = m_assets.get(path);
        if (resourcePath == null) {
            return null;
        }

        return getAssetFactory().createAbsoluteAsset(resourcePath, null, null);
    }

    public void setTapestryContext(TapestryContext tapestryContext) {
        m_tapestryContext = tapestryContext;
    }
}
