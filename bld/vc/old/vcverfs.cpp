#include "vc.h"
#ifdef USE_VERFS
// $Header: g:/dwight/repo/vc/rcs/vcverfs.cpp 1.4 1999/02/10 22:38:30 dwight Exp $
// vc encapsulation for verfs library
//

// RID = int
// key type = {nil, int, float, string}
// bag type = {hash, data, tree}
// mode = {read, read-write}
// openbag = int, long in 64-bitland
// TXN = int
// bagname = int
// data read and written are serialized vc structures, no schema.
// scan handle = int, long in 64-bitland
// 
// note: because of all the setjmp stuff, some of the code in this
// file probably needs to have some volatile decls, but it appears
// to work ok without it. and putting the volatiles in causes other
// compilation problems, so for now, i've left it out.
// read the longjmp man page for details.
// at some point, the verfs lib should probably be converted to c++
// exceptions. this file assumes we are calling into a C lib that
// handles the setjmp/longjump exception handling stuff ok.

#include "vcmap.h"
#include "dwvec.h"
#include "vcxstrm.h"
#include "vccomp.h"

#include "pl_inter.h"
#include "exc.h"
#include "pl_stats.h"
extern struct pl_phys_stats Pl_phys_stats; /* misc. stats */
extern struct pl_buf_stats Pl_buf_stats;
int pldm_max_obj_size();


#define EXC(ret) if(exc_handle("*", plexc) != 0) return ret;
#define EXC2(stmt, ret) if(exc_handle("*", plexc) != 0) {{stmt} return ret;}

static vc Hash("hash");
static vc Data("data");
static vc KeyInt("int");
static vc KeyString("string");
static vc KeyFloat("float");
static vc KeyByte("byte");
static vc Read("r");
static vc ReadWrite("rw");
static vc HintNewFrame("new-frame");
static vc HintNewCluster("new-cluster");
static vc HintDontCare("dont-care");

int
plexc(char **excv, char *)
{
	// construct exception string and raise LH exception
	VCArglist a;
	for(int i = 0; excv[i]; ++i)
		a[i] = vc(excv[i]);
	Vcmap->excraise(excv[0], &a);
	CHECK_ANY_BO(1);
	return 0;
}
/*
s/$/EXC
*/

EXC_CTX_STD(vc,
vcverfs_init,(vc dir, vc file),(dir, file)
){
	EXC(vcnil)
	pldm_init((char *)(const char *)dir, (char *)(const char *)file, 0, 0);
	return vctrue;
}

EXC_CTX_STD(vc,
vcverfs_exit,(),()
){
	EXC(vcnil)
	pldm_exit();
	return vctrue;
}

EXC_CTX_STD(vc,
vcverfs_txn_start,(),()
){
	EXC(vcnil)
	TXN_ID t = txn_start();
	return vc((int)t);
}

EXC_CTX_STD(vc,
vcverfs_txn_abort,(vc txn),(txn)
){
	EXC(vcnil)
	txn_abort(txn);
	return vctrue;
}

EXC_CTX_STD(vc,
vcverfs_txn_commit,(vc txn, vc dur),(txn, dur)
){
	EXC(vcnil)
	txn_commit(txn, dur.is_nil() ? 0 : 1);
	return vctrue;
}

EXC_CTX_STD(vc,
vcverfs_bag_open,(vc num, vc mode, vc txn),(num, mode, txn)
){
	EXC(vcnil)
	BAG_NUMBER b = (BAG_NUMBER)(long)num;
	TXN_ID tx = (TXN_ID)(long)txn;
	enum bag_mode bm = BAG_RD;

	if(mode == Read)
		bm = BAG_RD;
	else if(mode == ReadWrite)
		bm = BAG_RDWR;

	BAG bag = bag_open(b, bm, tx);
	return vc((long)bag);
}

