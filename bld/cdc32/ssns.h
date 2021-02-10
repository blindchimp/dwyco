// File:    ssns.h
// Author:  Brian Allen Vanderburg II
// Purpose: A simple signals and slots implemnetation
// ---------------------------------------------------------------------

// Configuration support
#define SSNS_NO_THREADS //- No support for multiple threads, ansi/iso c++
// SSNS_WIN32_THREADS - Use Windows threads
// SSNS_POSIX_THREADS - Use Posix threads
// SSNS_DLL - Build or use as a shared library/DLL
// SSNS_BUILDING - This is automatically set from the source file

// Include guard
#ifndef __BAVII_SSNS_H__
#define __BAVII_SSNS_H__

// Detect how to import/export symbols
#if defined(WIN32)
    #define SSNS_IMPORT __declspec(dllimport)
    #define SSNS_EXPORT __declspec(dllexport)
#else
    #define SSNS_IMPORT
    #define SSNS_EXPORT
#endif

#ifdef SSNS_DLL
    #ifdef SSNS_BUILDING
        #define SSNS_IMPEXP SSNS_EXPORT
    #else
        #define SSNS_IMPEXP SSNS_IMPORT
    #endif
#else
    #define SSNS_IMPEXP
#endif

// Includes
#include <list>
#include <set>

// Put everything in the namespace
namespace ssns
{

// Forward defs
class trackable;
class signal_base;
class connection;

// Mutex locker, will lock the signals/slots global mutex in
// multithreaded mode, in singlethreaded mode do nothing
// This is the only thing that has to be imported/exported
// when building/using SSNS as a shared library/DLL
class SSNS_IMPEXP mutex_locker
{
public:
    mutex_locker();
    ~mutex_locker();
private:
    // No copy
    mutex_locker(const mutex_locker& copy) { }
    mutex_locker& operator=(const mutex_locker& copy) { }
};

// Slot bases
// A slot is a function or member functin that can be called
class slot_base
{
public:
    slot_base() { }
    virtual ~slot_base() { }

    virtual trackable* get_dest() = 0;
};

// Slot bases for a specific signature

class slot_base0 : public slot_base
{
public:
    virtual void emit() = 0;
    virtual slot_base0* clone() = 0;
    virtual slot_base0* duplicate(trackable* newdest) = 0;
};
template <typename T1>
class slot_base1 : public slot_base
{
public:
    virtual void emit(T1) = 0;
    virtual slot_base1<T1>* clone() = 0;
    virtual slot_base1<T1>* duplicate(trackable* newdest) = 0;
};
template <typename T1, typename T2>
class slot_base2 : public slot_base
{
public:
    virtual void emit(T1, T2) = 0;
    virtual slot_base2<T1, T2>* clone() = 0;
    virtual slot_base2<T1, T2>* duplicate(trackable* newdest) = 0;
};
template <typename T1, typename T2, typename T3>
class slot_base3 : public slot_base
{
public:
    virtual void emit(T1, T2, T3) = 0;
    virtual slot_base3<T1, T2, T3>* clone() = 0;
    virtual slot_base3<T1, T2, T3>* duplicate(trackable* newdest) = 0;
};
template <typename T1, typename T2, typename T3, typename T4>
class slot_base4 : public slot_base
{
public:
    virtual void emit(T1, T2, T3, T4) = 0;
    virtual slot_base4<T1, T2, T3, T4>* clone() = 0;
    virtual slot_base4<T1, T2, T3, T4>* duplicate(trackable* newdest) = 0;
};
template <typename T1, typename T2, typename T3, typename T4, typename T5>
class slot_base5 : public slot_base
{
public:
    virtual void emit(T1, T2, T3, T4, T5) = 0;
    virtual slot_base5<T1, T2, T3, T4, T5>* clone() = 0;
    virtual slot_base5<T1, T2, T3, T4, T5>* duplicate(trackable* newdest) = 0;
};
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
class slot_base6 : public slot_base
{
public:
    virtual void emit(T1, T2, T3, T4, T5, T6) = 0;
    virtual slot_base6<T1, T2, T3, T4, T5, T6>* clone() = 0;
    virtual slot_base6<T1, T2, T3, T4, T5, T6>* duplicate(trackable* newdest) = 0;
};
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
class slot_base7 : public slot_base
{
public:
    virtual void emit(T1, T2, T3, T4, T5, T6, T7) = 0;
    virtual slot_base7<T1, T2, T3, T4, T5, T6, T7>* clone() = 0;
    virtual slot_base7<T1, T2, T3, T4, T5, T6, T7>* duplicate(trackable* newdest) = 0;
};


// Global function slots

class slot_ptrfun0 : public slot_base0
{
//friend class signal0;
public:
    slot_ptrfun0() : m_fn(0)
    {
    }

    slot_ptrfun0(void (*fn)()) : m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return 0;
    }

    void emit()
    {
        (*m_fn)();
    }

    slot_base0* clone()
    {
        return new slot_ptrfun0(*this);
    }

    slot_base0* duplicate(trackable* newdest)
    {
        return 0;
    }

public:
    void (*m_fn)();
};
template <typename T1>
class slot_ptrfun1 : public slot_base1<T1>
{
//friend class signal1<T1>;
public:
    slot_ptrfun1() : m_fn(0)
    {
    }

    slot_ptrfun1(void (*fn)(T1)) : m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return 0;
    }

    void emit(T1 a1)
    {
        (*m_fn)(a1);
    }

    slot_base1<T1>* clone()
    {
        return new slot_ptrfun1<T1>(*this);
    }

    slot_base1<T1>* duplicate(trackable* newdest)
    {
        return 0;
    }

public:
    void (*m_fn)(T1);
};
template <typename T1, typename T2>
class slot_ptrfun2 : public slot_base2<T1, T2>
{
//friend class signal2<T1, T2>;
public:
    slot_ptrfun2() : m_fn(0)
    {
    }

    slot_ptrfun2(void (*fn)(T1, T2)) : m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return 0;
    }

    void emit(T1 a1, T2 a2)
    {
        (*m_fn)(a1, a2);
    }

    slot_base2<T1, T2>* clone()
    {
        return new slot_ptrfun2<T1, T2>(*this);
    }

    slot_base2<T1, T2>* duplicate(trackable* newdest)
    {
        return 0;
    }

public:
    void (*m_fn)(T1, T2);
};
template <typename T1, typename T2, typename T3>
class slot_ptrfun3 : public slot_base3<T1, T2, T3>
{
//friend class signal3<T1, T2, T3>;
public:
    slot_ptrfun3() : m_fn(0)
    {
    }

    slot_ptrfun3(void (*fn)(T1, T2, T3)) : m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return 0;
    }

    void emit(T1 a1, T2 a2, T3 a3)
    {
        (*m_fn)(a1, a2, a3);
    }

    slot_base3<T1, T2, T3>* clone()
    {
        return new slot_ptrfun3<T1, T2, T3>(*this);
    }

    slot_base3<T1, T2, T3>* duplicate(trackable* newdest)
    {
        return 0;
    }

public:
    void (*m_fn)(T1, T2, T3);
};
template <typename T1, typename T2, typename T3, typename T4>
class slot_ptrfun4 : public slot_base4<T1, T2, T3, T4>
{
//friend class signal4<T1, T2, T3, T4>;
public:
    slot_ptrfun4() : m_fn(0)
    {
    }

    slot_ptrfun4(void (*fn)(T1, T2, T3, T4)) : m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return 0;
    }

    void emit(T1 a1, T2 a2, T3 a3, T4 a4)
    {
        (*m_fn)(a1, a2, a3, a4);
    }

    slot_base4<T1, T2, T3, T4>* clone()
    {
        return new slot_ptrfun4<T1, T2, T3, T4>(*this);
    }

    slot_base4<T1, T2, T3, T4>* duplicate(trackable* newdest)
    {
        return 0;
    }

