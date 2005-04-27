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
package org.sipfoundry.sipxconfig.components;

import java.util.Collection;

import ognl.Ognl;
import ognl.OgnlException;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.util.prop.OgnlUtils;

public class ObjectSelectionModel implements IPropertySelectionModel {

    private transient Object m_objParsedExpression;

    private Object[] m_objects;

    private String m_labelExpression;

    public void setCollection(Collection objects) {
        m_objects = objects.toArray();
    }

    public void setLabelExpression(String labelOgnlExpression) {
        m_labelExpression = labelOgnlExpression;
    }

    public int getOptionCount() {
        return m_objects.length;
    }

    public Object getOption(int index) {
        return m_objects[index];
    }

    public String getLabel(int index) {
        // source adapted from OgnlTableColumnEvaluator

        // If no expression is given, then this is dummy column. Return something.
        if (StringUtils.isBlank(m_labelExpression)) {
            return StringUtils.EMPTY;
        }

        buildParsedExpression();

        try {
            Object objValue = Ognl.getValue(m_objParsedExpression, m_objects[index]);
            return safeToString(objValue);
        } catch (OgnlException e) {
            throw new RuntimeException(e);
        }
    }
    
    private synchronized void buildParsedExpression() {
        if (m_objParsedExpression == null) {
            m_objParsedExpression = OgnlUtils.getParsedExpression(m_labelExpression);
        }
    }

    private String safeToString(Object o) {
        return o == null ? StringUtils.EMPTY : o.toString();
    }

    /** based off StringPropertySelectionModel */
    public String getValue(int index) {
        return Integer.toString(index);
    }

    /** based off StringPropertySelectionModel */
    public Object translateValue(String value) {
        int index = Integer.parseInt(value);
        return getOption(index);
    }
}