EXC_CTX_STD(vc,
vcverfs_bag_close,(vc bag),(bag)
){
	EXC(vcnil)
	BAG bh = (BAG) (long) bag;
	bag_close(bh);
	return vctrue;
}

EXC_CTX_STD(vc,
vcverfs_bag_create,(vc type, vc key, vc parentnum, vc txn),(type, key, parentnum, txn)
){
	EXC(vcnil)
	BAG_NUMBER b = (BAG_NUMBER)(long)parentnum;
	TXN_ID tx = txn;
	enum bag_type bt;
	enum key_type kt;

	if(type == Hash)
	{
		bt = HASH_BAG;
		if(key == KeyInt)
			kt = INT_KEY;
		else if(key == KeyString)
			kt = STRING_KEY;
		else if(key == KeyFloat)
			kt = FLOAT_KEY;
		else if(key == KeyByte)
			kt = BYTE_KEY;
	}
	else if(type == Data)
	{
		kt = NO_KEY;
		bt = DATA_BAG;
	}


	BAG bag = bag_create(bt, kt, b, tx);
	return vc((long)bag);
}

EXC_CTX_STD(vc,
vcverfs_bag_remove,(vc num, vc txn),(num, txn)
){
	EXC(vcnil)
	BAG_NUMBER b = (BAG_NUMBER)(long)num;
	long recovered = bag_remove(b, (TXN_ID)(long)txn);
	return vc(recovered);
}

EXC_CTX_STD(vc,
vcverfs_bag_exists,(vc num, vc txn),(num, txn)
){
	EXC(vcnil)
	int b = bag_exists((BAG_NUMBER)(long)num, (TXN_ID)(long)txn);
	if(b)
		return vctrue;
	return vcnil;
}

EXC_CTX_STD(vc,
vcverfs_bag_number,(vc bag),(bag)
){
	EXC(vcnil)
	BAG_NUMBER b = bag_number((BAG)(long)bag);
	return vc((long)b);
}

// data bag functions
EXC_CTX_STD(vc,
vcverfs_bag_data_write,(vc bag, vc hintrid, vc data, vc txn),(bag, hintrid, data, txn)
){

	char *buf = new char[pldm_max_obj_size()];
	vcxstream strm(buf, pldm_max_obj_size(), vcxstream::FIXED);
	int len;
	RID h;
	RID r;

	// note: this longjmp stuff is dicey, we put this here so
	// the stream objects get cleaned up (before it this was above
	// the decls, and the objects were not cleaned up when
	// an exception was raised.
	EXC2(delete [] buf; , vcnil)

	if(!strm.open(vcxstream::WRITEABLE))
	{
		delete [] buf;
		exc_raise("A:VCVERFS.STREAM_OPEN", "verfs_bag_data_write", NULL);
		// NOTREACHED
		return vcnil;
	}
	vc_composite::new_dfs();
	if((len = data.xfer_out(strm)) < 0)
	{
		delete [] buf;
		exc_raise("A:VCVERFS.XFER_OUT", "verfs_bag_data_write", NULL);
		// NOTREACHED
		return vcnil;
	}
	h = RID_DONT_CARE;
	if(hintrid.is_nil())
		h = RID_DONT_CARE;
	else if(hintrid == HintNewFrame)
		h = RID_NEW_FRAME;
	else if(hintrid == HintNewCluster)
		h = RID_NEW_CLUSTER;
	else if(hintrid == HintDontCare)
		h = RID_DONT_CARE;

	r = bag_data_write((BAG)(long)bag, h, len, buf, (TXN_ID)(long)txn);

	delete [] buf;

	return vc((long)r);
	
}

