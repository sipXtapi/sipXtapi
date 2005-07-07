package beans;

import java.io.IOException;
import java.io.StringReader;

import junit.framework.TestCase;

import com.pingtel.commserver.beans.XmlEditor;

public class XmlEditorTest extends TestCase
{

    /**
     * Test that XmlEditor can handle valid xml properly
     */
    public void testValidXml()
        throws IOException
    {
        XmlEditor xml = new XmlEditor();
        StringBuffer content = new StringBuffer(1024);
        content.append("<?xml version=\"1.0\" ?>\n");
        content.append("<root></root>\n");
        xml.setContent(content.toString());
        assertTrue("valid xml single root content", xml.isValid());
        assertTrue("no error message", xml.getErrorMessage() == null);
        xml.save("XmlEditorTest.testValidXml.out");

        // test save/load
        XmlEditor xmlLoad = new XmlEditor();
        xmlLoad.load("XmlEditorTest.testValidXml.out");
        assertTrue("valid xml load single root content", xmlLoad.isValid());
        assertTrue("error message from loaded xml", xmlLoad.getErrorMessage() == null);
    }

    /**
     * Test that XmlEditor can handle invalid xml properly
     */
    public void testInvalidXml()
    {
        XmlEditor xml = new XmlEditor();
        StringBuffer content = new StringBuffer(1024);
        content.append("<?xml version=\"1.0\" ?>\n");
        content.append("<root>\n");
        xml.setContent(content.toString());
        assertTrue("invalid xml for a bad content", !xml.isValid());
        assertTrue("should be error message", xml.getErrorMessage() != null);

        try
        {
            xml.save("XmlEditorTest.testInvalidXml.out");
            fail("Xml editor should not let you save invalid xml");
        }
        catch (IOException expected)
        {
            assertTrue("Could not save invalid xml", true);
        }
    }

    /**
     * Test that XmlEditor can handle invalid xml properly
     */
    public void testEscapeXml()
    {
        String test = "&lt; kuku &gt;";
        String escaped = "&amp;lt; kuku &amp;gt;";
        assertEquals( escaped, XmlEditor.escapeForBrowser(test) );
    }

}
