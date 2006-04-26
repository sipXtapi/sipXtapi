//
// BEGIN SIPFOUNDRY:
//  Added public settor for maxSize, see 
//  https://issues.apache.org/jira/browse/TAPESTRY-368
// END SIPFOUNDRY:
//

// Copyright 2005 The Apache Software Foundation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package org.apache.tapestry.multipart;

import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;

import org.apache.commons.fileupload.DiskFileUpload;
import org.apache.commons.fileupload.FileItem;
import org.apache.commons.fileupload.FileUploadException;
import org.apache.hivemind.ApplicationRuntimeException;
import org.apache.tapestry.request.IUploadFile;

/**
 * Implementation of {@link org.apache.tapestry.multipart.MultipartDecoder}that is based on <a
 * href="http://jakarta.apache.org/commons/fileupload/">Jakarta FileUpload </a>.
 * 
 * @author Howard M. Lewis Ship
 * @author Joe Panico
 * @since 4.0
 */
public class MultipartDecoderImpl_Bug368 implements MultipartDecoder
{
    /**
     * Map of UploadPart (which implements IUploadFile), keyed on parameter name.
     */

    private Map _uploadParts = new HashMap();

    /**
     * Map of ValuePart, keyed on parameter name.
     */
    private Map _valueParts = new HashMap();

    private int _maxSize = 10000000;

    private int _thresholdSize = 1024;

    private String _repositoryPath = System.getProperty("java.io.tmpdir");

    private String _encoding;

    public HttpServletRequest decode(HttpServletRequest request)
    {
        _encoding = request.getCharacterEncoding();

        DiskFileUpload upload = createUpload(request);

        try
        {
            List fileItems = upload
                    .parseRequest(request, _thresholdSize, _maxSize, _repositoryPath);

            processFileItems(fileItems);
        }
        catch (FileUploadException ex)
        {
            throw new ApplicationRuntimeException(MultipartMessages.unableToDecode(ex), ex);
        }

        Map parameterMap = buildParameterMap();

        return new UploadFormParametersWrapper(request, parameterMap);
    }
    
    //  BEGIN SIPFOUNDRY:
    public void setMaxSize(int maxSize) {
        _maxSize = maxSize;
    }
    //  END SIPFOUNDRY:

    private Map buildParameterMap()
    {
        Map result = new HashMap();

        Iterator i = _valueParts.entrySet().iterator();
        while (i.hasNext())
        {
            Map.Entry e = (Map.Entry) i.next();

            String name = (String) e.getKey();
            ValuePart part = (ValuePart) e.getValue();

            result.put(name, part.getValues());
        }

        return result;
    }

    private void processFileItems(List parts)
    {
        if (parts == null)
            return;

        Iterator i = parts.iterator();

        while (i.hasNext())
        {
            FileItem item = (FileItem) i.next();

            processFileItem(item);
        }
    }

    private void processFileItem(FileItem item)
    {
        if (item.isFormField())
        {
            processFormFieldItem(item);
            return;
        }

        processUploadFileItem(item);
    }

    private void processUploadFileItem(FileItem item)
    {
        String name = item.getFieldName();

        UploadPart part = new UploadPart(item);

        _uploadParts.put(name, part);
    }

    void processFormFieldItem(FileItem item)
    {
        String name = item.getFieldName();

        String value = extractFileItemValue(item);

        ValuePart part = (ValuePart) _valueParts.get(name);

        if (part == null)
            _valueParts.put(name, new ValuePart(value));
        else
            part.add(value);
    }

    private String extractFileItemValue(FileItem item)
    {
        try
        {
            return (_encoding == null) ? item.getString() : item.getString(_encoding);
        }
        catch (UnsupportedEncodingException ex)
        {
            throw new ApplicationRuntimeException(MultipartMessages.unsupportedEncoding(
                    _encoding,
                    ex), ex);
        }
    }

    protected DiskFileUpload createUpload(HttpServletRequest request)
    {
        DiskFileUpload upload = new DiskFileUpload();

        if (_encoding != null)
            upload.setHeaderEncoding(_encoding);

        return upload;
    }

    public IUploadFile getFileUpload(String parameterName)
    {
        return (IUploadFile) _uploadParts.get(parameterName);
    }

    public void cleanup()
    {
        Iterator i = _uploadParts.values().iterator();

        while (i.hasNext())
        {
            UploadPart part = (UploadPart) i.next();

            part.cleanup();
        }
    }

}