EXC_CTX_STD(vc,
vcverfs_bag_data_read,(vc bag, vc rid, vc data_out, vc txn),(bag, rid, data_out, txn)
){
	
	BAG b = (BAG)(long)bag;
	RID r = (RID)(long)rid;

	EXC(vcnil)

	int len = bag_data_rec_size(b, r, (TXN_ID)(long)txn);

	char *buf = new char[len];
	vcxstream strm(buf, len, vcxstream::FIXED);

	// see notes above on setjmp bogosity
	EXC2(delete [] buf; , vcnil)

	int got = bag_data_read(b, r, len, buf, 0, (TXN_ID)(long)txn);
	if(got != len)
	{
		delete [] buf;
		buf = 0;
		exc_raise("A:VCVERFS.BAD_READ_LEN", "verfs_bag_data_read", NULL);
		// NOTREACHED
		return vcnil;
	}
	
	if(!strm.open(vcxstream::READABLE))
	{
		delete [] buf;
		buf = 0;
		exc_raise("A:VCVERFS.BAD_STREAM_OPEN", "verfs_bag_data_read", NULL);
		// NOTREACHED
		return vcnil;
	}
	vc v;
	if(v.xfer_in(strm) != len)
	{
		delete [] buf;
		buf = 0;
		exc_raise("A:VCVERFS.BAD_XFER_IN", "verfs_bag_data_read", NULL);
		// NOTREACHED
		return vcnil;
	}
	data_out.local_bind(v);
	delete [] buf;
	return vctrue;
}

EXC_CTX_STD(vc,
vcverfs_bag_data_remove,(vc bag, vc rid, vc txn),(bag, rid, txn)
){
	EXC(vcnil)

	bag_data_remove((BAG)(long)bag, (RID)(long)rid, (TXN_ID)(long)txn);
	return vctrue;
}


EXC_CTX_STD(vc,
vcverfs_bag_data_replace,(vc bag, vc rid, vc data, vc txn),(bag, rid, data, txn)
){
	char *buf = new char[pldm_max_obj_size()];
	vcxstream strm(buf, pldm_max_obj_size(), vcxstream::FIXED);

	// see notes above on setjmp bogons
	EXC2(delete [] buf; , vcnil)

	if(!strm.open(vcxstream::WRITEABLE))
	{
		delete [] buf;
		buf = 0;
		exc_raise("A:VCVERFS.BAD_STREAM_OPEN", "verfs_bag_data_replace", NULL);
		// NOTREACHED
		return vcnil;
	}
	int len;
	vc_composite::new_dfs();
	if((len = data.xfer_out(strm)) < 0)
	{
		delete [] buf;
		buf = 0;
		exc_raise("A:VCVERFS.XFER_OUT", "verfs_bag_data_replace", NULL);
		// NOTREACHED
		return vcnil;
	}

	int r = bag_data_replace((BAG)(long)bag, (RID)(long)rid, len, buf, (TXN_ID)(long)txn);

	delete [] buf;

	if(r)
		return vctrue;
	return vcnil;
}

EXC_CTX_STD(vc,
vcverfs_bag_data_record_exists,(vc bag, vc rid, vc txn),(bag, rid, txn)
){
	EXC(vcnil)

	int r = bag_data_record_exists((BAG)(long)bag, (RID)(long)rid, (TXN_ID)(long)txn);
	if(r)
		return vctrue;
	return vcnil;
}

// hash bag functions

