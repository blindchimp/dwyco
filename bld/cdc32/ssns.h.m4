changequote([,])dnl
divert([-1])

dnl How many arguments should be supported
define(SSNS_ARGCOUNT,7)

dnl This is a helper to create a loop of sequentially named items with
dnl a comma between them: loop(4,[i],[T[]i]) = 'T1, T2, T3, T4'  it is
dnl used for argument and template parameter lists.  If '0' is specified
dnl it will expand to nothing
define([loop], [ifelse($1,0,[],[pushdef([$2], [1])_loop($@)popdef([$2])])])
define([_loop],
       [$3[]ifelse($2, [$1], [], [define([$2], incr($2))[, ]$0($@)])])

dnl This one is similar to above, but does not put in the comma and
dnl space between the items.  Also, it starts at 0 instead of 1. It is
dnl used for repeating the needed definitions of the classes for as
dnl many arguemtns are desired.  If 0 is specified, it will only
dnl expand the base class with no arguments.
define([repeat], [pushdef([$2], [0])_repeat($@)popdef([$2])])
define([_repeat],
       [$3[]ifelse($2, [$1], [], [define([$2], incr($2))$0($@)])])


dnl Slot base for specify slot signature
define([SLOT_BASE], 
[ifelse($1,0,[],[template <loop($1,[i],[typename T[]i])>])
class slot_base$1 : public slot_base
{
public:
    virtual void emit(loop($1,[i],[T[]i])) = 0;
    virtual slot_base$1[]ifelse($1,0,[],[<loop($1,[i],[T[]i])>])* clone() = 0;
    virtual slot_base$1[]ifelse($1,0,[],[<loop($1,[i],[T[]i])>])* duplicate(trackable* newdest) = 0;
};
])

dnl Global function slots
define([SLOT_PTRFUN], 
[ifelse($1,0,[],[template <loop($1,[i],[typename T[]i])>])
class slot_ptrfun$1 : public slot_base$1[]ifelse($1,0,[],[<loop($1,[i],[T[]i])>])
{
//friend class signal$1[]ifelse($1,0,[],[<loop($1,[i],[T[]i])>]);
public:
    slot_ptrfun$1() : m_fn(0)
    {
    }

    slot_ptrfun$1(void (*fn)(loop($1,[i],[T[]i]))) : m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return 0;
    }

    void emit(loop($1,[i],[T[]i a[]i]))
    {
        (*m_fn)(loop($1,[i],[a[]i]));
    }

    slot_base$1[]ifelse($1,0,[],[<loop($1,[i],[T[]i])>])* clone()
    {
        return new slot_ptrfun$1[]ifelse($1,0,[],[<loop($1,[i],[T[]i])>])(*this);
    }

    slot_base$1[]ifelse($1,0,[],[<loop($1,[i],[T[]i])>])* duplicate(trackable* newdest)
    {
        return 0;
    }

public:
    void (*m_fn)(loop($1,[i],[T[]i]));
};
])

dnl Member function slots
define([SLOT_MEMFUN], 
[template <typename T_obj[]ifelse($1,0,[],[, loop($1,[i],[typename T[]i])])>
class slot_memfun$1 : public slot_base$1[]ifelse($1,0,[],[<loop($1,[i],[T[]i])>])
{
public:
    slot_memfun$1() : m_obj(0), m_fn(0)
    {
    }
    
    slot_memfun$1(T_obj* obj, void (T_obj::*fn)(loop($1,[i],[T[]i]))) : m_obj(obj), m_fn(fn)
    {
    }

    trackable* get_dest()
    {
        return m_obj;
    }

    void emit(loop($1,[i],[T[]i a[]i]))
    {
        (m_obj->*m_fn)(loop($1,[i],[a[]i]));
    }

    slot_base$1[]ifelse($1,0,[],[<loop($1,[i],[T[]i])>])* clone()
    {
        return new slot_memfun$1<T_obj[]ifelse($1,0,[],[, loop($1,[i],[T[]i])])>(*this);
    }

    slot_base$1[]ifelse($1,0,[],[<loop($1,[i],[T[]i])>])* duplicate(trackable* newdest)
    {
        return new slot_memfun$1<T_obj[]ifelse($1,0,[],[, loop($1,[i],[T[]i])])>((T_obj*)newdest, m_fn);
    }

private:
    T_obj* m_obj;
    void (T_obj::*m_fn)(loop($1,[i],[T[]i]));
};
])

