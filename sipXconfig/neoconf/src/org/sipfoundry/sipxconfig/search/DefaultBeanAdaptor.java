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
package org.sipfoundry.sipxconfig.search;

import java.io.Serializable;
import java.util.Arrays;
import java.util.Iterator;
import java.util.Set;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.lucene.document.Document;
import org.apache.lucene.document.Field;
import org.apache.lucene.index.Term;
import org.hibernate.type.StringType;
import org.hibernate.type.Type;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.Group;

public class DefaultBeanAdaptor implements BeanAdaptor {
    /** only those classes can be used to search by class */
    public static final Class[] CLASSES = {
        // TODO: inject externally
        User.class, Phone.class, Group.class, Gateway.class, CallGroup.class, DialingRule.class,
        Bridge.class, Conference.class, ParkOrbit.class, AutoAttendant.class
    };

    private static final Log LOG = LogFactory.getLog(DefaultBeanAdaptor.class);

    private static final String[] DESCRIPTION_FIELDS = {
        "description"
    };

    private static final String[] NAME_FIELDS = {
        "lastName", "firstName", "name", "extension", "userName", "serialNumber"
    };

    /**
     * Name of the fields that are stored in the index. All the remaining string fields are
     * indexed, but not stored.
     */

    private static final String[] FIELDS;

    static {
        FIELDS = (String[]) ArrayUtils.addAll(NAME_FIELDS, DESCRIPTION_FIELDS);
        Arrays.sort(FIELDS);
    }

    /**
     * @return true if the document should be added to index
     */
    public boolean documentFromBean(Document document, Object bean, Serializable id,
            Object[] state, String[] fieldNames, Type[] types) {
        if (!indexClass(document, bean.getClass())) {
            return false;
        }
        document.add(Field.Keyword(BeanWithId.ID_PROPERTY, getKeyword(bean, id)));
        for (int i = 0; i < fieldNames.length; i++) {
            Object value = state[i];
            if (value != null) {
                indexField(document, value, fieldNames[i], types[i]);
            }
        }
        return true;
    }

    private boolean indexField(Document document, Object state, String fieldName, Type type) {
        if (Arrays.binarySearch(FIELDS, fieldName) >= 0) {
            // index all fields we know about
            document.add(Field.Text(fieldName, (String) state));
            document.add(Field.UnStored(Indexer.DEFAULT_FIELD, (String) state));
            return true;
        } else if (type instanceof StringType) {
            // index all strings
            document.add(Field.UnStored(Indexer.DEFAULT_FIELD, (String) state));
            return true;
        } else if (fieldName.equals("aliases")) {
            Set aliases = (Set) state;
            for (Iterator a = aliases.iterator(); a.hasNext();) {
                String alias = (String) a.next();
                document.add(Field.UnStored("alias", alias));
                document.add(Field.UnStored(Indexer.DEFAULT_FIELD, alias));
            }
            return true;
        }
        return false;
    }

    public boolean indexClass(Document doc, Class beanClass) {
        for (int i = 0; i < CLASSES.length; i++) {
            Class klass = CLASSES[i];
            if (klass.isAssignableFrom(beanClass)) {
                doc.add(Field.Keyword(Indexer.CLASS_FIELD, klass.getName()));
                return true;
            }
        }
        return false;
    }

    private String getKeyword(Object bean, Serializable id) {
        StringBuffer buffer = new StringBuffer();
        buffer.append(bean.getClass().getName());
        buffer.append(':');
        buffer.append(id.toString());
        return buffer.toString();
    }

    public Term getIdentityTerm(Object bean, Serializable id) {
        return new Term(BeanWithId.ID_PROPERTY, getKeyword(bean, id));
    }

    public Identity getBeanIdentity(Document document) {
        try {
            String docId = document.get("id");
            String[] ids = StringUtils.split(docId, ':');
            Class klass = Class.forName(ids[0]);
            Integer id = Integer.valueOf(ids[1]);
            Identity ident = new Identity(klass, id);
            ident.setName(fieldsToString(document, NAME_FIELDS));
            ident.setDescription(fieldsToString(document, DESCRIPTION_FIELDS));
            return ident;
        } catch (NumberFormatException e) {
            LOG.warn("invalid bean id", e);
            return null;
        } catch (ClassNotFoundException e) {
            LOG.warn("invalid bean class", e);
            return null;
        }
    }

    private String fieldsToString(Document doc, String[] fields) {
        StringBuffer buffer = new StringBuffer();
        for (int i = 0; i < fields.length; i++) {
            Field field = doc.getField(fields[i]);
            if (field == null) {
                continue;
            }
            if (buffer.length() > 0) {
                buffer.append(", ");
            }
            buffer.append(field.stringValue());

        }
        return buffer.toString();
    }
}