EXC_CTX_STD(vc,
vcverfs_bag_hash_insert,(vc bag, vc key, vc data, vc replace, vc txn),(bag, key, data, replace, txn)
){
	char *buf = new char[pldm_max_obj_size()];
	vcxstream strm(buf, pldm_max_obj_size(), vcxstream::FIXED);

	EXC2(delete [] buf; , vcnil)

	if(!strm.open(vcxstream::WRITEABLE))
	{
		delete [] buf;
		buf = 0;
		exc_raise("A:VCVERFS.BAD_STREAM_OPEN", "verfs_bag_has_insert", NULL);
		// NOTREACHED
		return vcnil;
	}
	int len;
	vc_composite::new_dfs();
	if((len = data.xfer_out(strm)) < 0)
	{
		delete [] buf;
		buf = 0;
		exc_raise("A:VCVERFS.XFER_OUT", "verfs_bag_hash_insert", NULL);
		// NOTREACHED
		return vcnil;
	}

	union key k;
	switch(key.type())
	{
	case VC_STRING:
		if(Vcmap->global_contains("OLDHASH"))
			k.s = (char *)(const char *)key;
		else
		{
			k.b.b = (POINTER)(const char *)key;
			k.b.l = key.len();
		}
		break;
	case VC_INT:
		k.i = key;
		break;
	default:
		exc_raise("A:VCVERFS.BAD_KEY_TYPE", "verfs_bag_hash_insert", NULL);
		// NOTREACHED
		USER_BOMB("key must be int or string", vcnil);
	}

	int r = bag_hash_insert((BAG)(long)bag, k, len, buf, replace.is_nil() ? 0 : 1, (TXN_ID)(long)txn);

	delete [] buf;

	if(r)
		return vctrue;
	return vcnil;
}

EXC_CTX_STD(vc,
vcverfs_bag_hash_lookup,(vc bag, vc key, vc data_out, vc txn),(bag, key, data_out, txn)
){
	EXC(vcnil)
	
	union key k;
	switch(key.type())
	{
	case VC_STRING:
		if(Vcmap->global_contains("OLDHASH"))
			k.s = (char *)(const char *)key;
		else
		{
			k.b.b = (POINTER)(const char *)key;
			k.b.l = key.len();
		}
		break;
	case VC_INT:
		k.i = key;
		break;
	default:
		exc_raise("A:VCVERFS.BAD_KEY_TYPE", "verfs_bag_hash_lookup", NULL);
		// NOTREACHED
		USER_BOMB("key must be int or string", vcnil);
	}

	RECLEN len;
	POINTER buf;
	int found = bag_hash_lookup((BAG)(long)bag, k, &len, &buf, (TXN_ID)(long)txn);
	if(!found)
	{
		return vcnil;
	}
	
	vcxstream strm(buf, len, vcxstream::FIXED);

	EXC(vcnil)

	if(!strm.open(vcxstream::READABLE))
	{
		free(buf);
		exc_raise("A:VCVERFS.BAD_STREAM_OPEN", "verfs_bag_hash_lookup", NULL);
		// NOTREACHED
		return vcnil;
	}
	vc v;
	if(v.xfer_in(strm) != len)
	{
		free(buf);
		exc_raise("A:VCVERFS.BAD_XFER_IN", "verfs_bag_data_read", NULL);
		// NOTREACHED
		return vcnil;
	}
	data_out.local_bind(v);
	free(buf);
	return vctrue;
}

EXC_CTX_STD(vc,
vcverfs_bag_hash_delete,(vc bag, vc key, vc txn),(bag, key, txn)
){
	EXC(vcnil)
	
	union key k;
	switch(key.type())
	{
	case VC_STRING:
		if(Vcmap->global_contains("OLDHASH"))
			k.s = (char *)(const char *)key;
		else
		{
			k.b.b = (POINTER)(const char *)key;
			k.b.l = key.len();
		}
		break;
	case VC_INT:
		k.i = key;
		break;
	default:
		exc_raise("A:VCVERFS.BAD_KEY_TYPE", "verfs_bag_hash_delete", NULL);
		// NOTREACHED
		USER_BOMB("key must be int or string", vcnil);
	}

	int ret = bag_hash_delete((BAG)(long)bag, k, (TXN_ID)(long)txn);
	if(ret)
		return vctrue;
	return vcnil;
}

// scan functions

EXC_CTX_STD(vc,
vcverfs_bag_scan_open,(vc bag, vc txn),(bag, txn)
){
	EXC(vcnil)

	BAG_SCAN b = bag_open_scan((BAG)(long)bag, (TXN_ID)(long)txn);
	return vc((long)b);
}

