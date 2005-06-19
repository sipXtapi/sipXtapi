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
package org.sipfoundry.sipxconfig.phone.grandstream;

import org.apache.commons.lang.enums.Enum;

/**
 * Static differences in grandstream models
 */
public final class GrandstreamModel extends Enum {

    public static final GrandstreamModel MODEL_BUDGETONE = new GrandstreamModel(
            "grandstreamBudgeTone", 1);

    public static final GrandstreamModel MODEL_HANDYTONE = new GrandstreamModel(
            "grandstreamHandyTone", 1);

    private static final GrandstreamModel UNKNOWN = new GrandstreamModel("unknown", 0);

    private String m_modelId;

    private int m_maxLines;

    private String m_modelFile;

    /* IP Models */
    private GrandstreamModel(String modelId, int maxLines) {
        super(modelId);
        setModelId(modelId);
        setMaxLines(maxLines);
    }

    public String getModelFile() {
        return m_modelFile;
    }

    public void setModelFile(String modelFile) {
        m_modelFile = modelFile;
    }

    public static GrandstreamModel getModel(String modelId) {
        GrandstreamModel model = (GrandstreamModel) getEnumMap(GrandstreamModel.class).get(
                modelId);
        return model == null ? UNKNOWN : model;
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
