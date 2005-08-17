/*
 *
 *
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 snom technology AG
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.snom;

import org.apache.commons.lang.enums.Enum;

public final class SnomModel extends Enum {

    public static final SnomModel MODEL_360 = new SnomModel("snom360", 12);

    public static final SnomModel MODEL_320 = new SnomModel("snom320", 12);

    public static final SnomModel UNKNOWN = new SnomModel("unknown", 0);

    private String m_modelId;

    private int m_maxLines;

    private SnomModel(String modelId, int maxLines) {
        super(modelId);
        setModelId(modelId);
        setMaxLines(maxLines);
    }

    public static SnomModel getModel(String modelId) {
        SnomModel model = (SnomModel) getEnumMap(SnomModel.class).get(modelId);
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
