//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifdef HAVE_NSS

#include "os/OsStatus.h"
#include "os/OsTLS.h"
#include "net/pk12wrapper.h"

#define PKCS12_IN_BUFFER_SIZE	200

static SECStatus p12u_SwapUnicodeBytes(SECItem *uniItem)
{
    unsigned int i;
    unsigned char a;
    
    if ((uniItem == NULL) || (uniItem->len % 2))
    {
	    return SECFailure;
    }
    
    for(i = 0; i < uniItem->len; i += 2)
    {
	    a = uniItem->data[i];
	    uniItem->data[i] = uniItem->data[i+1];
	    uniItem->data[i+1] = a;
    }

    return SECSuccess;
}

static PRBool _P12Wrapper_UnicodeConversion(PRBool toUnicode,
				    unsigned char *inBuf,
				    unsigned int   inBufLen,
				    unsigned char *outBuf,
				    unsigned int   maxOutBufLen,
				    unsigned int  *outBufLen,
				    PRBool	   swapBytes)
{
    SECItem it;
    SECItem *dup = NULL;
    PRBool ret;

    it.data = inBuf;
    it.len = inBufLen;
    dup = SECITEM_DupItem(&it);

    if (!toUnicode && swapBytes)
    {
	    if (p12u_SwapUnicodeBytes(dup) != SECSuccess)
	    {
	        SECITEM_ZfreeItem(dup, PR_TRUE);
	        return PR_FALSE;
	    }
    }
    
    ret = PORT_UCS2_UTF8Conversion(toUnicode, dup->data, dup->len,
                                   outBuf, maxOutBufLen, outBufLen);
    if (dup)
    {
    	SECITEM_ZfreeItem(dup, PR_TRUE);
    }
    return ret;
}



SECStatus P12Wrapper_UnicodeConversion(PRArenaPool *arena,
                             SECItem *dest,
                             SECItem *src,
		                     PRBool toUnicode,
		                     PRBool swapBytes)
{
    unsigned int len = 0;
    
    if(NULL == src || NULL == dest)
    {
	    return SECFailure;
    }
    
    if (toUnicode)
    {
        len = src->len << 2;
    }
    else
    {
        len = src->len;
    }
    
    if(arena)
    {
	    dest->data = (unsigned char*)PORT_ArenaZAlloc(arena, len);
    }
    else
    {
	    dest->data = (unsigned char*)PORT_ZAlloc(len);
    }
    
    if(PR_FALSE == 
       PORT_UCS2_ASCIIConversion(toUnicode,
                                 src->data,
                                 src->len,
				                 dest->data,
				                 len,
				                 &dest->len,
				                 swapBytes) )
   {
	    if(NULL == arena)
	    {
	        PORT_Free(dest->data);
	    }
	    dest->data = NULL;
	    return SECFailure;
    }
    return SECSuccess;
}


bool P12Wrapper_Init(char *dir, char *dbprefix)
{
    bool bRet = false;

    if (OS_SUCCESS == OsTLS::Initialize(dir))
    {
        bRet = true;
    }
    return bRet;
}


SECItem * NicknameCollisionCallback(SECItem *old_nick, PRBool *cancel, void *wincx)
{
    char *nick = NULL;
    SECItem *ret_nick = NULL;

    if(cancel == NULL)
    {
      return NULL;
    }
    *cancel = PR_TRUE;
    return NULL;
}

SECStatus P12Wrapper_InitializeSlot(PK11SlotInfo *slot, SECItem *slotPw)
{
    if (PK11_CheckUserPassword(slot, (char*)slotPw->data) != SECSuccess)
    {
        return SECFailure;
    }

    if (PK11_Authenticate(slot, PR_TRUE, slotPw) != SECSuccess)
    {
    	return SECFailure;
    }

    return SECSuccess;
}

SECStatus P12Wrapper_ImportPKCS12ObjectFromBuffer(char *pkcs12Buffer, int pkcs12Size, PK11SlotInfo *slot,
			SECItem *slotPw, SECItem *p12FilePw)
{
    SEC_PKCS12DecoderContext *p12dcx = NULL;
    SECItem uniPwitem;
    SECItem *pwItem = NULL;
    SECStatus rv = SECFailure;
    bool retVal = false;
    PRBool swapUnicode = PR_FALSE;
    int error;
    memset(&uniPwitem, 0, sizeof(SECItem));
    char *p0 = NULL;

#ifdef IS_LITTLE_ENDIAN
    swapUnicode = PR_TRUE;
#endif

    rv = P12Wrapper_InitializeSlot(slot, slotPw);
    
//    PK11_SetPrivateKeyNickname(SECKEYPrivateKey *privKey, nickname);
    
    if (SECSuccess != rv)
    {
	    goto cleanup;
    }

	p0 = (char*)p12FilePw->data;
    pwItem = SECITEM_AllocItem(NULL, NULL, PL_strlen(p0) + 1);
    memcpy(pwItem->data, p0, pwItem->len);
    PORT_Memset(p0, 0, PL_strlen(p0));
    PORT_Free(p0);

    /* convert the password */
    if(P12Wrapper_UnicodeConversion(NULL, &uniPwitem, pwItem, PR_TRUE, swapUnicode) != SECSuccess)
    {
	    goto cleanup;
    }


    /* init the decoder context */
    p12dcx = SEC_PKCS12DecoderStart(&uniPwitem, slot, slotPw,
				    NULL, NULL, NULL, NULL, NULL);
    if(!p12dcx)
    {
	    goto cleanup;
    }
   
    rv = SEC_PKCS12DecoderUpdate(p12dcx, (unsigned char *)pkcs12Buffer, pkcs12Size);

    if(rv != SECSuccess)
    {
	    error = PR_GetError();
	
	    if(error == SEC_ERROR_DECRYPTION_DISALLOWED)
	    {
	        PR_SetError(error, 0);
	        goto cleanup;
	    }
    }

    rv = SECFailure;

    if(SEC_PKCS12DecoderVerify(p12dcx) != SECSuccess)
    {
    	goto cleanup;
    }

    if (SECSuccess != SEC_PKCS12DecoderValidateBags(p12dcx, NicknameCollisionCallback))
    {
    	goto cleanup;
    }

    rv = SEC_PKCS12DecoderImportBags(p12dcx);
    if(SECSuccess != rv)
    {
	    error = PR_GetError();
    	goto cleanup;
    }

    rv = SECSuccess;

cleanup:
    if (p12dcx)
    {
	    SEC_PKCS12DecoderFinish(p12dcx);
    }

    if (uniPwitem.data)
    {
	    SECITEM_ZfreeItem(&uniPwitem, PR_FALSE);
    }

    if (pwItem)
    {
	    SECITEM_ZfreeItem(pwItem, PR_TRUE);
    }

    return rv;
}

#endif