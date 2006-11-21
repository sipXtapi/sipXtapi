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
package org.sipfoundry.sipxconfig.site.cdr;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.cdr.CdrSearch;
import org.sipfoundry.sipxconfig.components.NewEnumPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryContext;

public abstract class CdrFilter extends BaseComponent {

    public abstract boolean getSearchMode();

    public abstract TapestryContext getTapestry();

    public IPropertySelectionModel getSelectionModel() {
        NewEnumPropertySelectionModel model = new NewEnumPropertySelectionModel(CdrSearch.Mode.class);

        String label = getMessages().getMessage("label.filter");
        return getTapestry().addExtraOption(model, label);
    }

}