EXC_CTX_STD(vc,
vcverfs_bag_scan_open_logical,(vc bag, vc txn),(bag, txn)
){
	EXC(vcnil)

	BAG_SCAN b = bag_open_scan((BAG)(long)bag, (TXN_ID)(long)txn, 1);
	return vc((long)b);
}

EXC_CTX_STD(vc,
vcverfs_bag_scan_close,(vc bag),(bag)
){
	EXC(vcnil)
	bag_close_scan((BAG_SCAN)(long)bag);
	return vctrue;
}

EXC_CTX_STD(vc,
vcverfs_bag_scan_next_record_raw,(vc bagscan, vc data_out, vc txn),(bagscan, data_out, txn)
){
	EXC(vcnil)
	RECLEN len;
	POINTER buf;
	int found = bag_next_record((BAG_SCAN)(long)bagscan, &len, &buf, (TXN_ID)(int)txn);
	if(!found)
	{
		return vcnil;
	}
	
	vc v(VC_BSTRING, buf, (long)len);
	data_out.local_bind(v);
	return vctrue;
	
}

EXC_CTX_STD(vc,
vcverfs_bag_scan_next_record_raw_logical,(vc bagscan, vc data_out, vc rid_out, vc txn),(bagscan, data_out, rid_out, txn)
){
	EXC(vcnil)
	RECLEN len;
	POINTER buf;
	RID rid;
	int found = bag_next_record((BAG_SCAN)(long)bagscan, &len, &buf, (TXN_ID)(long)txn, &rid);
	if(!found)
	{
		return vcnil;
	}
	
	vc v(VC_BSTRING, buf, (long)len);
	data_out.local_bind(v);
	rid_out.local_bind((long)rid);
	return vctrue;
	
}

EXC_CTX_STD(vc,
vcverfs_bag_scan_next_record_hash,(vc bagscan, vc key_out, vc data_out, vc txn),(bagscan, key_out, data_out, txn)
){
	EXC(vcnil)
	RECLEN len;
	POINTER buf;
	BAG_SCAN bs = (BAG_SCAN)(long)bagscan;
	enum key_type kt = bde_key_type(bs->bag->bde);
	int found = bag_next_record((BAG_SCAN)(long)bagscan, &len, &buf, (TXN_ID)(long)txn);
	if(!found)
	{
		return vcnil;
	}
	int l;
	switch(kt)
	{
	case INT_KEY:
		key_out.local_bind(vc(*(long *)buf));
		buf += sizeof(long);
		len -= sizeof(long);
		break;

	default:
	case FLOAT_KEY:
		::abort();
		break;
	case STRING_KEY:
		key_out.local_bind(vc((char *)buf));
		l = strlen(buf) + 1;
		buf += ROUNDUP(l);
		len -= ROUNDUP(l);
		break;
	case BYTE_KEY:
		key_out.local_bind(vc(VC_BSTRING, buf + sizeof(RECLEN), *(RECLEN *)buf));
		len -= ROUNDUP(sizeof(RECLEN) + *(RECLEN *)buf);
		buf += ROUNDUP(sizeof(RECLEN) + *(RECLEN *)buf);
		break;


	}
	
	vcxstream strm(buf, len, vcxstream::FIXED);

	EXC(vcnil)

	if(!strm.open(vcxstream::READABLE))
	{
		exc_raise("A:VCVERFS.BAD_STREAM_OPEN", "verfs_bag_scan_next_record_hash", NULL);
		// NOTREACHED
		return vcnil;
	}
	vc v;
	if(v.xfer_in(strm) != len)
	{
		exc_raise("A:VCVERFS.BAD_XFER_IN", "verfs_bag_scan_next_record_hash", NULL);
		// NOTREACHED
	}
	data_out.local_bind(v);
	return vctrue;
	
}

