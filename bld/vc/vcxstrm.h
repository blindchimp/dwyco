
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCXSTRM_H
#define VCXSTRM_H
//
// VC xfer stream
// This class provides a number of services to allow easy
// transfer of info between a client of this class and
// an underlying device.
// To use one of these streams, you create a stream object
// and specify the device to use. To xfer data to the device
// you call the in_want/out_want member functions. The
// stream calls the overflow and underflow member functions of the
// device object when physical I/O is necessary.
// There are a variety of styles of I/O & devices supported
// via both constructor args and args to specific I/O calls.
//
//
// $Header: g:/dwight/repo/vc/rcs/vcxstrm.h 1.49 1998/12/09 05:11:28 dwight Exp $

#include "dwvecp.h"
#include "vc.h"
#include "dwgrows.h"
#include "vcdeflt.h"

class vcxstream;

typedef DwVecP<vc_default> ChitTable;

typedef long (*VCXUNDERFUN)(vcxstream&, char *, long min, long max);
typedef long (*VCXOVERFUN)(vcxstream&, char *, long howmuch);
typedef int VCXCHIT;

#define NB_ERROR 0
#define NB_RETRY -1
#define NB_DONE 1

class vcxstream
{
friend vc lh_bf_xfer_enc(vc);
friend vc vclh_bf_xfer_enc_ctx(vc, vc);
friend vc vclh_compress_xfer(vc, vc);
friend vc vclh_serialize(vc);
friend vc serialize(vc);
friend int deserialize(vc, vc&);
friend vc vclh_encdec_xfer_enc_ctx(vc, vc);
friend class vc_uvsocket;
friend class vc_double;
friend class vc_decomposable;
friend class vc_int;
friend class vc_string;
friend class vc;
friend class vc_winsock;
friend class vc_winsock_datagram;

public:
    enum how_close {
        // for readable streams, any unread data in the buffer is discarded.
        // warning: you don't always have control over how much data is
        // in the buffer, so be careful about flushing readable streams.
        // for writable streams, issue whatever i/o operation is
        // necessary to write remaining data out.
        FLUSH,

        // for readable streams, do nothing.
        // for writable streams, make buffer empty without issuing final write.
        DISCARD,

        // only valid for atomic i/o-style (see below.)
        // on readables, pushes back any data that has been read since
        // the last open. leaves the stream in a state such that the pushed
        // back data will be returned by subsequent in_want calls, and then
		// i/o operations will continue if more data is available after
        // the old data is read.
        //
        // for writables,
        RETRY,

        // just reset the chit table, but don't fiddle with the buffers.
        // this should allow multiple open-close pairs on a single
        // stream connected to a device. inhibits the "devclose"
        // operation.
        CONTINUE
    };

    enum status {NOT_OPEN, OPEN, CLOSED, READABLE, WRITEABLE, RETRYING,
                WRITE_RETRY, READ_RETRY};

    enum style {
        // buffer is fixed length, and cannot be changed
        // for readable streams, an i/o operation is performed
		// at open time to try and fill the buffer immediately.
        FIXED,

        // buffer can be extended by up to req length
        CONTINUOUS,

        // buf can be extended by req length and more
        CONTINUOUS_READAHEAD
    };

    enum e_iostyle {
        // for reading, i/o ops are not logged (useful only for blocking devices)
        // for writing, i/o ops are done piecemeal as buffer overflows
        MULTIPLE,

        // for reading, i/o ops are logged, and retry is allowed (on close, data is
        //  reinserted into the stream to be re-read on next in_want)
        // for writing, i/o ops are inhibited and results accumulated in a buffer
        //  which is then written in one i/o op at close time
        ATOMIC
    };

    static long Max_element_len;
    static long Max_elements;
    static long Max_depth;
    static long Max_memory;

	// use this when there is no device, just a fixed buffer
	// 
	//vcxstream(char *buf = 0, long len = 2048, enum style = FIXED);
	vcxstream(const char *buf = 0, long len = 2048, enum style = FIXED);
	
	// not working yet, but this can be used with a
	// disembodied device.
	vcxstream(VCXUNDERFUN underflow, VCXOVERFUN overflow, vc_default *vcp, 
		char *buf = 0, long len = 2048, enum style = CONTINUOUS);
	
	// use this one when you know the lifetime of the device
	// is short (you can pass a pointer to the device without the
	// need to encapsulate it...)
	vcxstream(vc_default *, char *buf = 0, long len = 2048, enum style = CONTINUOUS);
	
	// use this one one if the device and stream are long-lived, or
	// it isn't possible to get at the underlying pointer object.
	vcxstream(vc, char *buf = 0, long len = 2048, enum style = CONTINUOUS);
	
	~vcxstream();

