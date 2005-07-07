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
package org.sipfoundry.sipxconfig.resource;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;

import com.thoughtworks.xstream.XStream;
import com.thoughtworks.xstream.alias.ClassMapper;
import com.thoughtworks.xstream.mapper.MapperWrapper;

import org.sipfoundry.sipxconfig.common.User;

public class XStreamTranslator implements ResourceTranslator {

    private XStream m_xstream;

    public XStreamTranslator() {
        m_xstream = new XStream() {
            protected MapperWrapper wrapMapper(MapperWrapper next) {
                return new FieldPrefixStrippingMapper(next);
            }
        };

        // TODO take as list
        m_xstream.alias("user", User.class);
    }

    public Object read(InputStream in) {
        Object o = m_xstream.fromXML(new InputStreamReader(in));
        return o;
    }

    public void merge(InputStream in, Object base) {
        m_xstream.fromXML(new InputStreamReader(in), base);
    }

    public InputStream write(Object object) {
        String xml = m_xstream.toXML(object);
        InputStream stream = new ByteArrayInputStream(xml.getBytes());

        return stream;
    }

    /**
     * A sample mapper strips the underscore prefix of fieldnames in the XML
     */
    class FieldPrefixStrippingMapper extends MapperWrapper {
        
        private static final String MEMBER_PREFIX = "m_";

        public FieldPrefixStrippingMapper(ClassMapper wrapped) {
            super(wrapped);
        }

        public String serializedMember(Class type, String memberName) {
            String fixedMemberName = memberName;
            if (memberName.startsWith(MEMBER_PREFIX)) {
                // m_blah -> blah
                fixedMemberName = memberName.substring(2); // chop off leading char and underscore
            }
            return super.serializedMember(type, fixedMemberName);
        }

        public String realMember(Class type, String serialized) {
            String fieldName = super.realMember(type, serialized);
            // Not very efficient or elegant, but enough to get the point across.
            // Luckily the CachingMapper will ensure this is only ever called once per field per class.
            try {
                type.getDeclaredField(MEMBER_PREFIX + fieldName);
                return MEMBER_PREFIX + fieldName;
            } catch (NoSuchFieldException e) {
                return fieldName;
            }
        }
    }
}
