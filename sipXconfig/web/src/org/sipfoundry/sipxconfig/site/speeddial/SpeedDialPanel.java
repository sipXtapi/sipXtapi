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
package org.sipfoundry.sipxconfig.site.speeddial;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.speeddial.Button;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class SpeedDialPanel extends BaseComponent {

    @Parameter
    public abstract List<Button> getButtons();

    public abstract Button getButton();

    public abstract int getIndex();

    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
    }

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this)) {
            afterRewind();
        }
    }

    private void afterRewind() {
    }

    public void add() {
        Button button = new Button();
        getButtons().add(button);
    }

    public void remove(int i) {
        getButtons().remove(i);
    }
}