    // open/close pairs is how the xstream object calculates
    // how to record references in structures that contain
    // object that are referenced more than once.
    // as streaming proceeds, the serializer can use the chit
    // functions to represent references to objects it has seen more
    // than once. closing a stream deletes the chit table.
	int open(enum status, enum e_iostyle = MULTIPLE);
	int close(enum how_close how = FLUSH);
    int open2(enum status, enum e_iostyle = MULTIPLE);
    int close2(enum how_close how);
    char *in_want(long);
    char *out_want(long);

    // these are some arbitrary limits you can set and the
    // deserializer will check them and fail if they are
    // exceeded. this is useful if you know the character of
    // the objects that are going to be deserialized, and
    // the input may be from an unknown source.
    // it might be better to provide some means of hooking the
    // memory allocator and terminating after a certain amount of
    // memory had been allocated, but that is environment dependent
    // and would probably require some exception handling.
    // this should be adequate to allow limiting mischief for simple
    // outward facing protocols where the expected form is fairly restricted.
    // NOTE: hitting one of these limits will stop the deserialization
    // and consumption of input in a more or less random place. it is
    // up to the caller to finalize the device (close it, stop using it.)
    // if you don't do this, and continue to read data from the device, you
    // will almost certainly end up with trash.

    // the max number of elements in a composite (like a vector)
    long max_elements;
    // the max length in the representation (in bytes) of any non-composite (int, float, string)
    long max_element_len;
    // the maximum number of levels of the structure to be parsed.
    // for example, just a single composite (like an int) is 0 "levels".
    // this excludes all composites.
    // a single vector is 1 level. if the vector contains another vector, that is 2, and so on.
    // setting this to -1 will return an error for any deserialization.
    long max_depth;

    // max amount of memory for a single deserialization operation.
    // note: this is not checked at allocation time, but rather when
    // a complete deserialization operation is performed. this eliminates
    // situations where constructors might fail. but, it allows the
    // deserialization to continue past this limit.
    void set_max_memory(int);

    int flushnb();
    enum status get_status();
private:
    long max_memory;
    int max_count_digits;  // roughly log10(max_memory)
    unsigned long memory_tally;
friend void* ::operator new(std::size_t sz);
    static unsigned long Memory_tally;

    int retry();
	void put_back(const char *, long);
	int flush();

	int has_input();

	vc_default *chit_get(VCXCHIT);
	void chit_append(vc_default *);
	VCXCHIT chit_find(vc_default *);
	void chit_new_table();
	void commit();
    int check_status(enum status);

    // being able to serialize things that were non-trees
    // seemed like a good idea. but honestly, i think it is a waste
    // at this point.
    //
    // let's just avoid dealing with self-referential things. it works, but
    // if the caller sends in something with a cycle, it creates garbage and might end up as a DOS.
    // in 20+ years i've never needed this functionality, so let's just
    // disable it by default.
    //
    // NOTE: we just rely on the visited flag, and ignore the chit_table. if you send in a
    // vc that is not a tree, it is almost certainly a programming error, so we panic.
    // just to illustrate:
    // lbind(empty_vec vector())
    // lbind(dag vector(<empty_vec> <empty_vec>))
    // <<serialize dag someway>> results in a panic.
    //
    // but, serializing
    // lbind(dag vector(vector() vector()))
    // works fine, since the two inner vectors are distinct
    //
    // NOTE2: there is nothing in the encoding that says "this should be non-self-referential".
    // so on the decode side, you have to make sure this self_ref flag gets set out of band if you *do* want
    // to allow self-ref.

    // dammit: the above is mostly true, but there is code in the servers that inadvertently creates
    // non-tree structures, from processing commands recursively. there is a lot of code to look
    // at to get rid of this "accident" with processing "auth-command" encapsulated items. the problem
    // crops up during return processing with referencing semi-global error and extra return values.
    bool allow_self_ref = true;
	ChitTable *chit_table;
    void chit_destroy_table();
	
	// tells which device to use
	enum devtype {INDEPENDENT, VCPTR, VC, NONE} dtype;
	VCXUNDERFUN uflow;
	VCXOVERFUN oflow;
	vc_default *manager;	// objects whose under/overflow functions to invoke
	vc mgr;

	int read_only;
	
	char *buf;		// pointer to beginning of buffer
	char *cur;		// current read/write pointer
	char *eob;		// one character past end of buf
    char *eod;		// one character past end of valid data for reading
    long len;		// allocated size pointed to by buf
	enum status stat;
	enum style styl;
	enum e_iostyle iostyle;
	int own_buf;	// 1 means we can alloc/dealloc buffer

	DwGrowingString log;
	
	long do_overflow(char *buf, long len);
    // this calls back into the "underflow" method on the device (or whatever is set up to handle reading from a device.)
    // the function returns the number of bytes actually read.
    // if you don't consume *any* bytes from the device, return -1.
    // it is *best* if you return at least min bytes, BUT if you don't, the stream
    // will buffer what you *do* return (but in_want will return 0.) you cannot
    // return more than max bytes (the stream only has enough space that much at most.)
	long do_underflow(char *buf, long min, long max);
	long do_devopen(int style, int stat);
	long do_devclose(int style);

};

#endif
