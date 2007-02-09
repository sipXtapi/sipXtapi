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
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.ParseException;
import java.util.Date;

import com.thoughtworks.xstream.XStream;
import com.thoughtworks.xstream.annotations.Annotations;
import com.thoughtworks.xstream.annotations.XStreamAlias;
import com.thoughtworks.xstream.converters.basic.DateConverter;
import com.thoughtworks.xstream.io.xml.DomDriver;
import com.thoughtworks.xstream.mapper.MapperWrapper;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.XstreamFieldMapper;

public class Voicemail implements Comparable {
    private String m_basename;
    private MessageDescriptor m_descriptor;
    private File m_mailbox;
    private File m_userDirectory; 
   
    public Voicemail(File mailstoreDirectory, String userId, String folderId, String basename) {
        m_userDirectory = new File(mailstoreDirectory, userId);
        m_mailbox = new File(m_userDirectory, folderId);
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
    
    public boolean isHeard() {
        return !(new File(getMailboxDirectory(), getBasename() + ".sta").exists());
    }
    
    public void move(String destinationFolderId) {
        File destination = new File(m_userDirectory, destinationFolderId);
        for (File f : getAllFiles()) {
            f.renameTo(new File(destination, f.getName()));
        }        
    }
    
    public void delete() {
        for (File f : getAllFiles()) {
            f.delete();
        }
    }
    
    public File[] getAllFiles() {
        return new File[] {
            getMediaFile(),
            getDescriptorFile()
        };
    }
    
    public File getMailboxDirectory() {
        return m_mailbox;
    }

    public File getMediaFile() {
        return new File(getMailboxDirectory(), getBasename() + ".wav");
    }

    public File getDescriptorFile() {
        return new File(getMailboxDirectory(), getBasename() + ".xml");
    }

    public Date getTimestamp() {
        return getDescriptor().getTimestamp();
    }
    
    /**
     * TODO This needs to replace basename
     */
    public String getRealBasename() {
        return getBasename().substring(0, getBasename().indexOf('-'));
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
            FileInputStream descriptorFile = null;
            try {
                descriptorFile = new FileInputStream(getDescriptorFile());
                m_descriptor = readMessageDescriptor(descriptorFile);
            } catch (IOException e) {
                throw new RuntimeException(e);
            } finally {
                IOUtils.closeQuietly(descriptorFile);
            }
        }

        return m_descriptor;
    }

    static MessageDescriptor readMessageDescriptor(InputStream in) throws IOException {
        XStream xstream = getXmlSerializer();
        MessageDescriptor md = (MessageDescriptor) xstream.fromXML(in);
        return md;
    }
    
    
    /**
     * Element order is not preserved!!!
     */
    static void writeMessageDescriptor(MessageDescriptor md, OutputStream out) throws IOException {
        XStream xstream = getXmlSerializer();
        // See http://xstream.codehaus.org/faq.html#XML   
        // Section  "Why does XStream not write XML in UTF-8?"
        out.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n".getBytes());             
        xstream.toXML(md, out);
    }

    static XStream getXmlSerializer() {
        XStream xstream = new XStream(new DomDriver()) {
            protected MapperWrapper wrapMapper(MapperWrapper next) {
                return new XstreamFieldMapper(next);
            }
        };
        Annotations.configureAliases(xstream, MessageDescriptor.class);
        
        // NOTE: xtream's dateformatter uses fixed ENGLISH Locale, which
        // turns out is ok because mediaserver writes out timestamp in a fixed
        // format independent of OS locale.
        xstream.registerConverter(new DateConverter(MessageDescriptor.TIMESTAMP_FORMAT, new String[0]));
        
        return xstream;
    }

    public String getSubject() {
        return getDescriptor().getSubject();
    }
    
    public void setSubject(String subject) {
        getDescriptor().setSubject(subject);
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
    
    public void save() {
        FileOutputStream out = null;
        try {
            out = new FileOutputStream(getDescriptorFile());
            writeMessageDescriptor(getDescriptor(), out);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(out);
        }
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
