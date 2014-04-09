//
// Copyright (C) 2010-2013 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _OsAtomicsAndroid_H_ // [
#define _OsAtomicsAndroid_H_

#include <cutils/atomic.h>
#include <assert.h>

#define USE_GCC_ATOMICS_ON_ANDROID

#ifdef __x86_64__ // [
#  error 64-bit Android systems are not supported yet.
#endif // __x86_64__ ]

typedef enum memory_order {
   memory_order_relaxed, memory_order_consume, memory_order_acquire,
   memory_order_release, memory_order_acq_rel, memory_order_seq_cst
} memory_order;

template<class T>
class OsAtomic32
{
public:
   bool is_lock_free() const
   {return true;}

   void store(T val, memory_order order = memory_order_seq_cst)
   {
      if (order == memory_order_relaxed)
      {
         // AFAIK, on all architectures supported by Android write to
         // an aligned memory location is atomic.
         mVal = (int32_t)val;
      }
      else
      {
         // Atomic write with memory barrier.
         // Assert that we are on aligned memory location, hense this is atomic anyway
         //assert((((int)&mVal) & 0x3) == 0);
         // Android 2.3 and above has android_atomic_write #defined as android_atomic_release_store
         // Its not clear this is even needed.  So comment it out for now
#ifdef USE_GCC_ATOMICS_ON_ANDROID
         __sync_lock_test_and_set(&mVal, (int32_t)val);
#else
         android_atomic_write((int32_t)val, &mVal);
#endif
         //mVal = (int32_t)val;
      }
   }

   T load(memory_order = memory_order_seq_cst) const
   {
      // BUG: We should add memory barrier here if memory order is not "relaxed".
      return (T)mVal;
   }

   operator T() const
   {return load();}

   T exchange(T val, memory_order = memory_order_seq_cst)
   {
      return (T)android_atomic_swap((int32_t)val, &mVal);
   }

//   bool compare_exchange(T &, T , memory_order, memory_order);

   bool compare_exchange(T &expected, T desired, memory_order = memory_order_seq_cst)
   {
      if (android_atomic_cmpxchg((int32_t)expected, (int32_t)desired, &mVal) > 0)
      {
         expected = (T)mVal;
         return false;
      }
      return true;
   }

   void fence(memory_order) const
   { /* BUG: memory barrier is not implemented */ };

   T fetch_add(T val, memory_order = memory_order_seq_cst)
   {return (T)android_atomic_add((int32_t)val, &mVal);}

   T fetch_sub(T val, memory_order = memory_order_seq_cst)
   {return (T)android_atomic_add(-(int32_t)val, &mVal);}

   T fetch_and(T val, memory_order = memory_order_seq_cst)
   {return (T)android_atomic_and((int32_t)val, &mVal);}

   T fetch_or(T val, memory_order = memory_order_seq_cst)
   {return (T)android_atomic_or((int32_t)val, &mVal);}

//   T fetch_xor(T val, memory_order = memory_order_seq_cst);

   explicit OsAtomic32<T>(T val = 0) : mVal((int32_t)val) {};

   T operator=(T val)
   {store(val); return val;}

   T operator++(int)
   {
#ifdef USE_GCC_ATOMICS_ON_ANDROID
       return (T)__sync_fetch_and_add(&mVal, 1)+1;
#else
       return (T)android_atomic_inc(&mVal)+1;
#endif
   }

   T operator--(int)
   {
#ifdef USE_GCC_ATOMICS_ON_ANDROID
       return((T)__sync_fetch_and_sub(&mVal, 1)-1);
#else
       return((T)android_atomic_dec(&mVal)-1);
#endif
   }

   T operator++()
   {
#ifdef USE_GCC_ATOMICS_ON_ANDROID
       return((T)__sync_add_and_fetch(&mVal, 1));
#else
       return((T)android_atomic_inc(&mVal));
#endif
   }

   T operator--()
   {
#ifdef USE_GCC_ATOMICS_ON_ANDROID
       return((T)__sync_sub_and_fetch(&mVal, 1));
#else
       return((T)android_atomic_dec(&mVal));
#endif
   }

   T operator+=(T val)
   {return fetch_add(val)+val;}

   T operator-=(T val)
   {return fetch_sub(val)-val;}

   T operator&=(T val)
   {return fetch_and(val)&val;}

   T operator|=(T val)
   {return fetch_or(val)|val;}

//   T operator^=(T val);

protected:
   volatile int32_t mVal;

