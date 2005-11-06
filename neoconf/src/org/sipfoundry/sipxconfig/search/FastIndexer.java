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

import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.document.Document;
import org.apache.lucene.index.IndexReader;
import org.apache.lucene.index.IndexWriter;
import org.apache.lucene.index.Term;
import org.apache.lucene.store.Directory;
import org.hibernate.type.Type;

public class FastIndexer implements Indexer {
    private Directory m_directory;

    private Analyzer m_analyzer;

    private BeanAdaptor m_beanAdaptor;

    private boolean m_createIndex = true;

    public void indexBean(Object bean, Serializable id, Object[] state, String[] fieldNames,
            Type[] types) {
        removeBean(bean, id);
        Document document = new Document();
        if (!m_beanAdaptor.documentFromBean(document, bean, id, state, fieldNames, types)) {
            return;
        }
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
            Term idTerm = m_beanAdaptor.getIdentityTerm(bean, id);
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

    public void setBeanAdaptor(BeanAdaptor beanAdaptor) {
        m_beanAdaptor = beanAdaptor;
    }

    public void open() {
        // intentionally empty
    }

    public void close() {
        // intentionally empty
    }
}
