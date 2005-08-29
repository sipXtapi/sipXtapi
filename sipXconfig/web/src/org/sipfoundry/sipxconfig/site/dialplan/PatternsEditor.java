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
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPattern;

/**
 * PatternsEditor
 * 
 * TODO: we may want to use direct links and rule id's in future. As it's
 * implemented now, this page has problems with stale links.
 */
public abstract class PatternsEditor extends BaseComponent {
    private Object m_patternToBeRemoved;

    public abstract List getPatterns();

    public abstract DialPattern getPattern();

    public boolean isLast() {
        DialPattern pattern = getPattern();
        List patterns = getPatterns();
        return pattern == patterns.get(patterns.size() - 1);
    }

    public void add(IRequestCycle cycle_) {
        List patterns = getPatterns();
        patterns.add(new DialPattern());
    }

    public void delete(IRequestCycle cycle_) {
        m_patternToBeRemoved = getPattern();
    }

    /**
     * Once cannot modify the patterns list inside of the delete listener.
     * Instead delete listener just sets the flag.
     */
    private void delayedDelete() {
        if (null != m_patternToBeRemoved) {
            List patterns = getPatterns();
            patterns.remove(m_patternToBeRemoved);
            m_patternToBeRemoved = null;
        }
    }

    /**
     * There is no "componentSubmit" method that would be called after rendering
     * is done (just before or after form submit listener called). Listeners
     * cannot really modify values of the properties that are used for rendering
     * (specifically listener cannot remove the item from the list used in
     * Foreach). According to e-mail on the Tapestry users list one has to override
     * renderComponent and put the processing of such delayed listeners there.
     */
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        if (cycle.isRewinding()) {
            delayedDelete();
        }
    }
}