public:
    void (*m_fn)(T1, T2, T3, T4);
};
template <typename T1, typename T2, typename T3, typename T4, typename T5>
class slot_ptrfun5 : public slot_base5<T1, T2, T3, T4, T5>
{
//friend class signal5<T1, T2, T3, T4, T5>;
public:
    slot_ptrfun5() : m_fn(0)
    {
    }

    slot_ptrfun5(void (*fn)(T1, T2, T3, T4, T5)) : m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return 0;
    }

    void emit(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5)
    {
        (*m_fn)(a1, a2, a3, a4, a5);
    }

    slot_base5<T1, T2, T3, T4, T5>* clone()
    {
        return new slot_ptrfun5<T1, T2, T3, T4, T5>(*this);
    }

    slot_base5<T1, T2, T3, T4, T5>* duplicate(trackable* newdest)
    {
        return 0;
    }

public:
    void (*m_fn)(T1, T2, T3, T4, T5);
};
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
class slot_ptrfun6 : public slot_base6<T1, T2, T3, T4, T5, T6>
{
//friend class signal6<T1, T2, T3, T4, T5, T6>;
public:
    slot_ptrfun6() : m_fn(0)
    {
    }

    slot_ptrfun6(void (*fn)(T1, T2, T3, T4, T5, T6)) : m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return 0;
    }

    void emit(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6)
    {
        (*m_fn)(a1, a2, a3, a4, a5, a6);
    }

    slot_base6<T1, T2, T3, T4, T5, T6>* clone()
    {
        return new slot_ptrfun6<T1, T2, T3, T4, T5, T6>(*this);
    }

    slot_base6<T1, T2, T3, T4, T5, T6>* duplicate(trackable* newdest)
    {
        return 0;
    }

public:
    void (*m_fn)(T1, T2, T3, T4, T5, T6);
};
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
class slot_ptrfun7 : public slot_base7<T1, T2, T3, T4, T5, T6, T7>
{
//friend class signal7<T1, T2, T3, T4, T5, T6, T7>;
public:
    slot_ptrfun7() : m_fn(0)
    {
    }

    slot_ptrfun7(void (*fn)(T1, T2, T3, T4, T5, T6, T7)) : m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return 0;
    }

    void emit(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7)
    {
        (*m_fn)(a1, a2, a3, a4, a5, a6, a7);
    }

    slot_base7<T1, T2, T3, T4, T5, T6, T7>* clone()
    {
        return new slot_ptrfun7<T1, T2, T3, T4, T5, T6, T7>(*this);
    }

    slot_base7<T1, T2, T3, T4, T5, T6, T7>* duplicate(trackable* newdest)
    {
        return 0;
    }

public:
    void (*m_fn)(T1, T2, T3, T4, T5, T6, T7);
};


// Member function slots
template <typename T_obj>
class slot_memfun0 : public slot_base0
{
public:
    slot_memfun0() : m_obj(0), m_fn(0)
    {
    }
    
    slot_memfun0(T_obj* obj, void (T_obj::*fn)()) : m_obj(obj), m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return m_obj;
    }

    void emit()
    {
        (m_obj->*m_fn)();
    }

    slot_base0* clone()
    {
        return new slot_memfun0<T_obj>(*this);
    }

    slot_base0* duplicate(trackable* newdest)
    {
        return new slot_memfun0<T_obj>((T_obj*)newdest, m_fn);
    }

private:
    T_obj* m_obj;
    void (T_obj::*m_fn)();
};
template <typename T_obj, typename T1>
class slot_memfun1 : public slot_base1<T1>
{
public:
    slot_memfun1() : m_obj(0), m_fn(0)
    {
    }
    
    slot_memfun1(T_obj* obj, void (T_obj::*fn)(T1)) : m_obj(obj), m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return m_obj;
    }

    void emit(T1 a1)
    {
        (m_obj->*m_fn)(a1);
    }

    slot_base1<T1>* clone()
    {
        return new slot_memfun1<T_obj, T1>(*this);
    }

    slot_base1<T1>* duplicate(trackable* newdest)
    {
        return new slot_memfun1<T_obj, T1>((T_obj*)newdest, m_fn);
    }

private:
    T_obj* m_obj;
    void (T_obj::*m_fn)(T1);
};
template <typename T_obj, typename T1, typename T2>
class slot_memfun2 : public slot_base2<T1, T2>
{
public:
    slot_memfun2() : m_obj(0), m_fn(0)
    {
    }
    
    slot_memfun2(T_obj* obj, void (T_obj::*fn)(T1, T2)) : m_obj(obj), m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return m_obj;
    }

    void emit(T1 a1, T2 a2)
    {
        (m_obj->*m_fn)(a1, a2);
    }

    slot_base2<T1, T2>* clone()
    {
        return new slot_memfun2<T_obj, T1, T2>(*this);
    }

    slot_base2<T1, T2>* duplicate(trackable* newdest)
    {
        return new slot_memfun2<T_obj, T1, T2>((T_obj*)newdest, m_fn);
    }

private:
    T_obj* m_obj;
    void (T_obj::*m_fn)(T1, T2);
};
template <typename T_obj, typename T1, typename T2, typename T3>
class slot_memfun3 : public slot_base3<T1, T2, T3>
{
public:
    slot_memfun3() : m_obj(0), m_fn(0)
    {
    }
    
    slot_memfun3(T_obj* obj, void (T_obj::*fn)(T1, T2, T3)) : m_obj(obj), m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return m_obj;
    }

    void emit(T1 a1, T2 a2, T3 a3)
    {
        (m_obj->*m_fn)(a1, a2, a3);
    }

    slot_base3<T1, T2, T3>* clone()
    {
        return new slot_memfun3<T_obj, T1, T2, T3>(*this);
    }

    slot_base3<T1, T2, T3>* duplicate(trackable* newdest)
    {
        return new slot_memfun3<T_obj, T1, T2, T3>((T_obj*)newdest, m_fn);
    }

private:
    T_obj* m_obj;
    void (T_obj::*m_fn)(T1, T2, T3);
};
template <typename T_obj, typename T1, typename T2, typename T3, typename T4>
class slot_memfun4 : public slot_base4<T1, T2, T3, T4>
{
public:
    slot_memfun4() : m_obj(0), m_fn(0)
    {
    }
    
    slot_memfun4(T_obj* obj, void (T_obj::*fn)(T1, T2, T3, T4)) : m_obj(obj), m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return m_obj;
    }

    void emit(T1 a1, T2 a2, T3 a3, T4 a4)
    {
        (m_obj->*m_fn)(a1, a2, a3, a4);
    }

    slot_base4<T1, T2, T3, T4>* clone()
    {
        return new slot_memfun4<T_obj, T1, T2, T3, T4>(*this);
    }

    slot_base4<T1, T2, T3, T4>* duplicate(trackable* newdest)
    {
        return new slot_memfun4<T_obj, T1, T2, T3, T4>((T_obj*)newdest, m_fn);
    }

private:
    T_obj* m_obj;
    void (T_obj::*m_fn)(T1, T2, T3, T4);
};
template <typename T_obj, typename T1, typename T2, typename T3, typename T4, typename T5>
class slot_memfun5 : public slot_base5<T1, T2, T3, T4, T5>
{
public:
    slot_memfun5() : m_obj(0), m_fn(0)
    {
    }
    
    slot_memfun5(T_obj* obj, void (T_obj::*fn)(T1, T2, T3, T4, T5)) : m_obj(obj), m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return m_obj;
    }

    void emit(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5)
    {
        (m_obj->*m_fn)(a1, a2, a3, a4, a5);
    }

    slot_base5<T1, T2, T3, T4, T5>* clone()
    {
        return new slot_memfun5<T_obj, T1, T2, T3, T4, T5>(*this);
    }

    slot_base5<T1, T2, T3, T4, T5>* duplicate(trackable* newdest)
    {
        return new slot_memfun5<T_obj, T1, T2, T3, T4, T5>((T_obj*)newdest, m_fn);
    }

