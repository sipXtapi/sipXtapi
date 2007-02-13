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
package org.sipfoundry.sipxconfig.setting;

import java.util.Iterator;

public class ConditionalSet extends SettingSet implements ConditionalSetting {
    private String m_if;
    private String m_unless;

    public String getIf() {
        return m_if;
    }

    public void setIf(String if1) {
        m_if = if1;
    }

    public String getUnless() {
        return m_unless;
    }

    public void setUnless(String unless) {
        m_unless = unless;
    }

    /**
     * Handle if/unless expression with custom evaluator
     */
    public Setting evaluate(SettingExpressionEvaluator evaluator) {
        Setting settings = new ExpressionEvaluatorRunner().evaluate(evaluator, this);
        return settings;
    }

    protected Setting shallowCopy() {
        SettingSet copy = (SettingSet) super.shallowCopy();
        return copy;
    }

    /**
     * Not re-entrant or multi-threaded. (Use and throw away)
     */
    static class ExpressionEvaluatorRunner implements SettingVisitor {
        private SettingExpressionEvaluator m_evaluator;
        private SettingSet m_copy;

        public Setting evaluate(SettingExpressionEvaluator evaluator, ConditionalSet settings) {
            m_evaluator = evaluator;
            m_copy = (SettingSet) settings.shallowCopy();
            Iterator i = settings.getValues().iterator();
            while (i.hasNext()) {
                Setting s = (Setting) i.next();
                s.acceptVisitor(this);
            }
            return m_copy;
        }

        public void visitSetting(Setting setting) {
            ConditionalSetting conditional = (ConditionalSetting) setting;
            if (isTrue(conditional.getIf(), conditional.getUnless(), setting)) {
                Setting copy = conditional.copy();
                addCopy(copy);
            }
        }

        public boolean visitSettingGroup(Setting set) {
            ConditionalSet conditional = (ConditionalSet) set;
            boolean isTrue = isTrue(conditional.getIf(), conditional.getUnless(), set);
            if (isTrue) {
                Setting copy = conditional.shallowCopy();
                addCopy(copy);
            }
            return isTrue;
        }

        private void addCopy(Setting copy) {
            String parentPath = copy.getParent().getPath();
            Setting parentCopy = m_copy.getSetting(parentPath);
            if (parentCopy != null) {
                parentCopy.addSetting(copy);
            }
        }

        private boolean isTrue(String ifExpression, String unlessExpression, Setting setting) {
            boolean isTrue = true;

            if (ifExpression != null && unlessExpression != null) {
                isTrue = m_evaluator.isExpressionTrue(ifExpression, setting)
                        && !m_evaluator.isExpressionTrue(unlessExpression, setting);
            } else if (ifExpression != null) {
                isTrue = m_evaluator.isExpressionTrue(ifExpression, setting);
            } else if (unlessExpression != null) {
                isTrue = !m_evaluator.isExpressionTrue(unlessExpression, setting);
            }

            return isTrue;
        }
    }
}
