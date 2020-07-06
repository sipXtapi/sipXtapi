//
// Copyright (C) 2008-2020 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _OsAtomics_H_ // [
#define _OsAtomics_H_

#ifdef HAVE_C_ATOMICS // [
#  include <cstdatomic>

typedef atomic_int OsAtomicInt;
typedef atomic_uint OsAtomicUInt;
typedef atomic_long OsAtomicLong;
typedef atomic_ulong OsAtomicULong;
typedef atomic_address OsAtomicVoidPtr;

#define OsAtomicLight OsAtomic

#elif defined(ANDROID) // HAVE_C_ATOMICS ][ ANDROID

#include <os/OsAtomicsAndroid.h>

#else // ANDROID ][

#ifdef __arm__
#warning ARM version of OsAtomics need to be fixed to support ARM features.
#endif

#  include <os/OsLock.h>
#  include <os/OsMutex.h>
#  include <stddef.h>
#  include <os/OsIntTypes.h>

typedef enum memory_order {
   memory_order_relaxed, memory_order_consume, memory_order_acquire,
   memory_order_release, memory_order_acq_rel, memory_order_seq_cst
} memory_order;

template<class T>
class OsAtomic
{
public:
   bool is_lock_free() const
   {return false;}

   void store(T val, ::memory_order  = ::memory_order_seq_cst)
   {OsLock lock(mMutex); mVal = val;}

   T load(::memory_order = ::memory_order_seq_cst) const
   {OsLock lock(mMutex); return mVal;}

   operator T() const
   {return load();}

//   T exchange(T val, ::memory_order = memory_order_seq_cst);
//   bool compare_exchange(T &, T , ::memory_order, ::memory_order);
//   bool compare_exchange(T &, T , ::memory_order = memory_order_seq_cst);

   void fence(::memory_order) const
   {};

   T fetch_add(T val, ::memory_order = ::memory_order_seq_cst)
   {OsLock lock(mMutex); T temp = mVal; mVal += val; return temp;}

   T fetch_sub(T val, ::memory_order = ::memory_order_seq_cst)
   {OsLock lock(mMutex); T temp = mVal; mVal -= val; return temp;}

   T fetch_and(T val, ::memory_order = ::memory_order_seq_cst)
   {OsLock lock(mMutex); T temp = mVal; mVal &= val; return temp;}

   T fetch_or(T val, ::memory_order = ::memory_order_seq_cst)
   {OsLock lock(mMutex); T temp = mVal; mVal |= val; return temp;}

   T fetch_xor(T val, ::memory_order = ::memory_order_seq_cst)
   {OsLock lock(mMutex); T temp = mVal; mVal ^= val; return temp;}

   OsAtomic<T>() : mMutex(0) {};

   explicit OsAtomic<T>(T val) : mVal(val), mMutex(0) {};

   T operator=(T val)
   {store(val); return val;}

   T operator++(int)
   {return fetch_add(1);}

   T operator--(int)
   {return fetch_sub(1);}

   T operator++()
   {return operator+=(1);}

   T operator--()
   {return operator-=(1);}

   T operator+=(T val)
   {OsLock lock(mMutex); mVal += val; return mVal;}

   T operator-=(T val)
   {OsLock lock(mMutex); mVal -= val; return mVal;}

   T operator&=(T val)
   {OsLock lock(mMutex); mVal &= val; return mVal;}

   T operator|=(T val)
   {OsLock lock(mMutex); mVal |= val; return mVal;}

   T operator^=(T val)
   {OsLock lock(mMutex); mVal ^= val; return mVal;}

private:
   T       mVal;
   mutable OsMutex mMutex;

   // Prohibit use of copy constructor and operator=
   OsAtomic<T>(const OsAtomic<T>&);
   OsAtomic<T>& operator=(const OsAtomic<T>&);
};

typedef OsAtomic<int> OsAtomicInt;
typedef OsAtomic<unsigned int> OsAtomicUInt;
typedef OsAtomic<long> OsAtomicLong;
typedef OsAtomic<unsigned long> OsAtomicULong;

class OsAtomicBool {
public:
   bool is_lock_free() const
   {return false;}

   OsAtomicBool() : mMutex(0) {};

   explicit OsAtomicBool(bool val) : mVal(val), mMutex(0) {};

   void store(bool val, ::memory_order  = ::memory_order_seq_cst)
   {OsLock lock(mMutex); mVal = val;}

   bool load(::memory_order = ::memory_order_seq_cst) const
   {OsLock lock(mMutex); return mVal;}

   bool operator=(bool val)
   {store(val); return val;}

   operator bool() const
   {return load();}

//   bool exchange(bool, ::memory_order = ::memory_order_seq_cst);
//   bool compare_exchange(bool&, bool, ::memory_order, ::memory_order);
//   bool compare_exchange(bool&, bool, ::memory_order = ::memory_order_seq_cst);

   void fence(::memory_order) const
   {}

private:
   bool     mVal;
   mutable OsMutex mMutex;

   // Prohibit use of copy constructor and operator=
   OsAtomicBool(const OsAtomicBool&);
   OsAtomicBool& operator=(const OsAtomicBool&);
};

class OsAtomicVoidPtr
{
public:
   bool is_lock_free() const
   {return false;}

   void store(void* val, ::memory_order  = ::memory_order_seq_cst)
   {OsLock lock(mMutex); mVal = val;}

   void* load(::memory_order = ::memory_order_seq_cst) const
   {OsLock lock(mMutex); return mVal;}

   operator void*() const
   {return load();}

//   void* exchange(void* val, ::memory_order = ::memory_order_seq_cst);
//   bool compare_exchange(void* &, void* , ::memory_order, ::memory_order);
//   bool compare_exchange(void* &, void* , ::memory_order = ::memory_order_seq_cst);

