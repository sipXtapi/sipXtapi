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
package org.sipfoundry.sipxconfig.phone.polycom;

import org.apache.commons.lang.enum.Enum;

/**
 * Differences in polycom phone models 
 */
public final class PolycomModel extends Enum {    

    /** basic phone */
    public static final PolycomModel MODEL_300 = new PolycomModel("polycom300", "SoundPoint IP 300", 2);

    /** standard phone */
    public static final PolycomModel MODEL_500 = new PolycomModel("polycom500", "SoundPoint IP 500", 4);

    /** deluxe phone */
    public static final PolycomModel MODEL_600 = new PolycomModel("polycom600", "SoundPoint IP 600", 6);

    /** conference phone, verify num lines */
    public static final PolycomModel MODEL_3000 = new PolycomModel("polycom3000", "SoundPoint IP 3000", 6);
    
    private static final PolycomModel UNKNOWN = new PolycomModel("unknown", "SoundPoint IP", 0);
    
    private String m_modelId;

    private String m_displayLabel;

    private int m_maxLines;
    
    private PolycomModel(String modelId, String displayLabel, int maxLines) {
        super(modelId);
        setModelId(modelId);
        setDisplayLabel(displayLabel);
        setMaxLines(maxLines);
    }
    
    public static PolycomModel getModel(String modelId) {
        PolycomModel model = (PolycomModel) getEnumMap(PolycomModel.class).get(modelId);
        return model == null ? UNKNOWN : model;
    }

    public String getDisplayLabel() {
        return m_displayLabel;
    }

    public void setDisplayLabel(String displayLabel) {
        this.m_displayLabel = displayLabel;
    }

    public int getMaxLines() {
        return m_maxLines;
    }

    public void setMaxLines(int maxLines) {
        m_maxLines = maxLines;
    }

    public String getModelId() {
        return m_modelId;
    }

    public void setModelId(String modelId) {
        m_modelId = modelId;
    }
}