private:
    T_obj* m_obj;
    void (T_obj::*m_fn)(T1, T2, T3, T4, T5);
};
template <typename T_obj, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
class slot_memfun6 : public slot_base6<T1, T2, T3, T4, T5, T6>
{
public:
    slot_memfun6() : m_obj(0), m_fn(0)
    {
    }
    
    slot_memfun6(T_obj* obj, void (T_obj::*fn)(T1, T2, T3, T4, T5, T6)) : m_obj(obj), m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return m_obj;
    }

    void emit(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6)
    {
        (m_obj->*m_fn)(a1, a2, a3, a4, a5, a6);
    }

    slot_base6<T1, T2, T3, T4, T5, T6>* clone()
    {
        return new slot_memfun6<T_obj, T1, T2, T3, T4, T5, T6>(*this);
    }

    slot_base6<T1, T2, T3, T4, T5, T6>* duplicate(trackable* newdest)
    {
        return new slot_memfun6<T_obj, T1, T2, T3, T4, T5, T6>((T_obj*)newdest, m_fn);
    }

private:
    T_obj* m_obj;
    void (T_obj::*m_fn)(T1, T2, T3, T4, T5, T6);
};
template <typename T_obj, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
class slot_memfun7 : public slot_base7<T1, T2, T3, T4, T5, T6, T7>
{
public:
    slot_memfun7() : m_obj(0), m_fn(0)
    {
    }
    
    slot_memfun7(T_obj* obj, void (T_obj::*fn)(T1, T2, T3, T4, T5, T6, T7)) : m_obj(obj), m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return m_obj;
    }

    void emit(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7)
    {
        (m_obj->*m_fn)(a1, a2, a3, a4, a5, a6, a7);
    }

    slot_base7<T1, T2, T3, T4, T5, T6, T7>* clone()
    {
        return new slot_memfun7<T_obj, T1, T2, T3, T4, T5, T6, T7>(*this);
    }

    slot_base7<T1, T2, T3, T4, T5, T6, T7>* duplicate(trackable* newdest)
    {
        return new slot_memfun7<T_obj, T1, T2, T3, T4, T5, T6, T7>((T_obj*)newdest, m_fn);
    }

private:
    T_obj* m_obj;
    void (T_obj::*m_fn)(T1, T2, T3, T4, T5, T6, T7);
};


// Base signal
class signal_base
{
public:
    signal_base() { }
    virtual ~signal_base() { }

    virtual void trackable_disconnect(trackable* obj) = 0;
    virtual void trackable_duplicate(const trackable* oldobj, trackable* newobj) = 0;

    virtual void connection_connect(connection* conn) = 0;
    virtual void connection_disconnect(connection* conn) = 0;

    virtual void disconnect(trackable* obj) = 0; // Pass null to disconnect ptrfun slots
    virtual void disconnect(slot_base* slot) = 0; // Disconnect a single slot
    virtual void disconnect_all() = 0;
};

// A trackable item, when destroyed will automatically remove itself from the signal
// Also, when a signal is destroyed, it will remove it from the trackable items
// Unlike some other classes, trackable is not virtual and does not have a
// virtual dtor
class trackable
{
    typedef std::set<signal_base*> signal_set;
    typedef signal_set::const_iterator const_iterator;

public:
    trackable()
    {
    }

    trackable(const trackable& copy)
    {
        mutex_locker lock;

        const_iterator it = copy.m_signals.begin();
        const_iterator itEnd = copy.m_signals.end();

        while(it != itEnd)
        {
            // Tell the signal to duplicate any slots that are
            // connected to the copy, but make them connect to us
   		    (*it)->trackable_duplicate(&copy, this);
			m_signals.insert(*it);
			++it;
		}
    }

    ~trackable()
    {
        disconnect_all();
    }

    void signal_connect(signal_base* sig)
    {
        mutex_locker lock;

        m_signals.insert(sig);
    }

    void signal_disconnect(signal_base* sig)
    {
        mutex_locker lock;

        m_signals.erase(sig);
    }

    void disconnect_all()
    {
        mutex_locker lock;

        const_iterator it = m_signals.begin();
        const_iterator itEnd = m_signals.end();

        while(it != itEnd)
        {
            (*it)->trackable_disconnect(this);
            ++it;
        }

        m_signals.erase(m_signals.begin(), m_signals.end());
    }

private:
    // The copy ctor copies connections, but assignment does not
    trackable& operator=(const trackable& copy) { }

    // The signals connected to this trackable
    signal_set m_signals;
};

// A connection object returned by signal::connect_ functions
// When the signal is destroyed the connection pointer will be
// reset.  Also if the connection object is destroyed first it
// will be removed from the signal connection list
class connection
{
public:
    connection() : m_signal(0), m_slot(0)
    {
    }

    connection(const connection& copy)
    {
        mutex_locker lock;

        m_signal = copy.m_signal;
        m_slot = copy.m_slot;

        if(m_signal)
            m_signal->connection_connect(this);
    }

    // This version should only be called by the signal object
    connection(signal_base* signal, slot_base* slot)
    {
        mutex_locker lock;

        m_signal = signal;
        m_slot = slot;

        if(m_signal)
            m_signal->connection_connect(this);
    }

    ~connection()
    {
        mutex_locker lock;

        if(m_signal)
            m_signal->connection_disconnect(this);
    }

    connection& operator=(const connection& copy)
    {
        mutex_locker lock;

        if(this != &copy)
        {
            if(m_signal)
                m_signal->connection_disconnect(this);

            m_signal = copy.m_signal;
            m_slot = copy.m_slot;

            if(m_signal)
                m_signal->connection_connect(this);
        }

        return *this;
    }

    void signal_disconnect()
    {
        mutex_locker lock;

        m_signal = 0;
    }

    void disconnect()
    {
        mutex_locker lock;

        if(m_signal)
            m_signal->disconnect(m_slot);
    }
private:
    signal_base* m_signal;
    slot_base* m_slot;
};

// The actual signals

class signal0 : public signal_base
{
protected:
    typedef slot_base0 slot_type;
    typedef std::list<slot_type*> slot_list;
    typedef std::list<connection*> connection_list;

public:
    signal0() { }

    signal0(const signal0& copy)
    {
        mutex_locker lock;

         slot_list::const_iterator it = copy.m_slots.begin();
         slot_list::const_iterator itEnd = copy.m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest())
            {
                (*it)->get_dest()->signal_connect(this);
            }
            m_slots.push_back((*it)->clone());

