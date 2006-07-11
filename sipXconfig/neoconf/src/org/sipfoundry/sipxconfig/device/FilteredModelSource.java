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
package org.sipfoundry.sipxconfig.device;

import java.util.Collection;
import java.util.regex.Pattern;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.sipfoundry.sipxconfig.phone.PhoneModel;

public class FilteredModelSource implements ModelSource {

    private Predicate m_filter;

    private ModelSource m_modelSource;

    public void setModelSource(ModelSource modelSource) {
        m_modelSource = modelSource;
    }

    /**
     * Sets regular expression that is used to construct the filter limiting which models are
     * available.
     */
    public void setCertified(String certifiedRegex) {
        m_filter = new CertifiedPhones(certifiedRegex);
    }

    public void setFilter(Predicate filter) {
        m_filter = filter;
    }

    public Collection<PhoneModel> getModels() {
        Collection models = m_modelSource.getModels();
        if (m_filter != null) {
            models = CollectionUtils.select(models, m_filter);
        }
        return models;
    }

    private static class CertifiedPhones implements Predicate {
        private Pattern m_pattern;

        public CertifiedPhones(String pattern) {
            m_pattern = Pattern.compile(pattern);
        }

        public boolean evaluate(Object object) {
            PhoneModel model = (PhoneModel) object;
            return m_pattern.matcher(model.getName()).matches();
        }
    }
}
