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
package org.sipfoundry.sipxconfig.setting;

import java.io.File;
import java.io.FilenameFilter;
import java.text.MessageFormat;
import java.util.Comparator;
import java.util.Set;
import java.util.TreeSet;
import java.util.regex.Pattern;

import org.apache.commons.lang.StringUtils;

public class ModelFilesContextImpl implements ModelFilesContext {
    public static final char LEVEL_SEPARATOR = '_';
    private static final String LEVEL_SEPARATOR_STR = String.valueOf(LEVEL_SEPARATOR);
    private static final String GLOB_REPLACEMENT_TOKEN = "xxx";
    private String m_configDirectory;
    
    private ModelBuilder m_modelBuilder;
    
    public Setting loadModelFile(String basename) {
        File modelFile = getModelFile(basename, null);
        SettingSet model = m_modelBuilder.buildModel(modelFile, null);
        return model.copy();        
    }
    
    /**
     * loadModelFile("phone.xml", "cisco", new String[] { "7912" })
     * Loads settings model from XML file
     * 
     * The full path of the model file is: systemEtcDirectory/manufacturer/basename
     * 
     * @return new copy of the settings model
     */
    public Setting loadModelFile(String basename, String manufacturer) {
        File modelFile = getModelFile(basename, manufacturer);
        Setting model = m_modelBuilder.buildModel(modelFile, null);
        return model;        
    }
    
    public Setting loadModelFile(String basename, String manufacturer, String[] details) {
        File dir = getModelDirectory(manufacturer);
        String[] candidates = getCandidates(basename, details);
        String[] files = new BasenameFilter(candidates).getMatches(dir);
        Setting model = null;
        for (int i = 0; i < files.length; i++) {
            File modelFile = getModelFile(files[i], manufacturer);
            model = m_modelBuilder.buildModel(modelFile, model);            
        }
        return model;
    }        
        
    File getModelDirectory(String manufacturer) {
        File modelDir;
        if (manufacturer == null) {
            modelDir = new File(m_configDirectory);
        } else {
            modelDir = new File(m_configDirectory, manufacturer);
        }
        return modelDir;
    }

    /**
     * @param manufacturer null if model file is in root directory
     */
    public File getModelFile(String basename, String manufacturer) {
        File modelDir = getModelDirectory(manufacturer);        
        return new File(modelDir, basename);
    }    

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }
    
    public void setModelBuilder(ModelBuilder modelBuilder) {
        m_modelBuilder = modelBuilder;
    }

    static String chompExtension(String name) {
        int dot = name.lastIndexOf('.');
        return dot >= 0 ? name.substring(0, dot) : null;
    }
    
    static String getExtension(String name) {
        int dot = name.lastIndexOf('.');
        return dot >= 0 && dot < name.length() - 1 ? name.substring(dot + 1) : null;
    }
    
    static String globToPattern(String name) {
        String x = "[.]";
        String periodRegExp = x;
        String escapedPeriod = x; // surprisingly, esacaped period and period regexp is identical strings
        String glob = name.replaceAll(periodRegExp, escapedPeriod);
        glob = glob.replaceAll(GLOB_REPLACEMENT_TOKEN, ".*");
        return glob;
    }
    
    static String[] getCandidates(String basename, String[] details) {
        if (details == null) {
            return new String[] { 
                basename 
            };
        }
        
        String[] candidates = new String[details.length + 1];
        candidates[0] = basename;
        String prefix = chompExtension(basename);
        String ext = getExtension(basename);
        for (int i = 0; i < details.length; i++) {
            String candidate = prefix + LEVEL_SEPARATOR + details[i];
            candidates[i + 1] = candidate + '.' + ext;
            prefix = candidate;
        }
        return candidates;
    }

    /**
     * Order setting files from most specific last, or least sepecific first. 
     * Takes into account hierarchy of "_" and wildcards (e.g. 79XXX is less specific
     * than 7912)
     */
    static class MostSpecificLast implements Comparator {

        public int compare(Object arg0, Object arg1) {            
            String s1 = arg0.toString();
            String s2 = arg1.toString();
            // higer count of '_' is more specific 
            int difference = compareDetailLevel(s1, s2);
            if (difference != 0) {
                return difference;
            }
            
            int specificityDifference = compareSpecificity(s1, s2);
            if (specificityDifference != 0) {
                return specificityDifference;
            }
            
            String msg = MessageFormat
                    .format(
                            "Logic error, one of these files should not passed file filter \"{0}\" \"{1}\"",
                            (Object[]) new String[] {s1, s2});
            throw new IllegalStateException(msg);
        }
    }
    
    /**
     * Strings with more details are more specific
     * e.g.  a_b (count = 1) is less specific then x_y_z (count = 2)
     *       so return positive value
     */
    static int compareDetailLevel(String s1, String s2) {
        int level1 = StringUtils.countMatches(s1, LEVEL_SEPARATOR_STR);
        int level2 = StringUtils.countMatches(s2, LEVEL_SEPARATOR_STR);
        return  (level1 - level2);
    }
    
    /**
     * Strings with wildcards furthest from the end of the string is the
     * least specific
     * e.g.  axxxc (distance of xxx from end = 4) is less specific than 
     *       abxxx (distance of xxx from end = 3) so return postive value     
     */
    static int compareSpecificity(String s1, String s2) {
        // algorithm details:
        //  - convert wildcards to "1" and anything else to "0"
        //  - reverse the string so comparison starts at end
        //  - pad shorter string w/1's, implicit wildcards        
        int len = Math.max(s1.length(), s2.length());
        String x1 = reversePad(encode(s1), len);
        String x2 = reversePad(encode(s2), len);
        return x2.compareTo(x1);
    }
    
    static String reversePad(String s, int len) {
        return StringUtils.rightPad(StringUtils.reverse(s), len, '1');
    }
    
    static String encode(String s) {
        String zero = "0";
        return s.replaceAll("[^x]", zero).replaceAll("(xxx)", "1").replaceAll("x", zero);
    }    
    
    static class BasenameFilter implements FilenameFilter {
        private String[] m_candidates;
        private Set m_matches = new TreeSet(new MostSpecificLast());
        BasenameFilter(String[] candidates) {
            m_candidates = candidates;
        }
                
        public boolean accept(File dir_, String name) {
            String glob = globToPattern(name);
            Pattern pattern = Pattern.compile(glob);
            for (int i = 0; i < m_candidates.length; i++) {
                if (pattern.matcher(m_candidates[i]).matches()) {
                    m_matches.add(name);
                    return true;
                    
                }
            }

            return false;
        }
        
        public String[] getMatches(File dir) {
            m_matches.clear();
            dir.list(this);
            return (String[]) m_matches.toArray(new String[m_matches.size()]);            
        }
    }    
}