            ++it;
        }
    }

    ~signal0()
    {
        disconnect_all();
    }

    void trackable_disconnect(trackable* obj)
    {
        mutex_locker lock;

         slot_list::iterator it = m_slots.begin();
         slot_list::iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
             slot_list::iterator itNext = it;
            ++itNext;

            if((*it)->get_dest() == obj)
            {
                delete *it;
                m_slots.erase(it);
            }

            it = itNext;
        }
    }

    void trackable_duplicate(const trackable* oldobj, trackable* newobj)
    {
        mutex_locker lock;

         slot_list::iterator it = m_slots.begin();
         slot_list::iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest() == oldobj)
            {
                m_slots.push_back((*it)->duplicate(newobj));
            }

            ++it;
        }
    }

    void connection_connect(connection* conn)
    {
        mutex_locker lock;

        m_connections.push_back(conn);
    }

    void connection_disconnect(connection* conn)
    {
        mutex_locker lock;

        connection_list::iterator it = m_connections.begin();
        connection_list::iterator itEnd = m_connections.end();

        while(it != itEnd)
        {
            if(*it == conn)
            {
                m_connections.erase(it);
                break;
            }
            ++it;
        }
    }

    // Connect to a normal global function
    connection connect_ptrfun(void (*fn)(), int unique = 0)
    {
        mutex_locker lock;
        
        slot_ptrfun0* slot = new slot_ptrfun0(fn);
        if(unique)
        {
            typename slot_list::iterator it = m_slots.begin();
            typename slot_list::iterator itEnd = m_slots.end();
            while(it != itEnd)
            {
                if(dynamic_cast<decltype(slot)>(*it)->m_fn == fn)
                {
                    delete slot;
                    return connection();
                }
                ++it;
            }
        }
        m_slots.push_back(slot);

        return connection(this, slot);
    }
    
    template <typename T_obj>
    connection connect_memfun(T_obj* obj, void (T_obj::*fn)())
    {
        mutex_locker lock;
        
        slot_memfun0<T_obj>* slot = new slot_memfun0<T_obj>(obj, fn);
        m_slots.push_back(slot);
        
        obj->signal_connect(this);

        return connection(this, slot);
    }

    void disconnect(trackable* obj)
    {
        mutex_locker lock;

         slot_list::iterator it = m_slots.begin();
         slot_list::iterator itEnd = m_slots.end();

        // Disconnect ALL slots that connect to the trackable,
        // not just the first one
        while(it != itEnd)
        {
             slot_list::iterator itNext = it;
            ++itNext;

            if((*it)->get_dest() == obj)
            {
                if(obj)
                    obj->signal_disconnect(this);

                delete *it;
                m_slots.erase(it);
            }

            it = itNext;
        }
    }

    void disconnect(slot_base* slot)
    {
        mutex_locker lock;

         slot_list::iterator it = m_slots.begin();
         slot_list::iterator itEnd = m_slots.end();
        trackable* dest = 0;

        while(it != itEnd)
        {
            if(*it == slot)
            {
                dest = (*it)->get_dest();
                delete *it;
                m_slots.erase(it);
                break;
            }

            ++it;
        }

        // Don't call signal_disconnect unless no other slots
        // have the same destination
        if(dest)
        {
            it = m_slots.begin();
            itEnd = m_slots.end();

            while(it != itEnd)
            {
                if((*it)->get_dest() == dest)
                {
                    break;
                }

                ++it;
            }

            if(it == itEnd)
            {
                // No other slot uses the same trackable
                dest->signal_disconnect(this);
            }
        }
    }

    void disconnect_all()
    {
        mutex_locker lock;

         slot_list::const_iterator it = m_slots.begin();
         slot_list::const_iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest())
            {
                (*it)->get_dest()->signal_disconnect(this);
            }

            delete *it;

            ++it;
        }

        m_slots.erase(m_slots.begin(), m_slots.end());

        // Also reset any connection pointers.  This is okay
        // even though we may not be gettin destroyed yet
        connection_list::const_iterator connit = m_connections.begin();
        connection_list::const_iterator connitEnd = m_connections.end();

        while(connit != connitEnd)
        {
            (*connit)->signal_disconnect();
            ++connit;
        }

        m_connections.erase(m_connections.begin(), m_connections.end());
    }

    void emit()
    {
        mutex_locker lock;
        
         slot_list::const_iterator it = this->m_slots.begin();
         slot_list::const_iterator itEnd = this->m_slots.end();
        
        while(it != itEnd)
        {
             slot_list::const_iterator itNext = it;
            ++itNext;
            
            (*it)->emit();
                
            it = itNext;
        }
    }

    void operator()()
    {
        return emit();
    }

protected:
    slot_list m_slots;
    connection_list m_connections;
};
template <typename T1>
class signal1 : public signal_base
{
protected:
    typedef slot_base1<T1> slot_type;
    typedef std::list<slot_type*> slot_list;
    typedef std::list<connection*> connection_list;

public:
    signal1() { }

    signal1(const signal1<T1>& copy)
    {
        mutex_locker lock;

        typename slot_list::const_iterator it = copy.m_slots.begin();
        typename slot_list::const_iterator itEnd = copy.m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest())
            {
                (*it)->get_dest()->signal_connect(this);
            }
            m_slots.push_back((*it)->clone());

            ++it;
        }
    }

    ~signal1()
    {
        disconnect_all();
    }

    void trackable_disconnect(trackable* obj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            typename slot_list::iterator itNext = it;
            ++itNext;

            if((*it)->get_dest() == obj)
            {
                delete *it;
                m_slots.erase(it);
            }

            it = itNext;
        }
    }

    void trackable_duplicate(const trackable* oldobj, trackable* newobj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest() == oldobj)
            {
                m_slots.push_back((*it)->duplicate(newobj));
            }

            ++it;
        }
    }

    void connection_connect(connection* conn)
    {
        mutex_locker lock;

        m_connections.push_back(conn);
    }

    void connection_disconnect(connection* conn)
    {
        mutex_locker lock;

        connection_list::iterator it = m_connections.begin();
        connection_list::iterator itEnd = m_connections.end();

        while(it != itEnd)
        {
            if(*it == conn)
            {
                m_connections.erase(it);
                break;
            }
            ++it;
        }
    }

    // Connect to a normal global function
    connection connect_ptrfun(void (*fn)(T1), int unique = 0)
    {
        mutex_locker lock;
        
        slot_ptrfun1<T1>* slot = new slot_ptrfun1<T1>(fn);
        if(unique)
        {
            typename slot_list::iterator it = m_slots.begin();
            typename slot_list::iterator itEnd = m_slots.end();
            while(it != itEnd)
            {
                if(dynamic_cast<decltype(slot)>(*it)->m_fn == fn)
                {
                    delete slot;
                    return connection();
                }
                ++it;
            }
        }
        m_slots.push_back(slot);

        return connection(this, slot);
    }
    
    template <typename T_obj>
    connection connect_memfun(T_obj* obj, void (T_obj::*fn)(T1))
    {
        mutex_locker lock;
        
        slot_memfun1<T_obj, T1>* slot = new slot_memfun1<T_obj, T1>(obj, fn);
        m_slots.push_back(slot);
        
        obj->signal_connect(this);

        return connection(this, slot);
    }

    void disconnect(trackable* obj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        // Disconnect ALL slots that connect to the trackable,
        // not just the first one
        while(it != itEnd)
        {
            typename slot_list::iterator itNext = it;
            ++itNext;

            if((*it)->get_dest() == obj)
            {
                if(obj)
                    obj->signal_disconnect(this);

                delete *it;
                m_slots.erase(it);
            }

            it = itNext;
        }
    }

    void disconnect(slot_base* slot)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();
        trackable* dest = 0;

        while(it != itEnd)
        {
            if(*it == slot)
            {
                dest = (*it)->get_dest();
                delete *it;
                m_slots.erase(it);
                break;
            }

            ++it;
        }

        // Don't call signal_disconnect unless no other slots
        // have the same destination
        if(dest)
        {
            it = m_slots.begin();
            itEnd = m_slots.end();

            while(it != itEnd)
            {
                if((*it)->get_dest() == dest)
                {
                    break;
                }

                ++it;
            }

            if(it == itEnd)
            {
                // No other slot uses the same trackable
                dest->signal_disconnect(this);
            }
        }
    }

    void disconnect_all()
    {
        mutex_locker lock;

        typename slot_list::const_iterator it = m_slots.begin();
        typename slot_list::const_iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest())
            {
                (*it)->get_dest()->signal_disconnect(this);
            }

            delete *it;

            ++it;
        }

        m_slots.erase(m_slots.begin(), m_slots.end());

        // Also reset any connection pointers.  This is okay
        // even though we may not be gettin destroyed yet
        connection_list::const_iterator connit = m_connections.begin();
        connection_list::const_iterator connitEnd = m_connections.end();

        while(connit != connitEnd)
        {
            (*connit)->signal_disconnect();
            ++connit;
        }

        m_connections.erase(m_connections.begin(), m_connections.end());
    }

    void emit(T1 a1)
    {
        mutex_locker lock;
        
        typename slot_list::const_iterator it = this->m_slots.begin();
        typename slot_list::const_iterator itEnd = this->m_slots.end();
        
        while(it != itEnd)
        {
            typename slot_list::const_iterator itNext = it;
            ++itNext;
            
            (*it)->emit(a1);
                
            it = itNext;
        }
    }

    void operator()(T1 a1)
    {
        return emit(a1);
    }

