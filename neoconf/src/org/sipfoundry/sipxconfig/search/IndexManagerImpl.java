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

import java.io.IOException;
import java.io.Serializable;
import java.util.Arrays;

import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.document.Document;
import org.apache.lucene.document.Field;
import org.apache.lucene.index.IndexReader;
import org.apache.lucene.index.IndexWriter;
import org.apache.lucene.index.Term;
import org.apache.lucene.store.Directory;
import org.hibernate.type.StringType;
import org.hibernate.type.Type;
import org.sipfoundry.sipxconfig.common.BeanWithId;

public class IndexManagerImpl implements IndexManager {
    // keep it sorted - used in binary search
    private static final String[] FIELDS = {
        "description", "extension", "firstName", "lastName", "name", "userName"
    };

    private Directory m_directory;

    private Analyzer m_analyzer;

    private boolean m_createIndex = true;

    private String getKeyword(Object bean, Serializable id) {
        StringBuffer buffer = new StringBuffer();
        buffer.append(bean.getClass().getName());
        buffer.append(':');
        buffer.append(id.toString());
        return buffer.toString();
    }

    private Document documentFromBean(Object bean, Serializable id, Object[] state,
            String[] fieldNames, Type[] types) {
        Document document = new Document();
        document.add(Field.Keyword(BeanWithId.ID_PROPERTY, getKeyword(bean, id)));
        for (int i = 0; i < fieldNames.length; i++) {
            String fieldName = fieldNames[i];
            Object value = state[i];
            if (value == null) {
                continue;
            }
            if (Arrays.binarySearch(FIELDS, fieldName) >= 0) {
                // index all fields we know about
                document.add(Field.Text(fieldName, (String) state[i]));
                document.add(Field.UnStored(IndexManager.DEFAULT_FIELD, (String) value));
            } else if (types[i] instanceof StringType) {
                // index all strings
                document.add(Field.UnStored(IndexManager.DEFAULT_FIELD, (String) value));
            }
        }

        return document;
    }

    public void indexBean(Object bean, Serializable id, Object[] state, String[] fieldNames,
            Type[] types) {
        removeBean(bean, id);
        Document document = documentFromBean(bean, id, state, fieldNames, types);
        IndexWriter writer = null;
        try {
            writer = new IndexWriter(m_directory, m_analyzer, m_createIndex);
            m_createIndex = false;
            writer.addDocument(document);

        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            LuceneUtils.closeQuietly(writer);
        }
    }

    public void removeBean(Object bean, Serializable id) {
        if (m_createIndex) {
            return;
        }
        IndexReader reader = null;
        try {
            reader = IndexReader.open(m_directory);
            Term idTerm = new Term(BeanWithId.ID_PROPERTY, getKeyword(bean, id));
            reader.delete(idTerm);
            reader.close();
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            LuceneUtils.closeQuietly(reader);
        }

    }

    public void setDirectory(Directory directory) {
        m_directory = directory;
    }

    public void setAnalyzer(Analyzer analyzer) {
        m_analyzer = analyzer;
    }
}
