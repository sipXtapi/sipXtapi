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
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.contrib.table.model.simple.SimpleTableColumn;

public class ExpressionTableColumn extends SimpleTableColumn {

    public ExpressionTableColumn(String strColumnName, String strDisplayName, boolean bSortable) {
        super(strColumnName, strDisplayName, bSortable);
    }

    public ExpressionTableColumn(String strColumnName, String strDisplayName, String expression_,
            boolean bSortable) {
        super(strColumnName, strDisplayName, bSortable);
    }
}