protected:
    slot_list m_slots;
    connection_list m_connections;
};
template <typename T1, typename T2>
class signal2 : public signal_base
{
protected:
    typedef slot_base2<T1, T2> slot_type;
    typedef std::list<slot_type*> slot_list;
    typedef std::list<connection*> connection_list;

public:
    signal2() { }

    signal2(const signal2<T1, T2>& copy)
    {
        mutex_locker lock;

        typename slot_list::const_iterator it = copy.m_slots.begin();
        typename slot_list::const_iterator itEnd = copy.m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest())
            {
                (*it)->get_dest()->signal_connect(this);
            }
            m_slots.push_back((*it)->clone());

            ++it;
        }
    }

    ~signal2()
    {
        disconnect_all();
    }

    void trackable_disconnect(trackable* obj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            typename slot_list::iterator itNext = it;
            ++itNext;

            if((*it)->get_dest() == obj)
            {
                delete *it;
                m_slots.erase(it);
            }

            it = itNext;
        }
    }

    void trackable_duplicate(const trackable* oldobj, trackable* newobj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest() == oldobj)
            {
                m_slots.push_back((*it)->duplicate(newobj));
            }

            ++it;
        }
    }

    void connection_connect(connection* conn)
    {
        mutex_locker lock;

        m_connections.push_back(conn);
    }

    void connection_disconnect(connection* conn)
    {
        mutex_locker lock;

        connection_list::iterator it = m_connections.begin();
        connection_list::iterator itEnd = m_connections.end();

        while(it != itEnd)
        {
            if(*it == conn)
            {
                m_connections.erase(it);
                break;
            }
            ++it;
        }
    }

    // Connect to a normal global function
    connection connect_ptrfun(void (*fn)(T1, T2), int unique = 0)
    {
        mutex_locker lock;
        
        slot_ptrfun2<T1, T2>* slot = new slot_ptrfun2<T1, T2>(fn);
        if(unique)
        {
            typename slot_list::iterator it = m_slots.begin();
            typename slot_list::iterator itEnd = m_slots.end();
            while(it != itEnd)
            {
                if(dynamic_cast<decltype(slot)>(*it)->m_fn == fn)
                {
                    delete slot;
                    return connection();
                }
                ++it;
            }
        }
        m_slots.push_back(slot);

        return connection(this, slot);
    }
    
    template <typename T_obj>
    connection connect_memfun(T_obj* obj, void (T_obj::*fn)(T1, T2))
    {
        mutex_locker lock;
        
        slot_memfun2<T_obj, T1, T2>* slot = new slot_memfun2<T_obj, T1, T2>(obj, fn);
        m_slots.push_back(slot);
        
        obj->signal_connect(this);

        return connection(this, slot);
    }

    void disconnect(trackable* obj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        // Disconnect ALL slots that connect to the trackable,
        // not just the first one
        while(it != itEnd)
        {
            typename slot_list::iterator itNext = it;
            ++itNext;

            if((*it)->get_dest() == obj)
            {
                if(obj)
                    obj->signal_disconnect(this);

                delete *it;
                m_slots.erase(it);
            }

            it = itNext;
        }
    }

    void disconnect(slot_base* slot)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();
        trackable* dest = 0;

        while(it != itEnd)
        {
            if(*it == slot)
            {
                dest = (*it)->get_dest();
                delete *it;
                m_slots.erase(it);
                break;
            }

            ++it;
        }

        // Don't call signal_disconnect unless no other slots
        // have the same destination
        if(dest)
        {
            it = m_slots.begin();
            itEnd = m_slots.end();

            while(it != itEnd)
            {
                if((*it)->get_dest() == dest)
                {
                    break;
                }

                ++it;
            }

            if(it == itEnd)
            {
                // No other slot uses the same trackable
                dest->signal_disconnect(this);
            }
        }
    }

    void disconnect_all()
    {
        mutex_locker lock;

        typename slot_list::const_iterator it = m_slots.begin();
        typename slot_list::const_iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest())
            {
                (*it)->get_dest()->signal_disconnect(this);
            }

            delete *it;

            ++it;
        }

        m_slots.erase(m_slots.begin(), m_slots.end());

        // Also reset any connection pointers.  This is okay
        // even though we may not be gettin destroyed yet
        connection_list::const_iterator connit = m_connections.begin();
        connection_list::const_iterator connitEnd = m_connections.end();

        while(connit != connitEnd)
        {
            (*connit)->signal_disconnect();
            ++connit;
        }

        m_connections.erase(m_connections.begin(), m_connections.end());
    }

    void emit(T1 a1, T2 a2)
    {
        mutex_locker lock;
        
        typename slot_list::const_iterator it = this->m_slots.begin();
        typename slot_list::const_iterator itEnd = this->m_slots.end();
        
        while(it != itEnd)
        {
            typename slot_list::const_iterator itNext = it;
            ++itNext;
            
            (*it)->emit(a1, a2);
                
            it = itNext;
        }
    }

    void operator()(T1 a1, T2 a2)
    {
        return emit(a1, a2);
    }

protected:
    slot_list m_slots;
    connection_list m_connections;
};
template <typename T1, typename T2, typename T3>
class signal3 : public signal_base
{
protected:
    typedef slot_base3<T1, T2, T3> slot_type;
    typedef std::list<slot_type*> slot_list;
    typedef std::list<connection*> connection_list;

public:
    signal3() { }

    signal3(const signal3<T1, T2, T3>& copy)
    {
        mutex_locker lock;

        typename slot_list::const_iterator it = copy.m_slots.begin();
        typename slot_list::const_iterator itEnd = copy.m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest())
            {
                (*it)->get_dest()->signal_connect(this);
            }
            m_slots.push_back((*it)->clone());

