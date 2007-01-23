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

import java.util.List;

import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.Node;

public class DistributionListsReader extends XmlReaderImpl<DistributionList[]> {

    @Override
    public DistributionList[] readObject(Document doc) {
        Node root = doc.getRootElement();
        List<Element> nodeLists = root.selectNodes("//distributions/list");
        DistributionList[] lists = new DistributionList[nodeLists.size()];
        for (int i = 0; i < nodeLists.size(); i++) {
            Node listNode = nodeLists.get(i);
            DistributionList list = new DistributionList(); 
            List<Node> extensionsNodes = listNode.selectNodes("destination");
            String[] extensions = new String[extensionsNodes.size()];            
            for (int j = 0; j < extensionsNodes.size(); j++) {
                extensions[j] = extensionsNodes.get(j).getText();
            }
            list.setExtensions(extensions);
            int position = Integer.parseInt(listNode.valueOf("index/text()"));
            list.setPosition(position);
        }
        
        return lists;
    }
}
