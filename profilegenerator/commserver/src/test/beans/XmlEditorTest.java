package beans;

import junit.framework.TestCase;
import com.pingtel.commserver.beans.XmlEditor;
import java.io.IOException;

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
        xml.load("XmlEditorTest.testValidXml.out");
        assertTrue("valid xml load single root content", xml.isValid());
        assertTrue("error message from loaded xml", xml.getErrorMessage() == null);
    }

    /**
     * Test that XmlEditor can handle invalid xml properly
     */
    public void testInvalidXml()
        throws IOException
    {
        XmlEditor xml = new XmlEditor();
        StringBuffer content = new StringBuffer(1024);
        content.append("<?xml version=\"1.0\" ?>\n");
        content.append("<root>\n");
        xml.setContent(content.toString());
        assertTrue("invalid xml for a bad content", !xml.isValid());
        assertTrue("should be error message", xml.getErrorMessage() != null);
        xml.save("XmlEditorTest.testInvalidXml.out");

        // test save/load
        XmlEditor xmlLoad = new XmlEditor();
        xml.load("XmlEditorTest.testInvalidXml.out");
        assertTrue("valid xml load single root content", !xml.isValid());
        assertTrue("error message from loaded xml", xml.getErrorMessage() != null);
    }
}
