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
package org.sipfoundry.sipxconfig.site;

import java.util.List;

import org.apache.tapestry.contrib.table.model.ITableColumnModel;
import org.apache.tapestry.contrib.table.model.ITableModel;
import org.apache.tapestry.contrib.table.model.ognl.ExpressionTableColumnModel;
import org.apache.tapestry.contrib.table.model.simple.SimpleTableModel;
import org.sipfoundry.sipxconfig.phone.Endpoint;

/**
 * List all the phones/endpoints for management and details drill-down
 */
public class ListPhones extends AbstractPhonePage {

    /** Serial number column */
    public static final String SERIAL_NUMBER = "Serial Number";

    // Return the model of the table
    public ITableModel getTableModel() {

        // Generate the list of data
        List endpointList = getPhoneContext().getPhoneDao().loadEndpoints();
        Endpoint[] endpoints = (Endpoint[]) endpointList.toArray(new Endpoint[0]);

        // Generate a simple sorting column model that uses OGNL to get the column data.
        // The columns are defined using pairs of strings.
        // The first string in the pair is the column name.
        // The second is an OGNL expression used to obtain the column value.
        ITableColumnModel objColumnModel = new ExpressionTableColumnModel(new String[] {
            SERIAL_NUMBER, "serialNumber" }, true);

        // Create the table model and return it
        return new SimpleTableModel(endpoints, objColumnModel);
    }

}