EXC_CTX_STD(vc,
vcverfs_bag_scan_next_record,(vc bagscan, vc data_out, vc txn),(bagscan, data_out, txn)
){
	EXC(vcnil)
	RECLEN len;
	POINTER buf;
	int found = bag_next_record((BAG_SCAN)(long)bagscan, &len, &buf, (TXN_ID)(long)txn);
	if(!found)
	{
		return vcnil;
	}
	
	vcxstream strm(buf, len, vcxstream::FIXED);

	EXC(vcnil)

	if(!strm.open(vcxstream::READABLE))
	{
		exc_raise("A:VCVERFS.BAD_STREAM_OPEN", "verfs_bag_scan_next_record", NULL);
		// NOTREACHED
		return vcnil;
	}
	vc v;
	if(v.xfer_in(strm) != len)
	{
		exc_raise("A:VCVERFS.BAD_XFER_IN", "verfs_bag_scan_next_record", NULL);
		return vcnil;
	}
	data_out.local_bind(v);
	return vctrue;
	
}

EXC_CTX_STD(vc,
vcverfs_bag_scan_next_record_logical,(vc bagscan, vc data_out, vc rid_out, vc txn),(bagscan, data_out, rid_out, txn)
){
	EXC(vcnil)
	RECLEN len;
	POINTER buf;
	RID rid;

	int found = bag_next_record((BAG_SCAN)(long)bagscan, &len, &buf, (TXN_ID)(long)txn, &rid);
	if(!found)
	{
		return vcnil;
	}
	
	vcxstream strm(buf, len, vcxstream::FIXED);

	EXC(vcnil)

	if(!strm.open(vcxstream::READABLE))
	{
		exc_raise("A:VCVERFS.BAD_STREAM_OPEN", "verfs_bag_scan_next_record_logical", NULL);
		// NOTREACHED
		return vcnil;
	}
	vc v;
	if(v.xfer_in(strm) != len)
	{
		exc_raise("A:VCVERFS.BAD_XFER_IN", "verfs_bag_scan_next_record_logical", NULL);
		return vcnil;
	}
	data_out.local_bind(v);
	rid_out.local_bind((long)rid);
	return vctrue;
	
}

EXC_CTX_STD(vc,
vcverfs_bag_scan_next_page,(vc bagscan, vc records_out, vc free_bytes_out, vc txn),(bagscan, records_out, free_bytes_out, txn)
){
	EXC(vcnil)
	RECLEN len;
	int rec_count;

	int found = bag_next_page((BAG_SCAN)(long)bagscan, &rec_count, &len, (TXN_ID)(long)txn);
	if(!found)
	{
		return vcnil;
	}
	
	records_out.local_bind(rec_count);
	free_bytes_out.local_bind((long)len);
	return vctrue;
}

EXC_CTX_STD(vc,
vcverfs_bag_scan_rewind,(vc bagscan, vc txn),(bagscan, txn)
){
	EXC(vcnil)
	bag_rewind_scan((BAG_SCAN)(long)bagscan, (TXN_ID)(long)txn);
	return vctrue;
}

