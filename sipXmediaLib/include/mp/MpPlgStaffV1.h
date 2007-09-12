//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: 

#ifndef _PlgStaff_h_
#define _PlgStaff_h_

#include <utl/UtlInt.h>
#include <utl/UtlVoidPtr.h>
#include <utl/UtlString.h>
#include <mp/plugins/PlgDefsV1.h>

class MpStaticCodecSimpleList {
private:
   MpStaticCodecSimpleList* mNext; //for make static list in .ctor section without using sipXportLib classes to prevent any problems of initialization static variables
public:
   MpStaticCodecSimpleList() : mNext(NULL) {}
   // Only bounding in chain is needed for compiled-in codecs
   MpStaticCodecSimpleList* bound(MpStaticCodecSimpleList* newItem) { this->mNext = newItem; return this; }
   UtlBoolean iseof() const { return mNext == NULL; }

   MpStaticCodecSimpleList* next() const { return mNext; }
};

class MpCodecCallInfoV1 : protected UtlVoidPtr , public MpStaticCodecSimpleList
{
   friend class MpCodecFactory;
   friend class MpPlgDecoderWrapper;
   friend class MpPlgEncoderWrapper;

protected:
   UtlBoolean mbStatic;   // compiled-in or loaded codec
   UtlString mModuleName; // loaded module name or nothing in compiled-in codecs

   UtlString mCodecModuleName; // prefix in module names
   dlPlgInitV1 mPlgInit;
   dlPlgDecodeV1 mPlgDecode;
   dlPlgEncodeV1 mPlgEncode;
   dlPlgFreeV1 mPlgFree;
   dlPlgEnumSDPAndModesV1 mPlgEnum;
   dlPlgGetSignalingDataV1 mPlgSignaling;
   
   UtlBoolean fSDPassigned;
   int sdpNum;
public:
   const char** getSDPModes(unsigned& modesCount);

protected:
   MpCodecCallInfoV1() : fSDPassigned(FALSE), sdpNum(-1) {}

public:
   MpCodecCallInfoV1* next() const { return (MpCodecCallInfoV1*)MpStaticCodecSimpleList::next(); }

   MpCodecCallInfoV1(const char* moduleName,
      const char* codecModuleName,
      dlPlgInitV1 plgInit,
      dlPlgDecodeV1 plgDecode,
      dlPlgEncodeV1 plgEncode,
      dlPlgFreeV1 plgFree,
      dlPlgEnumSDPAndModesV1 plgEnum,
      dlPlgGetSignalingDataV1 plgSignaling,
      UtlBoolean bStatic = TRUE) 
      : mModuleName(moduleName)
      , mCodecModuleName(codecModuleName)
      , mPlgInit(plgInit)
      , mPlgDecode(plgDecode)
      , mPlgEncode(plgEncode)
      , mPlgFree(plgFree)
      , mPlgEnum(plgEnum)
      , mPlgSignaling(plgSignaling)
      , mbStatic(bStatic)
      , fSDPassigned(FALSE)
      , sdpNum(-1)
   {}

   static void registerStaticCodec(const char* moduleName
      , const char* codecModuleName
      , dlPlgInitV1 plgInit
      , dlPlgDecodeV1 plgDecode
      , dlPlgEncodeV1 plgEncode
      , dlPlgFreeV1 plgFree
      , dlPlgEnumSDPAndModesV1 plgEnum
      , dlPlgGetSignalingDataV1 plgSignaling
      );
};




class AutoRegistratorBase
{
public:
   AutoRegistratorBase();
protected:
   static int nAutoRegistered;
};


#define MAKE_MP_UNIQUE_NAME(prefix) JOIN( prefix, __LINE__ )
#define JOIN( symbol1, symbol2 ) __DO_JOIN( symbol1, symbol2 )
#define __DO_JOIN( symbol1, symbol2 ) __DO_JOIN2( symbol1, symbol2 )
#define __DO_JOIN2( symbol1, symbol2 ) symbol1##symbol2

#define DECLARE_MP_STATIC_PLUGIN_CODEC_V1_(x,y)       \
class _plg_reg_class_##y :                            \
               public AutoRegistratorBase {           \
public:   int x;                                            \
   _plg_reg_class_##y ()  {                           \
   MpCodecCallInfoV1::registerStaticCodec(            \
   __FILE__,                                          \
   #x,                                                \
   PLG_INIT_V1(x),                                    \
   PLG_DECODE_V1(x),                                  \
   PLG_ENCODE_V1(x),                                  \
   PLG_FREE_V1(x),                                    \
   PLG_ENUM_V1(x)                                     \
   );	  x = (int)this;                                              \
}                                                     \
};                                                    \
   static _plg_reg_class_##y  _dummy_var_##x;   \
   extern "C" int dummy_func_##x() {return _dummy_var_##x.x; }


#define DECLARE_MP_STATIC_PLUGIN_CODEC_V1I(x,y) \
   DECLARE_MP_STATIC_PLUGIN_CODEC_V1_(x,y)

#define DECLARE_MP_STATIC_PLUGIN_CODEC_V1(x) \
   DECLARE_MP_STATIC_PLUGIN_CODEC_V1I(x, MAKE_MP_UNIQUE_NAME(x))




#endif //_PlgStaff_h_