   // Prohibit use of copy constructor and operator=
   OsAtomic32<T>(const OsAtomic32<T>&);
   OsAtomic32<T>& operator=(const OsAtomic32<T>&);
};

typedef OsAtomic32<int> OsAtomicInt;
typedef OsAtomic32<unsigned int> OsAtomicUInt;
typedef OsAtomic32<long> OsAtomicLong;
typedef OsAtomic32<unsigned long> OsAtomicULong;

class OsAtomicVoidPtr : protected OsAtomic32<void*>
{
public:
   bool is_lock_free() const
   {return OsAtomic32<void*>::is_lock_free();}

   void store(void* val, memory_order order = memory_order_seq_cst)
   {OsAtomic32<void*>::store(val, order);}

   void* load(memory_order order = memory_order_seq_cst) const
   {return OsAtomic32<void*>::load(order);}

   operator void*() const
   {return load();}

   void* exchange(void* val, memory_order order = memory_order_seq_cst)
   {return OsAtomic32<void*>::exchange(val, order);}

//   bool compare_exchange(void* &, void* , memory_order, memory_order);

   bool compare_exchange(void* &expected, void* desired, memory_order order = memory_order_seq_cst)
   {return OsAtomic32<void*>::compare_exchange(expected, desired, order);}

   void fence(memory_order order) const
   {OsAtomic32<void*>::fence(order);}

   void* fetch_add(ptrdiff_t val, memory_order order = memory_order_seq_cst)
   {return (void*)android_atomic_add((int32_t)val, &mVal);}

   void* fetch_sub(ptrdiff_t val, memory_order order = memory_order_seq_cst)
   {return (void*)android_atomic_add(-(int32_t)val, &mVal);}

//   OsAtomicVoidPtr() : mMutex(0) {};

   explicit OsAtomicVoidPtr(void* val = NULL) : OsAtomic32<void*>(val) {};

   void* operator=(void* val)
   {return OsAtomic32<void*>::operator=(val);}

   void* operator+=(ptrdiff_t val)
   {return (char*)fetch_add(val)+val;}

   void* operator-=(ptrdiff_t val)
   {return (char*)fetch_sub(val)-val;}

private:

   // Prohibit use of copy constructor and operator=
   OsAtomicVoidPtr(const OsAtomicVoidPtr&);
   OsAtomicVoidPtr& operator=(const OsAtomicVoidPtr&);
};

template<class T>
class OsAtomicPtr : protected OsAtomicVoidPtr
{
public:
   bool is_lock_free() const
   {return OsAtomicVoidPtr::is_lock_free();}

   void store(T* val, memory_order mo = memory_order_seq_cst)
   {OsAtomicVoidPtr::store((void*)val, mo);}

   T* load(memory_order mo = memory_order_seq_cst) const
   {return (T*)OsAtomicVoidPtr::load(mo);}

   operator T*() const
   {return (T*)OsAtomicVoidPtr::operator void*();}

   T* exchange(T* val, memory_order mo = memory_order_seq_cst)
   {return (T*)OsAtomicVoidPtr::exchange((void*)val, mo);}

//   bool compare_exchange(T* &, T* , memory_order mo, memory_order mo);

   bool compare_exchange(T* &expected, T* desired, memory_order mo = memory_order_seq_cst)
   {return (T*)OsAtomicVoidPtr::compare_exchange((void*&)expected, (void*)desired, mo);}

   void fence(memory_order mo) const
   {OsAtomicVoidPtr::fence(mo);}

   T* fetch_add(ptrdiff_t val, memory_order mo = memory_order_seq_cst)
   {return (T*)OsAtomicVoidPtr::fetch_add(val, mo);}

   T* fetch_sub(ptrdiff_t val, memory_order mo = memory_order_seq_cst)
   {return (T*)OsAtomicVoidPtr::fetch_sub(val, mo);}

//   OsAtomicPtr() {};

   explicit OsAtomicPtr(T* val = NULL) : OsAtomicVoidPtr((void*)val) {};

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

#define OsAtomicLight OsAtomic32
#define OsAtomicLightPtr OsAtomicPtr

typedef OsAtomicLight<int> OsAtomicLightInt;
typedef OsAtomicLight<unsigned int> OsAtomicLightUInt;
typedef OsAtomicLight<long> OsAtomicLightLong;
typedef OsAtomicLight<unsigned long> OsAtomicLightULong;
typedef OsAtomicLight<bool> OsAtomicLightBool;

#endif // _OsAtomicsAndroid_H_ ]
