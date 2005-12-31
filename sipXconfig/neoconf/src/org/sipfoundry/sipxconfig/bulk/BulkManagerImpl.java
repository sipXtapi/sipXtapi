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
package org.sipfoundry.sipxconfig.bulk;

import java.io.Reader;

import org.apache.commons.collections.Closure;
import org.apache.commons.lang.enums.ValuedEnum;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.Group;

public class BulkManagerImpl extends SipxHibernateDaoSupport implements BulkManager {
    private CsvParser m_csvParser;

    private CoreContext m_coreContext;

    private PhoneContext m_phoneContext;

    public void setCsvParser(CsvParser csvParser) {
        m_csvParser = csvParser;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void insertFromCsv(Reader reader) {
        Closure insertRow = new Closure() {
            public void execute(Object input) {
                insertRow((String[]) input);
            }
        };
        m_csvParser.parse(reader, insertRow);
    }

    private void insertRow(String[] row) {
        User user = newUserFromRow(row);
        Phone phone = newPhoneFromRow(row);

        // TODO: implement adding to groups
        // String phoneGroupName = get(row, Index.PHONE_GROUP);
        // String userGroupName = get(row, Index.USER_GROUP);
        insertData(user, null, phone, null);
    }

    private User newUserFromRow(String[] row) {
        User user = new User();
        user.setUserName(get(row, Index.USERNAME));
        user.setFirstName(get(row, Index.FIRST_NAME));
        user.setLastName(get(row, Index.LAST_NAME));
        user.setAliasesString(get(row, Index.ALIAS));
        user.setPin(get(row, Index.PINTOKEN), m_coreContext.getAuthorizationRealm());
        user.setSipPassword(get(row, Index.SIP_PASSWORD));
        return user;
    }

    private Phone newPhoneFromRow(String[] row) {
        PhoneModel model = PhoneModel.getModel(get(row, Index.BEAN_ID), get(row, Index.MODEL_ID));

        Phone phone = m_phoneContext.newPhone(model);
        phone.setDescription(get(row, Index.PHONE_DESCRIPTION));
        phone.setSerialNumber(get(row, Index.SERIAL_NUMBER));

        return phone;
    }

    /**
     * Creates user and adds it to user group, creates phone and adds it to phones group; then
     * creates the line for a newly added group on newly added phone.
     * 
     * @param user user to add or update
     * @param userGroup user group to which user will be added
     * @param phone phone to add or update
     * @param phoneGroup phone group to which phone will be added
     */
    void insertData(User user, Group userGroup, Phone phone, Group phoneGroup) {
        if (userGroup != null) {
            user.addGroup(userGroup);
        }

        if (phoneGroup != null) {
            phone.addGroup(phoneGroup);
        }

        Line line = phone.createLine();
        line.setUser(user);
        phone.addLine(line);

        m_coreContext.saveUser(user);
        m_phoneContext.storePhone(phone);
    }

    private static String get(String[] row, Index index) {
        return row[index.getValue()];
    }

    /**
     * Values of the enums below determine the exact format of CSV file "Username","Pintoken","Sip
     * Password","FirstName","LastName","Alias","UserGroup","SerialNumber","Manufacturer","Model","Phone
     * Group","Phone Description"
     */
    public static final class Index extends ValuedEnum {
        // user fields
        public static final Index USERNAME = new Index("userName", 0);
        public static final Index PINTOKEN = new Index("pintoken", 1);
        public static final Index SIP_PASSWORD = new Index("sipPassword", 2);
        public static final Index FIRST_NAME = new Index("firstName", 3);
        public static final Index LAST_NAME = new Index("lastName", 4);
        public static final Index ALIAS = new Index("alias", 5);
        public static final Index USER_GROUP = new Index("userGroupName", 6);

        // phone fields
        public static final Index SERIAL_NUMBER = new Index("serialNumber", 7);
        public static final Index BEAN_ID = new Index("beanId", 8);
        public static final Index MODEL_ID = new Index("modelId", 9);
        public static final Index PHONE_GROUP = new Index("phoneGroupName", 10);
        public static final Index PHONE_DESCRIPTION = new Index("description", 11);

        private Index(String name, int value) {
            super(name, value);
        }
    }
}
