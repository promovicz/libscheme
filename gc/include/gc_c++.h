
/****************************************************************************

Copyright (c) 1994 by Xerox Corporation.  All rights reserved.
 
THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 
Permission is hereby granted to use or copy this program
for any purpose,  provided the above notices are retained on all copies.
Permission to modify the code and to distribute modified code is granted,
provided the above notices are retained, and a notice that the code was
modified is included with the above copyright notice.

C++ Interface to the Boehm Collector

    Jesse Hull and John Ellis
    Last modified on Tue Feb 15 14:43:02 PST 1994 by ellis

This interface provides access to the Boehm collector (versions 3.6
and later).  It is intended to provide facilities similar to those
described in the Ellis-Detlefs proposal for C++ garbage collection.

To make a class collectable, derive it from the base class "gc":

    class MyClass: gc {...}

Then, "new MyClass" will allocate intances that will be automatically
garbage collected.

Collected objects can be explicitly deleted with "delete", e.g.

    MyClass* m = ...;
    delete m;

This will free the object's storage immediately.

Collected instances of non-class types can be allocated using
placement syntax with the argument "GC":

    typedef int A[ 10 ];
    A* a = new (GC) A;

The built-in "operator new" continues to allocate non-collectible
objects that the programmer must explicitly delete.  Collected object
may freely point at non-collected objects, and vice versa.

Object clean-up (finalization) can be specified using class
"gc_cleanup".  When an object derived from "gc_cleanup" is discovered
to be inaccessible by the collector, or when it is explicitly deleted,
its destructors will be invoked first.

Clean-up functions for non-class types can be specified as additional
placement arguments:

    A* a = new (GC, MyCleanup) A;

An object is considered "accessible" by the collector if it can be
reached by a path of pointers from static variables, automatic
variables of active functions, or from another object with clean-up
enabled.  This implies that if object A and B both have clean-up
enabled, and A points at B, B will be considered accessible, and A's
clean-up will be be invoked before B's.  If A points at B and B points
back to A, forming a cycle, that's considered a storage leak, and
neither will ever become inaccessible.  See the C interface gc.h for
low-level facilities for handling such cycles of objects with cleanup.

Cautions:
1. Arrays allocated without new placement syntax are
allocated as uncollectable objects.  They are traced by the
collector, but will not be reclaimed.

2. Be sure the collector has been augmented with "make c++".

3. If your compiler supports an overloaded new[] operator,
then gc_c++.cc and gc_c++.h should be suitably modified.

****************************************************************************/

#ifndef GC_CPP_H
#define GC_CPP_H

extern "C" {
#include "gc.h"
}

enum GCPlacement {GC, NoGC};

class gc {
public:
    inline void* operator new( size_t size );
    inline void* operator new( size_t size, GCPlacement gcp );
    inline void operator delete( void* obj );  };
    /*
    Intances of classes derived from "gc" will be allocated in the 
    collected heap by default, unless an explicit NoGC placement is
    specified. */

class gc_cleanup: public gc {
public:
    inline gc_cleanup();
    inline virtual ~gc_cleanup();
private:
    inline static void cleanup( void* obj, void* clientData ); };
    /*
    Instances of classes derived from "gc_cleanup" will be allocated
    in the collected heap by default.  Further, when the collector
    discovers an instance is inaccessible (see above) or when the
    instance is explicitly deleted, its destructors will be invoked.
    NOTE: Only one instance of "gc_cleanup" should occur in the
    inheritance heirarchy -- i.e. it should always be a virtual
    base. */

inline void* operator new( 
    size_t size, 
    GCPlacement gcp,
    void (*cleanup)( void*, void* ) = 0,
    void* clientData = 0 );
    /*
    If "gcp = GC", then this "operator new" allocates in the collected
    heap, otherwise in the non-collected heap.  When the allocated
    object "obj" becomes inaccessible, the collector will invoke the
    function "cleanup( obj, clientData )".  It is an error to specify
    a non-null "cleanup" when "gcp = NoGC". */

/****************************************************************************

Inline implementation

****************************************************************************/

inline void* gc::operator new( size_t size ) {
    return GC_MALLOC( size ); }
    
inline void* gc::operator new( size_t size, GCPlacement gcp ) {
    if (gcp == GC) 
        return GC_MALLOC( size );
    else
        return GC_MALLOC_UNCOLLECTABLE( size ); }

inline void gc::operator delete( void* obj ) {
    GC_FREE( obj ); }
    
inline gc_cleanup::~gc_cleanup() {
    GC_REGISTER_FINALIZER( this, 0, 0, 0, 0 ); }

inline void gc_cleanup::cleanup( void* obj, void* displ ) {
    ((gc_cleanup*) ((char *)obj + (ptrdiff_t)displ))->~gc_cleanup(); }

inline gc_cleanup::gc_cleanup() {
    register void *base = GC_base((void *)this);
    GC_REGISTER_FINALIZER( base, cleanup,
			   (void *)((char *)this - (char *)base), 0, 0 ); }

inline void* operator new( 
    size_t size, 
    GCPlacement gcp,
    void (*cleanup)( void*, void* ),
    void* clientData )
{
    void* obj;

    if (gcp == GC) {
        obj = GC_MALLOC( size );
        if (cleanup != 0) 
            GC_REGISTER_FINALIZER( obj, cleanup, clientData, 0, 0 ); }
    else {
        obj = GC_MALLOC_UNCOLLECTABLE( size ); };
    return obj; }
        

#endif

