

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Mon Nov 07 12:44:25 2005
 */
/* Compiler settings for _SampleGrabberCallback.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef ___SampleGrabberCallback_h__
#define ___SampleGrabberCallback_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ISampGrabCB_FWD_DEFINED__
#define __ISampGrabCB_FWD_DEFINED__
typedef interface ISampGrabCB ISampGrabCB;
#endif 	/* __ISampGrabCB_FWD_DEFINED__ */


#ifndef ___ISampGrabCBEvents_FWD_DEFINED__
#define ___ISampGrabCBEvents_FWD_DEFINED__
typedef interface _ISampGrabCBEvents _ISampGrabCBEvents;
#endif 	/* ___ISampGrabCBEvents_FWD_DEFINED__ */


#ifndef __CSampGrabCB_FWD_DEFINED__
#define __CSampGrabCB_FWD_DEFINED__

#ifdef __cplusplus
typedef class CSampGrabCB CSampGrabCB;
#else
typedef struct CSampGrabCB CSampGrabCB;
#endif /* __cplusplus */

#endif 	/* __CSampGrabCB_FWD_DEFINED__ */


/* header files for imported files */
#include "prsht.h"
#include "mshtml.h"
#include "mshtmhst.h"
#include "exdisp.h"
#include "objsafe.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __ISampGrabCB_INTERFACE_DEFINED__
#define __ISampGrabCB_INTERFACE_DEFINED__

/* interface ISampGrabCB */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_ISampGrabCB;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("50C3C252-E279-4767-AF54-4B3D5F922C19")
    ISampGrabCB : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetWindow( 
            /* [in] */ LONG hWnd) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISampGrabCBVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISampGrabCB * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISampGrabCB * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISampGrabCB * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISampGrabCB * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISampGrabCB * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISampGrabCB * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISampGrabCB * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetWindow )( 
            ISampGrabCB * This,
            /* [in] */ LONG hWnd);
        
        END_INTERFACE
    } ISampGrabCBVtbl;

    interface ISampGrabCB
    {
        CONST_VTBL struct ISampGrabCBVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISampGrabCB_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISampGrabCB_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISampGrabCB_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISampGrabCB_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISampGrabCB_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISampGrabCB_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISampGrabCB_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISampGrabCB_SetWindow(This,hWnd)	\
    (This)->lpVtbl -> SetWindow(This,hWnd)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ISampGrabCB_SetWindow_Proxy( 
    ISampGrabCB * This,
    /* [in] */ LONG hWnd);


void __RPC_STUB ISampGrabCB_SetWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISampGrabCB_INTERFACE_DEFINED__ */



#ifndef __SampleGrabberCallback_LIBRARY_DEFINED__
#define __SampleGrabberCallback_LIBRARY_DEFINED__

/* library SampleGrabberCallback */
/* [helpstring][uuid][version] */ 


EXTERN_C const IID LIBID_SampleGrabberCallback;

#ifndef ___ISampGrabCBEvents_DISPINTERFACE_DEFINED__
#define ___ISampGrabCBEvents_DISPINTERFACE_DEFINED__

/* dispinterface _ISampGrabCBEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__ISampGrabCBEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("D2975F1B-7D42-4B1A-AA47-23D6437364A3")
    _ISampGrabCBEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _ISampGrabCBEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ISampGrabCBEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ISampGrabCBEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ISampGrabCBEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ISampGrabCBEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ISampGrabCBEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ISampGrabCBEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ISampGrabCBEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _ISampGrabCBEventsVtbl;

    interface _ISampGrabCBEvents
    {
        CONST_VTBL struct _ISampGrabCBEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _ISampGrabCBEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _ISampGrabCBEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _ISampGrabCBEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _ISampGrabCBEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _ISampGrabCBEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _ISampGrabCBEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _ISampGrabCBEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___ISampGrabCBEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_CSampGrabCB;

#ifdef __cplusplus

class DECLSPEC_UUID("31576FBD-7081-4D6D-9A08-9B94C213E653")
CSampGrabCB;
#endif
#endif /* __SampleGrabberCallback_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