   void fence(::memory_order) const
   {};

   void* fetch_add(ptrdiff_t val, ::memory_order = ::memory_order_seq_cst)
   {OsLock lock(mMutex); void* temp = mVal; mVal = (int8_t*)mVal + val; return temp;}

   void* fetch_sub(ptrdiff_t val, ::memory_order = ::memory_order_seq_cst)
   {OsLock lock(mMutex); void* temp = mVal; mVal = (int8_t*)mVal - val; return temp;}

   OsAtomicVoidPtr() : mMutex(0) {};

   explicit OsAtomicVoidPtr(void* val) : mVal(val), mMutex(0) {};

   void* operator=(void* val)
   {store(val); return val;}

   void* operator+=(ptrdiff_t val)
   {OsLock lock(mMutex); mVal = (int8_t*)mVal + val; return mVal;}

   void* operator-=(ptrdiff_t val)
   {OsLock lock(mMutex); mVal = (int8_t*)mVal - val; return mVal;}

private:
   void*       mVal;
   mutable OsMutex mMutex;

   // Prohibit use of copy constructor and operator=
   OsAtomicVoidPtr(const OsAtomicVoidPtr&);
   OsAtomicVoidPtr& operator=(const OsAtomicVoidPtr&);
};

template<class T>
class OsAtomicPtr : public OsAtomicVoidPtr
{
public:
   bool is_lock_free() const
   {return false;}

   void store(T* val, ::memory_order mo = ::memory_order_seq_cst)
   {OsAtomicVoidPtr::store((void*)val, mo);}

   T* load(::memory_order mo = ::memory_order_seq_cst) const
   {return (T*)OsAtomicVoidPtr::load(mo);}

   operator T*() const
   {return (T*)OsAtomicVoidPtr::operator void*();}

//   T* exchange(T* val, ::memory_order mo = ::memory_order_seq_cst);
//   bool compare_exchange(T* &, T* , ::memory_order mo, ::memory_order mo);
//   bool compare_exchange(T* &, T* , ::memory_order mo = ::memory_order_seq_cst);

   void fence(::memory_order mo) const
   {OsAtomicVoidPtr::fence(mo);}

   T* fetch_add(ptrdiff_t val, ::memory_order mo = ::memory_order_seq_cst)
   {return (T*)OsAtomicVoidPtr::fetch_add(val, mo);}

   T* fetch_sub(ptrdiff_t val, ::memory_order mo = ::memory_order_seq_cst)
   {return (T*)OsAtomicVoidPtr::fetch_sub(val, mo);}

   OsAtomicPtr() {};

   explicit OsAtomicPtr(T* val) : OsAtomicVoidPtr((void*)val) {};

   T* operator=(T* val)
   {return (T*)OsAtomicVoidPtr::operator=((void*)val);}

   T* operator+=(ptrdiff_t val)
   {return (T*)OsAtomicVoidPtr::operator+=(val);}

   T* operator-=(ptrdiff_t val)
   {return (T*)OsAtomicVoidPtr::operator-=(val);}

private:
   // Prohibit use of copy constructor and operator=
   OsAtomicPtr(const OsAtomicPtr&);
   OsAtomicPtr& operator=(const OsAtomicPtr&);
};


#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__amd64__) || defined (__arm__)// [
template<class T>
class OsAtomicLight
{
public:
   bool is_lock_free() const
   {return true;}

   void store(T val, ::memory_order  = ::memory_order_relaxed)
   {mVal = val;}

   T load(::memory_order = ::memory_order_relaxed) const
   {return mVal;}

   operator T() const
   {return load();}

   void fence(::memory_order) const
   {};

   OsAtomicLight<T>() {};

   explicit OsAtomicLight<T>(T val) : mVal(val) {};

   T operator=(T val)
   {store(val); return val;}

private:
   volatile T       mVal;

   // Prohibit use of copy constructor and operator=
   OsAtomicLight<T>(const OsAtomicLight<T>&);
   OsAtomicLight<T>& operator=(const OsAtomicLight<T>&);
};


template<class T>
class OsAtomicLightPtr
{
public:
   bool is_lock_free() const
   {return true;}

   void store(T* val, ::memory_order mo = ::memory_order_relaxed)
   {mVal = val;}

   const T* load(::memory_order mo = ::memory_order_relaxed) const
   {return mVal;}

   T* load(::memory_order mo = ::memory_order_relaxed)
   {return mVal;}

   operator const T*() const
   {return load();}

   operator T*()
   {return load();}

   void fence(::memory_order mo) const
   {}

   OsAtomicLightPtr() {};

   explicit OsAtomicLightPtr(T* val) : mVal(val) {};

   T* operator=(T* val)
   {store(val); return val;}

private:
   T * volatile mVal;

   // Prohibit use of copy constructor and operator=
   OsAtomicLightPtr(const OsAtomicLightPtr&);
   OsAtomicLightPtr& operator=(const OsAtomicLightPtr&);
};

#else // X86/X86_64/ARM ][

#define OsAtomicLight OsAtomic
#define OsAtomicLightPtr OsAtomicPtr

#endif // !X86/X86_64/ARM

#endif // HAVE_C_ATOMICS ]

typedef OsAtomicLight<int> OsAtomicLightInt;
typedef OsAtomicLight<unsigned int> OsAtomicLightUInt;
typedef OsAtomicLight<long> OsAtomicLightLong;
typedef OsAtomicLight<unsigned long> OsAtomicLightULong;
typedef OsAtomicLight<bool> OsAtomicLightBool;

#endif // _OsAtomics_H_ ]