            ++it;
        }
    }

    ~signal3()
    {
        disconnect_all();
    }

    void trackable_disconnect(trackable* obj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            typename slot_list::iterator itNext = it;
            ++itNext;

            if((*it)->get_dest() == obj)
            {
                delete *it;
                m_slots.erase(it);
            }

            it = itNext;
        }
    }

    void trackable_duplicate(const trackable* oldobj, trackable* newobj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest() == oldobj)
            {
                m_slots.push_back((*it)->duplicate(newobj));
            }

            ++it;
        }
    }

    void connection_connect(connection* conn)
    {
        mutex_locker lock;

        m_connections.push_back(conn);
    }

    void connection_disconnect(connection* conn)
    {
        mutex_locker lock;

        connection_list::iterator it = m_connections.begin();
        connection_list::iterator itEnd = m_connections.end();

        while(it != itEnd)
        {
            if(*it == conn)
            {
                m_connections.erase(it);
                break;
            }
            ++it;
        }
    }

    // Connect to a normal global function
    connection connect_ptrfun(void (*fn)(T1, T2, T3), int unique = 0)
    {
        mutex_locker lock;
        
        slot_ptrfun3<T1, T2, T3>* slot = new slot_ptrfun3<T1, T2, T3>(fn);
        if(unique)
        {
            typename slot_list::iterator it = m_slots.begin();
            typename slot_list::iterator itEnd = m_slots.end();
            while(it != itEnd)
            {
                if(dynamic_cast<decltype(slot)>(*it)->m_fn == fn)
                {
                    delete slot;
                    return connection();
                }
                ++it;
            }
        }
        m_slots.push_back(slot);

        return connection(this, slot);
    }
    
    template <typename T_obj>
    connection connect_memfun(T_obj* obj, void (T_obj::*fn)(T1, T2, T3))
    {
        mutex_locker lock;
        
        slot_memfun3<T_obj, T1, T2, T3>* slot = new slot_memfun3<T_obj, T1, T2, T3>(obj, fn);
        m_slots.push_back(slot);
        
        obj->signal_connect(this);

        return connection(this, slot);
    }

    void disconnect(trackable* obj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        // Disconnect ALL slots that connect to the trackable,
        // not just the first one
        while(it != itEnd)
        {
            typename slot_list::iterator itNext = it;
            ++itNext;

            if((*it)->get_dest() == obj)
            {
                if(obj)
                    obj->signal_disconnect(this);

                delete *it;
                m_slots.erase(it);
            }

            it = itNext;
        }
    }

    void disconnect(slot_base* slot)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();
        trackable* dest = 0;

        while(it != itEnd)
        {
            if(*it == slot)
            {
                dest = (*it)->get_dest();
                delete *it;
                m_slots.erase(it);
                break;
            }

            ++it;
        }

        // Don't call signal_disconnect unless no other slots
        // have the same destination
        if(dest)
        {
            it = m_slots.begin();
            itEnd = m_slots.end();

            while(it != itEnd)
            {
                if((*it)->get_dest() == dest)
                {
                    break;
                }

                ++it;
            }

            if(it == itEnd)
            {
                // No other slot uses the same trackable
                dest->signal_disconnect(this);
            }
        }
    }

    void disconnect_all()
    {
        mutex_locker lock;

        typename slot_list::const_iterator it = m_slots.begin();
        typename slot_list::const_iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest())
            {
                (*it)->get_dest()->signal_disconnect(this);
            }

            delete *it;

            ++it;
        }

        m_slots.erase(m_slots.begin(), m_slots.end());

        // Also reset any connection pointers.  This is okay
        // even though we may not be gettin destroyed yet
        connection_list::const_iterator connit = m_connections.begin();
        connection_list::const_iterator connitEnd = m_connections.end();

        while(connit != connitEnd)
        {
            (*connit)->signal_disconnect();
            ++connit;
        }

        m_connections.erase(m_connections.begin(), m_connections.end());
    }

    void emit(T1 a1, T2 a2, T3 a3)
    {
        mutex_locker lock;
        
        typename slot_list::const_iterator it = this->m_slots.begin();
        typename slot_list::const_iterator itEnd = this->m_slots.end();
        
        while(it != itEnd)
        {
            typename slot_list::const_iterator itNext = it;
            ++itNext;
            
            (*it)->emit(a1, a2, a3);
                
            it = itNext;
        }
    }

    void operator()(T1 a1, T2 a2, T3 a3)
    {
        return emit(a1, a2, a3);
    }

protected:
    slot_list m_slots;
    connection_list m_connections;
};
template <typename T1, typename T2, typename T3, typename T4>
class signal4 : public signal_base
{
protected:
    typedef slot_base4<T1, T2, T3, T4> slot_type;
    typedef std::list<slot_type*> slot_list;
    typedef std::list<connection*> connection_list;

public:
    signal4() { }

    signal4(const signal4<T1, T2, T3, T4>& copy)
    {
        mutex_locker lock;

        typename slot_list::const_iterator it = copy.m_slots.begin();
        typename slot_list::const_iterator itEnd = copy.m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest())
            {
                (*it)->get_dest()->signal_connect(this);
            }
            m_slots.push_back((*it)->clone());

            ++it;
        }
    }

    ~signal4()
    {
        disconnect_all();
    }

    void trackable_disconnect(trackable* obj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            typename slot_list::iterator itNext = it;
            ++itNext;

            if((*it)->get_dest() == obj)
            {
                delete *it;
                m_slots.erase(it);
            }

            it = itNext;
        }
    }

    void trackable_duplicate(const trackable* oldobj, trackable* newobj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest() == oldobj)
            {
                m_slots.push_back((*it)->duplicate(newobj));
            }

            ++it;
        }
    }

    void connection_connect(connection* conn)
    {
        mutex_locker lock;

        m_connections.push_back(conn);
    }

    void connection_disconnect(connection* conn)
    {
        mutex_locker lock;

        connection_list::iterator it = m_connections.begin();
        connection_list::iterator itEnd = m_connections.end();

        while(it != itEnd)
        {
            if(*it == conn)
            {
                m_connections.erase(it);
                break;
            }
            ++it;
        }
    }

    // Connect to a normal global function
    connection connect_ptrfun(void (*fn)(T1, T2, T3, T4), int unique = 0)
    {
        mutex_locker lock;
        
        slot_ptrfun4<T1, T2, T3, T4>* slot = new slot_ptrfun4<T1, T2, T3, T4>(fn);
        if(unique)
        {
            typename slot_list::iterator it = m_slots.begin();
            typename slot_list::iterator itEnd = m_slots.end();
            while(it != itEnd)
            {
                if(dynamic_cast<decltype(slot)>(*it)->m_fn == fn)
                {
                    delete slot;
                    return connection();
                }
                ++it;
            }
        }
        m_slots.push_back(slot);

        return connection(this, slot);
    }
    
    template <typename T_obj>
    connection connect_memfun(T_obj* obj, void (T_obj::*fn)(T1, T2, T3, T4))
    {
        mutex_locker lock;
        
        slot_memfun4<T_obj, T1, T2, T3, T4>* slot = new slot_memfun4<T_obj, T1, T2, T3, T4>(obj, fn);
        m_slots.push_back(slot);
        
        obj->signal_connect(this);

        return connection(this, slot);
    }

    void disconnect(trackable* obj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        // Disconnect ALL slots that connect to the trackable,
        // not just the first one
        while(it != itEnd)
        {
            typename slot_list::iterator itNext = it;
            ++itNext;

            if((*it)->get_dest() == obj)
            {
                if(obj)
                    obj->signal_disconnect(this);

                delete *it;
                m_slots.erase(it);
            }

            it = itNext;
        }
    }

    void disconnect(slot_base* slot)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();
        trackable* dest = 0;

        while(it != itEnd)
        {
            if(*it == slot)
            {
                dest = (*it)->get_dest();
                delete *it;
                m_slots.erase(it);
                break;
            }

            ++it;
        }

        // Don't call signal_disconnect unless no other slots
        // have the same destination
        if(dest)
        {
            it = m_slots.begin();
            itEnd = m_slots.end();

            while(it != itEnd)
            {
                if((*it)->get_dest() == dest)
                {
                    break;
                }

                ++it;
            }

            if(it == itEnd)
            {
                // No other slot uses the same trackable
                dest->signal_disconnect(this);
            }
        }
    }

    void disconnect_all()
    {
        mutex_locker lock;

        typename slot_list::const_iterator it = m_slots.begin();
        typename slot_list::const_iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest())
            {
                (*it)->get_dest()->signal_disconnect(this);
            }

            delete *it;

            ++it;
        }

        m_slots.erase(m_slots.begin(), m_slots.end());

        // Also reset any connection pointers.  This is okay
        // even though we may not be gettin destroyed yet
        connection_list::const_iterator connit = m_connections.begin();
        connection_list::const_iterator connitEnd = m_connections.end();

        while(connit != connitEnd)
        {
            (*connit)->signal_disconnect();
            ++connit;
        }

        m_connections.erase(m_connections.begin(), m_connections.end());
    }

    void emit(T1 a1, T2 a2, T3 a3, T4 a4)
    {
        mutex_locker lock;
        
        typename slot_list::const_iterator it = this->m_slots.begin();
        typename slot_list::const_iterator itEnd = this->m_slots.end();
        
        while(it != itEnd)
        {
            typename slot_list::const_iterator itNext = it;
            ++itNext;
            
            (*it)->emit(a1, a2, a3, a4);
                
            it = itNext;
        }
    }

    void operator()(T1 a1, T2 a2, T3 a3, T4 a4)
    {
        return emit(a1, a2, a3, a4);
    }

protected:
    slot_list m_slots;
    connection_list m_connections;
};
template <typename T1, typename T2, typename T3, typename T4, typename T5>
class signal5 : public signal_base
{
protected:
    typedef slot_base5<T1, T2, T3, T4, T5> slot_type;
    typedef std::list<slot_type*> slot_list;
    typedef std::list<connection*> connection_list;

public:
    signal5() { }

