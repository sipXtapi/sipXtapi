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

import org.apache.tapestry.IAsset;
import org.apache.tapestry.asset.AssetFactory;

/**
 * UI control such as stylesheet assets to change skin
 */
public class SkinControl {
    public static final String CONTEXT_BEAN_NAME = "skin";
    private static final String RESOURCE_PATH = SkinControl.class.getPackage().getName().replace(
            '.', '/');
    private String m_layout = RESOURCE_PATH + "/layout.css";
    private String m_colors = RESOURCE_PATH + "/colors.css";
    private String m_logo = RESOURCE_PATH + "/sipxconfig-logo.png";
    private TapestryContext m_tapestryContext;
    private String m_helpLink;

    /**
     * Link to online help.  Can include 2 placeholders for app major and minor version numbers
     * example 
     *  http://example.com/help-{0}-{1}.html
     * will become
     *   "http://example.com/help-9-0.html
     * for version 9.0 of sipX
     * 
     * @return
     */
    public String getHelpLink() {
        return m_helpLink;
    }

    public void setHelpLink(String helpLink) {
        m_helpLink = helpLink;
    }

    public IAsset[] getStylesheetAssets() {
        IAsset[] assets = new IAsset[2];
        assets[0] = getAssetFactory().createAbsoluteAsset(m_layout, null, null);
        assets[1] = getAssetFactory().createAbsoluteAsset(m_colors, null, null);
        return assets;
    }

    private AssetFactory getAssetFactory() {
        return m_tapestryContext.getHivemindContext().getClasspathAssetFactory();
    }

    public IAsset getLogoAsset() {
        return getAssetFactory().createAbsoluteAsset(m_logo, null, null);
    }

    public void setTapestryContext(TapestryContext tapestryContext) {
        m_tapestryContext = tapestryContext;
    }

    public void setColors(String colors) {
        m_colors = colors;
    }

    public void setLayout(String layout) {
        m_layout = layout;
    }

    public void setLogo(String logo) {
        m_logo = logo;
    }
}