dnl Signal of a specific signature
define([SIGNAL], 
[ifelse($1,0,[],[template <loop($1,[i],[typename T[]i])>])
class signal$1 : public signal_base
{
protected:
    typedef slot_base$1[]ifelse($1,0,[],[<loop($1,[i],[T[]i])>]) slot_type;
    typedef std::list<slot_type*> slot_list;
    typedef std::list<connection*> connection_list;

public:
    signal$1() { }

    signal$1(const signal$1[]ifelse($1,0,[],[<loop($1,[i],[T[]i])>])& copy)
    {
        mutex_locker lock;

        ifelse($1,0,[],[typename]) slot_list::const_iterator it = copy.m_slots.begin();
        ifelse($1,0,[],[typename]) slot_list::const_iterator itEnd = copy.m_slots.end();

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

    ~signal$1()
    {
        disconnect_all();
    }

    void trackable_disconnect(trackable* obj)
    {
        mutex_locker lock;

        ifelse($1,0,[],[typename]) slot_list::iterator it = m_slots.begin();
        ifelse($1,0,[],[typename]) slot_list::iterator itEnd = m_slots.end();

        while(it != itEnd)
        {
            ifelse($1,0,[],[typename]) slot_list::iterator itNext = it;
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

        ifelse($1,0,[],[typename]) slot_list::iterator it = m_slots.begin();
        ifelse($1,0,[],[typename]) slot_list::iterator itEnd = m_slots.end();

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
    connection connect_ptrfun(void (*fn)(loop($1,[i],[T[]i])), int unique = 0)
    {
        mutex_locker lock;
        
        slot_ptrfun$1[]ifelse($1,0,[],[<loop($1,[i],[T[]i])>])* slot = new slot_ptrfun$1[]ifelse($1,0,[],[<loop($1,[i],[T[]i])>])(fn);
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
    connection connect_memfun(T_obj* obj, void (T_obj::*fn)(loop($1,[i],[T[]i])))
    {
        mutex_locker lock;
        
        slot_memfun$1<T_obj[]ifelse($1,0,[],[, loop($1,[i],[T[]i])])>* slot = new slot_memfun$1<T_obj[]ifelse($1,0,[],[, loop($1,[i],[T[]i])])>(obj, fn);
        m_slots.push_back(slot);
        
        obj->signal_connect(this);

        return connection(this, slot);
    }

    void disconnect(trackable* obj)
    {
        mutex_locker lock;

        ifelse($1,0,[],[typename]) slot_list::iterator it = m_slots.begin();
        ifelse($1,0,[],[typename]) slot_list::iterator itEnd = m_slots.end();

        // Disconnect ALL slots that connect to the trackable,
        // not just the first one
        while(it != itEnd)
        {
            ifelse($1,0,[],[typename]) slot_list::iterator itNext = it;
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

        ifelse($1,0,[],[typename]) slot_list::iterator it = m_slots.begin();
        ifelse($1,0,[],[typename]) slot_list::iterator itEnd = m_slots.end();
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

        ifelse($1,0,[],[typename]) slot_list::const_iterator it = m_slots.begin();
        ifelse($1,0,[],[typename]) slot_list::const_iterator itEnd = m_slots.end();

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

    void emit(loop($1,[i],[T[]i a[]i]))
    {
        mutex_locker lock;
        
        ifelse($1,0,[],[typename]) slot_list::const_iterator it = this->m_slots.begin();
        ifelse($1,0,[],[typename]) slot_list::const_iterator itEnd = this->m_slots.end();
        
        while(it != itEnd)
        {
            ifelse($1,0,[],[typename]) slot_list::const_iterator itNext = it;
            ++itNext;
            
            (*it)->emit(loop($1,[i],[a[]i]));
                
            it = itNext;
        }
    }

    void operator()(loop($1,[i],[T[]i a[]i]))
    {
        return emit(loop($1,[i],[a[]i]));
    }

protected:
    slot_list m_slots;
    connection_list m_connections;
};
])

divert[]dnl
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
repeat(SSNS_ARGCOUNT,[i],[SLOT_BASE(i)])

// Global function slots
repeat(SSNS_ARGCOUNT,[i],[SLOT_PTRFUN(i)])

// Member function slots
repeat(SSNS_ARGCOUNT,[i],[SLOT_MEMFUN(i)])

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
repeat(SSNS_ARGCOUNT,[i],[SIGNAL(i)])

} // namespace


#endif // __BAVII_SSNS_H__

