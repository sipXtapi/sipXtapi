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
package org.sipfoundry.sipxconfig.vm;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.text.ParseException;
import java.util.Date;

import com.thoughtworks.xstream.XStream;
import com.thoughtworks.xstream.annotations.Annotations;
import com.thoughtworks.xstream.annotations.XStreamAlias;
import com.thoughtworks.xstream.converters.basic.DateConverter;
import com.thoughtworks.xstream.io.xml.Dom4JDriver;
import com.thoughtworks.xstream.mapper.MapperWrapper;

import org.sipfoundry.sipxconfig.common.PrimaryKeySource;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.XstreamFieldMapper;

public class Voicemail implements Comparable, PrimaryKeySource {
    public static final char SEPARATOR = '/';
    private String m_basename;
    private MessageDescriptor m_descriptor;
    private File m_mailbox;

    public Voicemail(File mailstoreDirectory, String userId, String folderId, String basename) {
        File userDir = new File(mailstoreDirectory, userId);
        m_mailbox = new File(userDir, folderId);
        m_basename = basename;
    }

    public String getFolderId() {
        return m_mailbox.getName();
    }

    public String getUserId() {
        return m_mailbox.getParentFile().getName();
    }

    public String getBasename() {
        return m_basename;
    }

    public File getMediaFile() {
        return new File(m_mailbox, m_basename + ".wav");
    }

    public File getDescriptorFile() {
        return new File(m_mailbox, m_basename + ".xml");
    }

    public Date getTimestamp() {
        return getDescriptor().getTimestamp();
    }

    static class MessageDescriptorFormatException extends RuntimeException {
        MessageDescriptorFormatException(String message, ParseException cause) {
            super(message, cause);
        }
    }

    public long getDurationMillis() {
        return getDescriptor().getDurationsecs() * 1000;
    }

    MessageDescriptor getDescriptor() {
        if (m_descriptor == null) {
            try {
                FileInputStream descriptorFile = new FileInputStream(getDescriptorFile());
                m_descriptor = readMessageDescriptor(descriptorFile);
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
        }

        return m_descriptor;
    }

    MessageDescriptor readMessageDescriptor(InputStream in) throws IOException {
        XStream xstream = new XStream(new Dom4JDriver()) {
            protected MapperWrapper wrapMapper(MapperWrapper next) {
                return new XstreamFieldMapper(next);
            }
        };
        Annotations.configureAliases(xstream, MessageDescriptor.class);
        
        // NOTE: xtream's dateformatter uses fixed ENGLISH Locale, which
        // turns out is ok because mediaserver writes out timestamp in a fixed
        // format independent of OS locale.
        xstream.registerConverter(new DateConverter(MessageDescriptor.TIMESTAMP_FORMAT, new String[0]));
        
        MessageDescriptor md = (MessageDescriptor) xstream.fromXML(in);
        return md;
    }

    public String getSubject() {
        return getDescriptor().getSubject();
    }

    public String getFrom() {
        return getDescriptor().getFrom();
    }

    public String getFromBrief() {
        return SipUri.extractFullUser(getFrom().replace('+', ' '));
    }

    public int compareTo(Object o) {
        if (o == null || o instanceof Voicemail) {
            return -1;
        }
        return m_basename.compareTo(((Voicemail) o).getBasename());
    }

    public Object getPrimaryKey() {
        return getUserId() + SEPARATOR + getFolderId() + SEPARATOR + getBasename();
    }

    public static String[] decodePrimaryKey(Object primaryKey) {
        String[] ids = primaryKey.toString().split(String.valueOf(SEPARATOR));
        return ids;
    }

    @XStreamAlias("messagedescriptor")
    static class MessageDescriptor {
        static final String TIMESTAMP_FORMAT = "EEE, d-MMM-yyyy hh:mm:ss aaa z";
        private Date m_timestamp;
        private int m_durationsecs;
        private String m_subject;
        private String m_from;
        private String m_priority;
        private String m_id;        

        public int getDurationsecs() {
            return m_durationsecs;
        }

        public String getFrom() {
            return m_from;
        }

        public String getId() {
            return m_id;
        }

        public String getPriority() {
            return m_priority;
        }

        public String getSubject() {
            return m_subject;
        }

        public void setSubject(String subject) {
            m_subject = subject;
        }

        public Date getTimestamp() {
            return m_timestamp;
        }
    }
}