    signal5(const signal5<T1, T2, T3, T4, T5>& copy)
    {
        mutex_locker lock;

        typename slot_list::const_iterator it = copy.m_slots.begin();
        typename slot_list::const_iterator itEnd = copy.m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest())
            {
                (*it)->get_dest()->signal_connect(this);
            }
            m_slots.push_back((*it)->clone());

            ++it;
        }
    }

    ~signal5()
    {
        disconnect_all();
    }

    void trackable_disconnect(trackable* obj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            typename slot_list::iterator itNext = it;
            ++itNext;

            if((*it)->get_dest() == obj)
            {
                delete *it;
                m_slots.erase(it);
            }

            it = itNext;
        }
    }

    void trackable_duplicate(const trackable* oldobj, trackable* newobj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest() == oldobj)
            {
                m_slots.push_back((*it)->duplicate(newobj));
            }

            ++it;
        }
    }

    void connection_connect(connection* conn)
    {
        mutex_locker lock;

        m_connections.push_back(conn);
    }

    void connection_disconnect(connection* conn)
    {
        mutex_locker lock;

        connection_list::iterator it = m_connections.begin();
        connection_list::iterator itEnd = m_connections.end();

        while(it != itEnd)
        {
            if(*it == conn)
            {
                m_connections.erase(it);
                break;
            }
            ++it;
        }
    }

    // Connect to a normal global function
    connection connect_ptrfun(void (*fn)(T1, T2, T3, T4, T5), int unique = 0)
    {
        mutex_locker lock;
        
        slot_ptrfun5<T1, T2, T3, T4, T5>* slot = new slot_ptrfun5<T1, T2, T3, T4, T5>(fn);
        if(unique)
        {
            typename slot_list::iterator it = m_slots.begin();
            typename slot_list::iterator itEnd = m_slots.end();
            while(it != itEnd)
            {
                if(dynamic_cast<decltype(slot)>(*it)->m_fn == fn)
                {
                    delete slot;
                    return connection();
                }
                ++it;
            }
        }
        m_slots.push_back(slot);

        return connection(this, slot);
    }
    
    template <typename T_obj>
    connection connect_memfun(T_obj* obj, void (T_obj::*fn)(T1, T2, T3, T4, T5))
    {
        mutex_locker lock;
        
        slot_memfun5<T_obj, T1, T2, T3, T4, T5>* slot = new slot_memfun5<T_obj, T1, T2, T3, T4, T5>(obj, fn);
        m_slots.push_back(slot);
        
        obj->signal_connect(this);

        return connection(this, slot);
    }

    void disconnect(trackable* obj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        // Disconnect ALL slots that connect to the trackable,
        // not just the first one
        while(it != itEnd)
        {
            typename slot_list::iterator itNext = it;
            ++itNext;

            if((*it)->get_dest() == obj)
            {
                if(obj)
                    obj->signal_disconnect(this);

                delete *it;
                m_slots.erase(it);
            }

            it = itNext;
        }
    }

    void disconnect(slot_base* slot)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();
        trackable* dest = 0;

        while(it != itEnd)
        {
            if(*it == slot)
            {
                dest = (*it)->get_dest();
                delete *it;
                m_slots.erase(it);
                break;
            }

            ++it;
        }

        // Don't call signal_disconnect unless no other slots
        // have the same destination
        if(dest)
        {
            it = m_slots.begin();
            itEnd = m_slots.end();

            while(it != itEnd)
            {
                if((*it)->get_dest() == dest)
                {
                    break;
                }

                ++it;
            }

            if(it == itEnd)
            {
                // No other slot uses the same trackable
                dest->signal_disconnect(this);
            }
        }
    }

    void disconnect_all()
    {
        mutex_locker lock;

        typename slot_list::const_iterator it = m_slots.begin();
        typename slot_list::const_iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest())
            {
                (*it)->get_dest()->signal_disconnect(this);
            }

            delete *it;

            ++it;
        }

        m_slots.erase(m_slots.begin(), m_slots.end());

        // Also reset any connection pointers.  This is okay
        // even though we may not be gettin destroyed yet
        connection_list::const_iterator connit = m_connections.begin();
        connection_list::const_iterator connitEnd = m_connections.end();

        while(connit != connitEnd)
        {
            (*connit)->signal_disconnect();
            ++connit;
        }

        m_connections.erase(m_connections.begin(), m_connections.end());
    }

    void emit(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5)
    {
        mutex_locker lock;
        
        typename slot_list::const_iterator it = this->m_slots.begin();
        typename slot_list::const_iterator itEnd = this->m_slots.end();
        
        while(it != itEnd)
        {
            typename slot_list::const_iterator itNext = it;
            ++itNext;
            
            (*it)->emit(a1, a2, a3, a4, a5);
                
            it = itNext;
        }
    }

    void operator()(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5)
    {
        return emit(a1, a2, a3, a4, a5);
    }

protected:
    slot_list m_slots;
    connection_list m_connections;
};
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
class signal6 : public signal_base
{
protected:
    typedef slot_base6<T1, T2, T3, T4, T5, T6> slot_type;
    typedef std::list<slot_type*> slot_list;
    typedef std::list<connection*> connection_list;

public:
    signal6() { }

    signal6(const signal6<T1, T2, T3, T4, T5, T6>& copy)
    {
        mutex_locker lock;

        typename slot_list::const_iterator it = copy.m_slots.begin();
        typename slot_list::const_iterator itEnd = copy.m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest())
            {
                (*it)->get_dest()->signal_connect(this);
            }
            m_slots.push_back((*it)->clone());