EXC_CTX_STD(vc,
vcverfs_db_get_stats,(),()
){
	EXC(vcnil)
	vc ret(VC_MAP, "", 31);

	 ret.add_kv("new_resource", (int)Pl_phys_stats.new_resource);
	 ret.add_kv("hits", (int)Pl_phys_stats.hits);
	 ret.add_kv("replacements", (int)Pl_phys_stats.replacements);
	 ret.add_kv("db_reads", (int)Pl_phys_stats.db_reads);
	 ret.add_kv("db_writes", (int)Pl_phys_stats.db_writes);
	 ret.add_kv("safe_writes", (int)Pl_phys_stats.safe_writes);
	 ret.add_kv("safe_reads", (int)Pl_phys_stats.safe_reads);
	 ret.add_kv("safe_bp_writes", (int)Pl_phys_stats.safe_bp_writes);


	 ret.add_kv("dirty_reclaims", (int)Pl_buf_stats.dirty_reclaims);	
	 ret.add_kv("dirty_attaches", (int)Pl_buf_stats.dirty_attaches);
	 ret.add_kv("clean_reclaims", (int)Pl_buf_stats.clean_reclaims);
	 ret.add_kv("clean_attaches", (int)Pl_buf_stats.clean_attaches);
	 ret.add_kv("dirty_replaces", (int)Pl_buf_stats.dirty_replaces);
	 ret.add_kv("clean_replaces", (int)Pl_buf_stats.clean_replaces);
	 ret.add_kv("free_allocations", (int)Pl_buf_stats.free_allocations);
	 ret.add_kv("cache_misses", (int)Pl_buf_stats.cache_misses);
	 ret.add_kv("tossed_bufs", (int)Pl_buf_stats.tossed_bufs);
	 ret.add_kv("lrued_bufs", (int)Pl_buf_stats.lrued_bufs);
	 ret.add_kv("dusted_bufs", (int)Pl_buf_stats.dusted_bufs);
	 ret.add_kv("bfim_creations", (int)Pl_buf_stats.bfim_creations);
	 ret.add_kv("restored_bfims", (int)Pl_buf_stats.restored_bfims);
	 ret.add_kv("detached_bfims", (int)Pl_buf_stats.detached_bfims);
	 ret.add_kv("anti_bfims", (int)Pl_buf_stats.anti_bfims);
	 ret.add_kv("normal_flushes", (int)Pl_buf_stats.normal_flushes);
	 ret.add_kv("safe_flushes", (int)Pl_buf_stats.safe_flushes);
	 ret.add_kv("starts", (int)Pl_buf_stats.starts);
	 ret.add_kv("hard_commits", (int)Pl_buf_stats.hard_commits);
	 ret.add_kv("soft_commits", (int)Pl_buf_stats.soft_commits);
	 ret.add_kv("recovery_commits", (int)Pl_buf_stats.recovery_commits);
	 ret.add_kv("aborts", (int)Pl_buf_stats.aborts);
	 ret.add_kv("free_bufs", (int)Pl_buf_stats.free_bufs);
	 ret.add_kv("fixed_bufs", (int)Pl_buf_stats.fixed_bufs);
	 ret.add_kv("unfixed_bufs", (int)Pl_buf_stats.unfixed_bufs);
	 ret.add_kv("active_bufs", (int)Pl_buf_stats.active_bufs);
	 ret.add_kv("bfim_bufs", (int)Pl_buf_stats.bfim_bufs);
	 ret.add_kv("max_fixed_bufs", (int)Pl_buf_stats.max_fixed_bufs);
	 ret.add_kv("max_bfim_bufs", (int)Pl_buf_stats.max_bfim_bufs);

	return ret;
}

EXC_CTX_STD(vc,
vcverfs_db_get_blocks_free,(),()
){
	EXC(vcnil)
	long pldm_blocks_free(void);
	return pldm_blocks_free();
}

EXC_CTX_STD(vc,
vcverfs_db_special_sync,(),()
){
	EXC(vcnil)
	buf_sync();
	return vcnil;
}

EXC_CTX_STD(vc,
vcverfs_db_set_write_mode,(vc m),(m)
){
	EXC(vcnil)
	if(m == vc("os"))
		phys_set_write_mode(PHYS_WRITE_OS);
	else if(m == vc("sync"))
		phys_set_write_mode(PHYS_WRITE_SYNC);
	else
		USER_BOMB("write mode must be \"os\" or \"sync\"", vcnil);
	return vctrue;
}


static void
makefun(const char *name, const vc& fun)
{
	vc(name).bind(fun);
}

#define VC(fun, nicename, attr) vc(fun, nicename, #fun, attr)
#define VC2(fun, nicename, attr, trans) vc(fun, nicename, #fun, attr, trans)

