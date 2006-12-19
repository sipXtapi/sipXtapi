// Functions for extracting the text content of an XML element.

#include "xmlparser/ExtractContent.h"

void textContentShallow(UtlString& string,
                        TiXmlElement *element)
{
   // Clear the string.
   string.remove(0);

   // Iterate through all the children.
   for (TiXmlNode* child = element->FirstChild(); child;
        child = child->NextSibling())
   {
      // Examine the text nodes.
      if (child->Type() == TiXmlNode::TEXT)
      {
         // Append the content to the string.
         string.append(child->Value());
      }
   }
}

void textContentDeep(UtlString& string,
                     TiXmlElement *element)
{
   // Clear the string.
   string.remove(0);

   // Recurse into the XML.
   textContentDeepRecursive(string, element);
}

void textContentDeepRecursive(UtlString& string,
                              TiXmlElement *element)
{
   // Iterate through all the children.
   for (TiXmlNode* child = element->FirstChild(); child;
        child = child->NextSibling())
   {
      // Examine the text nodes.
      if (child->Type() == TiXmlNode::TEXT)
      {
         // Append the content to the string.
         string.append(child->Value());
      }
      else if (child->Type() == TiXmlNode::ELEMENT)
      {
         // Recurse on this element.
         textContentDeepRecursive(string, child->ToElement());
      }
   }
}