            ++it;
        }
    }

    ~signal6()
    {
        disconnect_all();
    }

    void trackable_disconnect(trackable* obj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            typename slot_list::iterator itNext = it;
            ++itNext;

            if((*it)->get_dest() == obj)
            {
                delete *it;
                m_slots.erase(it);
            }

            it = itNext;
        }
    }

    void trackable_duplicate(const trackable* oldobj, trackable* newobj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest() == oldobj)
            {
                m_slots.push_back((*it)->duplicate(newobj));
            }

            ++it;
        }
    }

    void connection_connect(connection* conn)
    {
        mutex_locker lock;

        m_connections.push_back(conn);
    }

    void connection_disconnect(connection* conn)
    {
        mutex_locker lock;

        connection_list::iterator it = m_connections.begin();
        connection_list::iterator itEnd = m_connections.end();

        while(it != itEnd)
        {
            if(*it == conn)
            {
                m_connections.erase(it);
                break;
            }
            ++it;
        }
    }

    // Connect to a normal global function
    connection connect_ptrfun(void (*fn)(T1, T2, T3, T4, T5, T6), int unique = 0)
    {
        mutex_locker lock;
        
        slot_ptrfun6<T1, T2, T3, T4, T5, T6>* slot = new slot_ptrfun6<T1, T2, T3, T4, T5, T6>(fn);
        if(unique)
        {
            typename slot_list::iterator it = m_slots.begin();
            typename slot_list::iterator itEnd = m_slots.end();
            while(it != itEnd)
            {
                if(dynamic_cast<decltype(slot)>(*it)->m_fn == fn)
                {
                    delete slot;
                    return connection();
                }
                ++it;
            }
        }
        m_slots.push_back(slot);

        return connection(this, slot);
    }
    
    template <typename T_obj>
    connection connect_memfun(T_obj* obj, void (T_obj::*fn)(T1, T2, T3, T4, T5, T6))
    {
        mutex_locker lock;
        
        slot_memfun6<T_obj, T1, T2, T3, T4, T5, T6>* slot = new slot_memfun6<T_obj, T1, T2, T3, T4, T5, T6>(obj, fn);
        m_slots.push_back(slot);
        
        obj->signal_connect(this);

        return connection(this, slot);
    }

    void disconnect(trackable* obj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        // Disconnect ALL slots that connect to the trackable,
        // not just the first one
        while(it != itEnd)
        {
            typename slot_list::iterator itNext = it;
            ++itNext;

            if((*it)->get_dest() == obj)
            {
                if(obj)
                    obj->signal_disconnect(this);

                delete *it;
                m_slots.erase(it);
            }

            it = itNext;
        }
    }

    void disconnect(slot_base* slot)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();
        trackable* dest = 0;

        while(it != itEnd)
        {
            if(*it == slot)
            {
                dest = (*it)->get_dest();
                delete *it;
                m_slots.erase(it);
                break;
            }

            ++it;
        }

        // Don't call signal_disconnect unless no other slots
        // have the same destination
        if(dest)
        {
            it = m_slots.begin();
            itEnd = m_slots.end();

            while(it != itEnd)
            {
                if((*it)->get_dest() == dest)
                {
                    break;
                }

                ++it;
            }

            if(it == itEnd)
            {
                // No other slot uses the same trackable
                dest->signal_disconnect(this);
            }
        }
    }

    void disconnect_all()
    {
        mutex_locker lock;

        typename slot_list::const_iterator it = m_slots.begin();
        typename slot_list::const_iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest())
            {
                (*it)->get_dest()->signal_disconnect(this);
            }

            delete *it;

            ++it;
        }

        m_slots.erase(m_slots.begin(), m_slots.end());

        // Also reset any connection pointers.  This is okay
        // even though we may not be gettin destroyed yet
        connection_list::const_iterator connit = m_connections.begin();
        connection_list::const_iterator connitEnd = m_connections.end();

        while(connit != connitEnd)
        {
            (*connit)->signal_disconnect();
            ++connit;
        }

        m_connections.erase(m_connections.begin(), m_connections.end());
    }

    void emit(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6)
    {
        mutex_locker lock;
        
        typename slot_list::const_iterator it = this->m_slots.begin();
        typename slot_list::const_iterator itEnd = this->m_slots.end();
        
        while(it != itEnd)
        {
            typename slot_list::const_iterator itNext = it;
            ++itNext;
            
            (*it)->emit(a1, a2, a3, a4, a5, a6);
                
            it = itNext;
        }
    }

    void operator()(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6)
    {
        return emit(a1, a2, a3, a4, a5, a6);
    }

protected:
    slot_list m_slots;
    connection_list m_connections;
};
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
class signal7 : public signal_base
{
protected:
    typedef slot_base7<T1, T2, T3, T4, T5, T6, T7> slot_type;
    typedef std::list<slot_type*> slot_list;
    typedef std::list<connection*> connection_list;

public:
    signal7() { }

    signal7(const signal7<T1, T2, T3, T4, T5, T6, T7>& copy)
    {
        mutex_locker lock;

        typename slot_list::const_iterator it = copy.m_slots.begin();
        typename slot_list::const_iterator itEnd = copy.m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest())
            {
                (*it)->get_dest()->signal_connect(this);
            }
            m_slots.push_back((*it)->clone());

            ++it;
        }
    }

    ~signal7()
    {
        disconnect_all();
    }

    void trackable_disconnect(trackable* obj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            typename slot_list::iterator itNext = it;
            ++itNext;

            if((*it)->get_dest() == obj)
            {
                delete *it;
                m_slots.erase(it);
            }

            it = itNext;
        }
    }

    void trackable_duplicate(const trackable* oldobj, trackable* newobj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest() == oldobj)
            {
                m_slots.push_back((*it)->duplicate(newobj));
            }

            ++it;
        }
    }

    void connection_connect(connection* conn)
    {
        mutex_locker lock;

        m_connections.push_back(conn);
    }

    void connection_disconnect(connection* conn)
    {
        mutex_locker lock;

        connection_list::iterator it = m_connections.begin();
        connection_list::iterator itEnd = m_connections.end();

        while(it != itEnd)
        {
            if(*it == conn)
            {
                m_connections.erase(it);
                break;
            }
            ++it;
        }
    }

    // Connect to a normal global function
    connection connect_ptrfun(void (*fn)(T1, T2, T3, T4, T5, T6, T7), int unique = 0)
    {
        mutex_locker lock;
        
        slot_ptrfun7<T1, T2, T3, T4, T5, T6, T7>* slot = new slot_ptrfun7<T1, T2, T3, T4, T5, T6, T7>(fn);
        if(unique)
        {
            typename slot_list::iterator it = m_slots.begin();
            typename slot_list::iterator itEnd = m_slots.end();
            while(it != itEnd)
            {
                if(dynamic_cast<decltype(slot)>(*it)->m_fn == fn)
                {
                    delete slot;
                    return connection();
                }
                ++it;
            }
        }
        m_slots.push_back(slot);

        return connection(this, slot);
    }
    
    template <typename T_obj>
    connection connect_memfun(T_obj* obj, void (T_obj::*fn)(T1, T2, T3, T4, T5, T6, T7))
    {
        mutex_locker lock;
        
        slot_memfun7<T_obj, T1, T2, T3, T4, T5, T6, T7>* slot = new slot_memfun7<T_obj, T1, T2, T3, T4, T5, T6, T7>(obj, fn);
        m_slots.push_back(slot);
        
        obj->signal_connect(this);

        return connection(this, slot);
    }

    void disconnect(trackable* obj)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();

        // Disconnect ALL slots that connect to the trackable,
        // not just the first one
        while(it != itEnd)
        {
            typename slot_list::iterator itNext = it;
            ++itNext;

            if((*it)->get_dest() == obj)
            {
                if(obj)
                    obj->signal_disconnect(this);

                delete *it;
                m_slots.erase(it);
            }

            it = itNext;
        }
    }

    void disconnect(slot_base* slot)
    {
        mutex_locker lock;

        typename slot_list::iterator it = m_slots.begin();
        typename slot_list::iterator itEnd = m_slots.end();
        trackable* dest = 0;

        while(it != itEnd)
        {
            if(*it == slot)
            {
                dest = (*it)->get_dest();
                delete *it;
                m_slots.erase(it);
                break;
            }

            ++it;
        }

        // Don't call signal_disconnect unless no other slots
        // have the same destination
        if(dest)
        {
            it = m_slots.begin();
            itEnd = m_slots.end();

            while(it != itEnd)
            {
                if((*it)->get_dest() == dest)
                {
                    break;
                }

                ++it;
            }

            if(it == itEnd)
            {
                // No other slot uses the same trackable
                dest->signal_disconnect(this);
            }
        }
    }

    void disconnect_all()
    {
        mutex_locker lock;

        typename slot_list::const_iterator it = m_slots.begin();
        typename slot_list::const_iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            if((*it)->get_dest())
            {
                (*it)->get_dest()->signal_disconnect(this);
            }

            delete *it;

            ++it;
        }

        m_slots.erase(m_slots.begin(), m_slots.end());

        // Also reset any connection pointers.  This is okay
        // even though we may not be gettin destroyed yet
        connection_list::const_iterator connit = m_connections.begin();
        connection_list::const_iterator connitEnd = m_connections.end();

        while(connit != connitEnd)
        {
            (*connit)->signal_disconnect();
            ++connit;
        }

        m_connections.erase(m_connections.begin(), m_connections.end());
    }

    void emit(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7)
    {
        mutex_locker lock;
        
        typename slot_list::const_iterator it = this->m_slots.begin();
        typename slot_list::const_iterator itEnd = this->m_slots.end();
        
        while(it != itEnd)
        {
            typename slot_list::const_iterator itNext = it;
            ++itNext;
            
            (*it)->emit(a1, a2, a3, a4, a5, a6, a7);
                
            it = itNext;
        }
    }

    void operator()(T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6, T7 a7)
    {
        return emit(a1, a2, a3, a4, a5, a6, a7);
    }

protected:
    slot_list m_slots;
    connection_list m_connections;
};


} // namespace


#endif // __BAVII_SSNS_H__