void
vcverfs_lhinit()
{
	makefun("db-init", VC(vcverfs_init, "db-init", VC_FUNC_BUILTIN_LEAF));
	makefun("db-exit", VC(vcverfs_exit, "db-exit", VC_FUNC_BUILTIN_LEAF));
	
	makefun("txn-start", VC(vcverfs_txn_start, "txn-start", VC_FUNC_BUILTIN_LEAF));
	makefun("txn-abort", VC(vcverfs_txn_abort, "txn-abort", VC_FUNC_BUILTIN_LEAF));
	makefun("txn-commit", VC(vcverfs_txn_commit, "txn-commit", VC_FUNC_BUILTIN_LEAF));
	
	makefun("bag-open", VC(vcverfs_bag_open, "bag-open", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-close", VC(vcverfs_bag_close, "bag-close", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-create", VC(vcverfs_bag_create, "bag-create", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-remove", VC(vcverfs_bag_remove, "bag-remove", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-exists", VC(vcverfs_bag_exists, "bag-exists", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-number", VC(vcverfs_bag_number, "bag-number", VC_FUNC_BUILTIN_LEAF));
	
	makefun("bag-data-read", VC(vcverfs_bag_data_read, "bag-data-read", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-data-write", VC(vcverfs_bag_data_write, "bag-data-write", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-data-replace", VC(vcverfs_bag_data_replace, "bag-data-replace", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-data-remove", VC(vcverfs_bag_data_remove, "bag-data-remove", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-data-record-exists", VC(vcverfs_bag_data_record_exists, "bag-data-record-exists", VC_FUNC_BUILTIN_LEAF));
	
	makefun("bag-hash-insert", VC(vcverfs_bag_hash_insert, "bag-hash-insert", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-hash-lookup", VC(vcverfs_bag_hash_lookup, "bag-hash-lookup", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-hash-delete", VC(vcverfs_bag_hash_delete, "bag-hash-delete", VC_FUNC_BUILTIN_LEAF));
	
	makefun("bag-scan-open", VC(vcverfs_bag_scan_open, "bag-scan-open", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-scan-open-logical", VC(vcverfs_bag_scan_open_logical, "bag-scan-open-logical", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-scan-close", VC(vcverfs_bag_scan_close, "bag-scan-close", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-scan-rewind", VC(vcverfs_bag_scan_rewind, "bag-scan-rewind", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-scan-next-record", VC(vcverfs_bag_scan_next_record, "bag-scan-next-record", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-scan-next-record-logical", VC(vcverfs_bag_scan_next_record_logical, "bag-scan-next-record-logical", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-scan-next-record-hash", VC(vcverfs_bag_scan_next_record_hash, "bag-scan-next-record-hash", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-scan-next-record-raw", VC(vcverfs_bag_scan_next_record_raw, "bag-scan-next-record-raw", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-scan-next-record-raw-logical", VC(vcverfs_bag_scan_next_record_raw_logical, "bag-scan-next-record-raw-logical", VC_FUNC_BUILTIN_LEAF));
	makefun("bag-scan-next-page", VC(vcverfs_bag_scan_next_page, "bag-scan-next-page", VC_FUNC_BUILTIN_LEAF));
	makefun("db-get-stats", VC(vcverfs_db_get_stats, "db-get-stats", VC_FUNC_BUILTIN_LEAF));
	makefun("db-get-blocks-free", VC(vcverfs_db_get_blocks_free, "db-get-blocks-free", VC_FUNC_BUILTIN_LEAF));
	makefun("db-special-sync", VC(vcverfs_db_special_sync, "db-special-sync", VC_FUNC_BUILTIN_LEAF));
	makefun("db-set-write-mode", VC(vcverfs_db_set_write_mode, "db-set-write-mode", VC_FUNC_BUILTIN_LEAF));
}
#endif
