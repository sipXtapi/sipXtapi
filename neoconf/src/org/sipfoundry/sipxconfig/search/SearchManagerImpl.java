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
import java.util.ArrayList;
import java.util.Collection;

import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.document.Document;
import org.apache.lucene.queryParser.ParseException;
import org.apache.lucene.queryParser.QueryParser;
import org.apache.lucene.search.Hits;
import org.apache.lucene.search.IndexSearcher;
import org.apache.lucene.search.Query;
import org.apache.lucene.store.Directory;
import org.sipfoundry.sipxconfig.search.BeanAdaptor.Identity;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class SearchManagerImpl extends HibernateDaoSupport implements SearchManager {

    private Directory m_directory;

    private Analyzer m_analyzer;

    private BeanAdaptor m_beanAdaptor;

    public void setDirectory(Directory directory) {
        m_directory = directory;
    }

    public void setAnalyzer(Analyzer analyzer) {
        m_analyzer = analyzer;
    }

    public void setBeanAdaptor(BeanAdaptor beanAdaptor) {
        m_beanAdaptor = beanAdaptor;
    }

    public Collection search(String queryText) {
        Hits hits = getHits(queryText);
        return hits2beans(hits);
    }

    private Collection hits2beans(Hits hits) {
        try {
            HibernateTemplate hibernate = getHibernateTemplate();
            final int hitCount = hits.length();
            Collection results = new ArrayList(hitCount);
            for (int i = 0; i < hitCount; i++) {
                Document document = hits.doc(i);
                Identity identity = m_beanAdaptor.getBeanIdentity(document);
                if (identity == null) {
                    continue;
                }
                Object found = hibernate.load(identity.getBeanClass(), identity.getBeanId());
                results.add(found);
            }
            return results;
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    private Hits getHits(String queryText) {
        IndexSearcher searcher = null;
        try {
            QueryParser parser = new QueryParser(Indexer.DEFAULT_FIELD, m_analyzer);
            Query query = parser.parse(queryText);
            searcher = new IndexSearcher(m_directory);
            return searcher.search(query);
        } catch (IOException e) {
            throw new RuntimeException();
        } catch (ParseException e) {
            throw new RuntimeException();
        } finally {
            LuceneUtils.closeQuietly(searcher);
        }
    }
}
