
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#ifndef NO_VCCRYPTO
// $Header: g:/dwight/repo/vc/rcs/vccrypt.cpp 1.8 1999/02/04 18:42:14 dwight Exp $
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <stdlib.h>
#include "vccomp.h"
#include "vcmap.h"
#include "vcxstrm.h"
#include "cryptlib.h"
#include "sha.h"
#include "dh.h"
#include "misc.h"
#include "vcmap.h"
#include "vccrypt2.h"
#include "blowfish.h"
//#include "rng.h"
#include "files.h"
#include "hex.h"
#include "modes.h"
#include "dsa.h"
#include "md5.h"
#include "base64.h"
#include "gcm.h"
#include "aes.h"
#include "randpool.h"
#include "sha3.h"
#ifdef LINUX
#include <unistd.h>
#include <fcntl.h>
#endif


using namespace CryptoPP;
using namespace Weak;

class VCFileSource : public Source
{
public:
    VCFileSource(vc f, int pumpAndClose=FALSE,
               BufferedTransformation *outQueue = new ByteQueue);
	~VCFileSource();

    unsigned int Pump(unsigned int size);
    unsigned long PumpAll();

private:
	vc file;
};

class VCFileSink : public Sink
{
public:
	~VCFileSink();
    VCFileSink(vc f);

    void InputFinished();
    void Put(byte inByte)
    {
		long len = 1;
		file.write(&inByte, len);
    }

    void Put(const byte *inString, unsigned int length);

private:
	vc file;
};



#define VCF_BUFSIZE 1024


VCFileSource::VCFileSource (vc f, int pumpAndClose, BufferedTransformation *outQueue)
    : Source(outQueue), file(f)
{
    if (pumpAndClose)
	{
		PumpAll();
        //Close();
	}
}

VCFileSource::~VCFileSource()
{
}

unsigned int VCFileSource::Pump(unsigned int size)
{
    unsigned int total=0;
	SecByteBlock buffer(dwmin(size, VCF_BUFSIZE));

	int ret = 1;
	while (size && ret)
	{
		long len = dwmin(size, VCF_BUFSIZE);
		ret = file.read((byte *)buffer, len);
		if(ret == 0)
			break;
		AttachedTransformation()->Put(buffer, len);
		size -= len;
		total += len;
	}

    return total;
}

unsigned long VCFileSource::PumpAll()
{
    unsigned long total=0;
    unsigned int l;

    while ((l=Pump(VCF_BUFSIZE)) != 0)
        total += l;

    return total;
}

VCFileSink::VCFileSink(vc f)
    : file(f)
{
}

VCFileSink::~VCFileSink()
{
}

void VCFileSink::InputFinished()
{
}

void VCFileSink::Put(const byte *inString, unsigned int length)
{
	long len = length;
    file.write(inString, len);
}

static vc
hash_file(vc file, HashTransformation& h)
{
	SecByteBlock buffer(VCF_BUFSIZE);
	byte *d = new byte[h.DigestSize()];

	int ret = 1;
	while (ret)
	{
		long len = VCF_BUFSIZE;
		ret = file.read((byte *)buffer.data(), len);
		if(ret == 0 || len == 0)
		{
			h.Final(d);
			vc rv(VC_BSTRING, (const char *)d, h.DigestSize());
			delete [] d;
			return rv;
		}
		h.Update(buffer.data(), len);
	}
	oopanic("hash file wtf");
	return vcnil;
}

vc
vclh_sha256(vc s)
{
	if(s.type() != VC_STRING && s.type() != VC_FILE)
		USER_BOMB("first arg to SHA must be string or file", vcnil);
		
	SHA256 md;
	SecByteBlock b(md.DigestSize());
	if(s.type() == VC_FILE)
	{
		return hash_file(s, md);
	}
	else
	{
		md.Update((const unsigned char *)(const char *)s, s.len());
		md.Final(b);
	}
	vc ret(VC_BSTRING, (const char *)b.data(), (long)md.DigestSize());
	return ret;
}

vc
vclh_sha3_256(vc s)
{
	if(s.type() != VC_STRING && s.type() != VC_FILE)
		USER_BOMB("first arg to SHA must be string or file", vcnil);
		
	SHA3_256 md;
	SecByteBlock b(md.DigestSize());
	if(s.type() == VC_FILE)
	{
		return hash_file(s, md);
	}
	else
	{
		md.Update((const unsigned char *)(const char *)s, s.len());
		md.Final(b);
	}
	vc ret(VC_BSTRING, (const char *)b.data(), (long)md.DigestSize());
	return ret;
}

vc
vclh_sha(vc s)
{
	if(s.type() != VC_STRING && s.type() != VC_FILE)
		USER_BOMB("first arg to SHA must be string or file", vcnil);
		
	SHA md;
	SecByteBlock b(md.DigestSize());
	if(s.type() == VC_FILE)
	{
		return hash_file(s, md);
	}
	else
	{
		md.Update((const unsigned char *)(const char *)s, s.len());
		md.Final(b);
	}
	vc ret(VC_BSTRING, (const char *)b.data(), (long)md.DigestSize());
	return ret;
}

vc
vclh_md5(vc s)
{
	if(s.type() != VC_STRING && s.type() != VC_FILE)
		USER_BOMB("first arg to MD5 must be string or file", vcnil);
		
	MD5 md;
	SecByteBlock b(md.DigestSize());
	if(s.type() == VC_FILE)
	{
		return hash_file(s, md);
	}
	else
	{
		md.Update((const unsigned char *)(const char *)s, s.len());
		md.Final(b);
	}
	vc ret(VC_BSTRING, (const char *)b.data(), (long)md.DigestSize());
	return ret;
}

vc
vclh_base64_encode(vc s)
{
	if(s.type() != VC_STRING && s.type() != VC_FILE)
		USER_BOMB("first arg to base64-encode must be string or file", vcnil);
		
	if(s.type() == VC_FILE)
	{
		oopanic("impl base64 encode");
#if 0
		Base64Encoder *bt = new Base64Encoder;
		VCFileSource vf(s, TRUE, bt);
		unsigned long len = bt->MaxRetrievable();
		byte *tb = new byte[len];
		bt->Get(tb, len);
		vc ret(VC_BSTRING, (const char *)tb, len);
		delete [] tb;
		return ret;
#endif
	}
	else
	{
		Base64Encoder bt;
		bt.Put((const byte *)(const char *)s, s.len());
		bt.MessageEnd();
        auto len = bt.MaxRetrievable();
		byte *tb = new byte[len];
		bt.Get(tb, len);
		vc ret(VC_BSTRING, (const char *)tb, len);
		delete [] tb;
		return ret;
	}
	oopanic("shouldn't get here");
	return vcnil;
}

vc
vclh_base64_decode(vc s)
{
	if(s.type() != VC_STRING && s.type() != VC_FILE)
		USER_BOMB("first arg to base64-decode must be string or file", vcnil);
		
	if(s.type() == VC_FILE)
	{
		oopanic("impl base64 decode");
#if 0
		Base64Decoder *bt = new Base64Decoder;
		VCFileSource vf(s, TRUE, bt);
		unsigned long len = bt->MaxRetrievable();
		byte *tb = new byte[len];
		bt->Get(tb, len);
		vc ret(VC_BSTRING, (const char *)tb, len);
		delete [] tb;
		return ret;
#endif
	}
	else
	{
		Base64Decoder bt;
		bt.Put((const byte *)(const char *)s, s.len());
		bt.MessageEnd();
        auto len = bt.MaxRetrievable();
		byte *tb = new byte[len];
		bt.Get(tb, len);
		vc ret(VC_BSTRING, (const char *)tb, len);
		delete [] tb;
		return ret;
	}
	oopanic("shouldn't get here");
	return vcnil;
}



static DSA::Signer *My_DSA_signer;
static DSA::Verifier *My_DSA_verifier;
static RandomPool *Rng;
static DH *MyDH;
static vc MyDH_pvt;

static void
init_rng()
{
	// seed, don't bother with it, just use
	// whatever rubbish is on the stack.
	// this bothers valgrind, so init it.
	byte a[8];
	byte k[8];
	memset(a, 0, sizeof(a));
	memset(k, 0, sizeof(k));
#ifndef UNIX
	*(int *)&k[0] = rand();
	*(int *)&k[4] = rand();
	*(int *)&a[0] = rand();
	*(int *)&a[4] = rand();
#else
	int fd = open("/dev/urandom", O_RDONLY);
	if(fd < 0)
	{
		*(int *)&k[0] = rand();
		*(int *)&k[4] = rand();
		*(int *)&a[0] = rand();
		*(int *)&a[4] = rand();
	}
	else
	{
		read(fd, k, sizeof(k));
		read(fd, a, sizeof(a));
		close(fd);
	}
#endif
	Rng = new RandomPool;
	Rng->IncorporateEntropy(a, sizeof(a));
	Rng->IncorporateEntropy(k, sizeof(k));
}

vc
vclh_dh_init(vc dhparms_file, vc bits)
{
	if(bits.is_nil())
		bits = vc(2048);
	if(bits.type() != VC_INT)
	{
		USER_BOMB("second arg must be # of bits (int)", vcnil);
		/*NOTREACHED*/
	}
	if(MyDH)
	{
		delete MyDH;
		MyDH = 0;
	}
	if(!Rng)
		init_rng();
	if(dhparms_file.type() != VC_STRING)
	{
		MyDH = new DH(*dynamic_cast<RandomNumberGenerator *>(Rng), (int)bits);
	}
	else
	{
		FileSource f2((const char *)dhparms_file, 1, new HexDecoder);
		MyDH = new DH(f2);
	}
	return vctrue;
}

vc
vclh_dh_init_std()
{

	if(MyDH)
	{
		delete MyDH;
		MyDH = 0;
	}
	if(!Rng)
		init_rng();

	// WARNING: this set of parameters is old and probably not terribly secure any more.
	// and it appears that some of the newer versions of cryptopp may not work reliably with them...
	// for example, on the mac, sometimes key agreement doesn't yield the same results as other
	// platforms. probably need to retire this and have a new key.
	StringSource f2("3081870281810087CD83F01B8F1FD92BD5DD51A3D519C9675211F4364EB62278E2A711B830E2AEF8EE4C0B81A02CEBAD095F55A83B492B8C8A9F0EB22B3E298001D3F2070297D8EB54E806884BC7D8F2E00720C71DF4B30B18CBA6ABDFA202EAF3344D024BBD22E7416D255DAB105E55440712B436C60377A4BC231B4C1EA186BA003B0E1EBA2B020103", 1, new HexDecoder);
	MyDH = new DH(f2);

	return vctrue;
}

vc
vclh_dsa_init(vc file)
{
	if(!Rng)
		init_rng();
	if(My_DSA_signer)
	{
		delete My_DSA_signer;
		My_DSA_signer = 0;
	}
	if(My_DSA_verifier)
	{
		delete My_DSA_verifier;
		My_DSA_verifier = 0;
	}
	if(file.is_nil())
	{
		DSA::PrivateKey privk;
		privk.GenerateRandomWithKeySize(*Rng, 2048);
		DSA::PublicKey pubk;
		pubk.AssignFrom(privk);

		My_DSA_signer = new DSA::Signer(privk);
	}
	else
	{
		FileSource fs(file, true, new HexDecoder);
		My_DSA_signer = new DSA::Signer(fs);
	}

	My_DSA_verifier = new DSA::Verifier(*My_DSA_signer);
	return vcnil;
}

vc
vclh_dsa_save(vc priv_filename, vc pub_filename)
{
	PrivateKey& pvt = My_DSA_signer->AccessPrivateKey();
	CryptoPP::HexEncoder he(new FileSink((const char *)priv_filename));
	pvt.Save(he);

	PublicKey& pbk = My_DSA_verifier->AccessPublicKey();
	CryptoPP::HexEncoder he2(new FileSink((const char *)pub_filename));
	pbk.Save(he2);
	return vcnil;
}

// use this before verifying signature
vc
vclh_dsa_pub_init(vc pub_filename)
{
	delete My_DSA_signer;
	My_DSA_signer = 0;
	delete My_DSA_verifier;
	FileSource fs((const char *)pub_filename, true, new HexDecoder);
	My_DSA_verifier = new DSA::Verifier(fs);
	return vcnil;
}

vc
vclh_dsa_sign(vc m)
{
	if(!My_DSA_signer || !Rng)
	{
		USER_BOMB("DSA not inited", vcnil);
	}
	if(m.type() != VC_STRING)
	{
		USER_BOMB("arg must be a string", vcnil);
	}
	byte *sig = new byte[My_DSA_signer->SignatureLength()];
	My_DSA_signer->SignMessage(*Rng, (const byte *)(const char *)m, m.len(), sig);
	vc ret(VC_BSTRING, (const char *)sig, (long)My_DSA_signer->SignatureLength());
	delete [] sig;
	return ret;
}

vc
vclh_dsa_verify(vc m, vc sig)
{
	if(!My_DSA_verifier)
	{
		USER_BOMB("DSA not inited", vcnil);
	}
	if(m.type() != VC_STRING || sig.type() != VC_STRING)
	{
		USER_BOMB("args must be a strings", vcnil);
	}
	if(sig.len() != My_DSA_verifier->SignatureLength())
	{
		return vcnil;
	}
	if(My_DSA_verifier->VerifyMessage((const byte *)(const char *)m, m.len(), (const byte *)(const char *)sig, sig.len()))
		return vctrue;
	return vcnil;
}


vc
vclh_dh_save(vc filename)
{
	if(!MyDH)
		USER_BOMB("DH not initialized", vcnil);
	if(filename.type() != VC_STRING)
		USER_BOMB("first arg must be filename", vcnil);
	HexEncoder h(new FileSink((const char *)filename));
	MyDH->DEREncode(h);
	return vctrue;
}

vc
vclh_dh_setup()
{
	if(!MyDH || !Rng)
		USER_BOMB("DH not initialized", vcnil);
	byte *pv1 = new byte[MyDH->PublicKeyLength()];
	byte *pvt = new byte[MyDH->PrivateKeyLength()];
	MyDH->GenerateKeyPair(*Rng, pvt, pv1);
	vc ret(VC_BSTRING, (const char *)pv1, (long)MyDH->PublicKeyLength());
	MyDH_pvt = vc(VC_BSTRING, (const char *)pvt, (long)MyDH->PrivateKeyLength());
	delete [] pv1;
	delete [] pvt;
	return ret;
}


vc
vclh_dh_agree(vc other_public)
{
	if(!MyDH || !Rng)
		USER_BOMB("DH not initialized", vcnil);
	if(other_public.type() != VC_STRING)
		USER_BOMB("first arg to dh_agree must be string (other public value)", vcnil);
	byte *pv1 = new byte[MyDH->AgreedValueLength()];
	MyDH->Agree(pv1, (const byte *)(const char *)MyDH_pvt, (byte *)(const char *)other_public);
	vc ret(VC_BSTRING, (const char *)pv1, (long)MyDH->AgreedValueLength());
	delete [] pv1;
	return ret;
}

// do simple encryption of a structure of strings/vectors/ints
// with 80 bit blowfish

#include "filters.h"
#include "blowfish.h"


static ECB_Mode<Blowfish>::Encryption *Be;
static ECB_Mode<Blowfish>::Decryption *Bd;
static StreamTransformationFilter *Bef;
static StreamTransformationFilter *Bdf;
static CTR_Mode<Blowfish>::Encryption *Cme;
static CTR_Mode<Blowfish>::Decryption *Cmd;

static CBC_Mode<Blowfish>::Encryption *CBCe;
static CBC_Mode<Blowfish>::Decryption *CBCd;

vc
lh_bf_init_key(vc key)
{
	// 80 bit blowfish
	if(key.len() < 10)
	{
		USER_BOMB("key must be 80 at least bits", vcnil);
	}
	delete Be;
	delete Bef;
	delete Bd;
	delete Bdf;
	delete Cme;
	delete Cmd;
	delete CBCe;
	delete CBCd;
	Be = 0;
	Bef = 0;
	Bd = 0;
	Bdf = 0;
	Cme = 0;
	Cmd = 0;
	CBCe = 0;
	CBCd = 0;

	Be = new ECB_Mode<Blowfish>::Encryption;
	Be->SetKey((const byte *)(const char *)key, key.len());
	Bef = new StreamTransformationFilter(*Be);

	Bd = new ECB_Mode<Blowfish>::Decryption;
	Bd->SetKey((const byte *)(const char *)key, key.len());
	Bdf = new StreamTransformationFilter(*Bd);

	return vctrue;
}

vc
lh_bf_init_key_stream(vc key1, vc key2)
{
	// 80 bit blowfish
	if(key1.len() < 10 || key2.len() < 10)
	{
		USER_BOMB("key must be at least 80 bits", vcnil);
	}
	delete Be;
	delete Bef;
	delete Bd;
	delete Bdf;
	delete Cme;
	delete Cmd;
	delete CBCe;
	delete CBCd;
	Be = 0;
	Bef = 0;
	Bd = 0;
	Bdf = 0;
	Cme = 0;
	Cmd = 0;
	CBCe = 0;
	CBCd = 0;
	
	Cme = new CTR_Mode<Blowfish>::Encryption;
	Cme->SetKeyWithIV((const byte *)(const char *)key1, key1.len(), (const byte *)"01234567");
	Bef = new StreamTransformationFilter(*Cme);
	Cmd = new CTR_Mode<Blowfish>::Encryption;
	Cmd->SetKeyWithIV((const byte *)(const char *)key2, key2.len(), (const byte *)"01234567");
	Bdf = new StreamTransformationFilter(*Cmd);

	return vctrue;
}

vc
lh_bf_init_key_cbc(vc key, vc iv)
{
	// 80 bit blowfish
	if(key.len() < 10)
	{
		USER_BOMB("key must be at least 80 bits", vcnil);
	}
	byte ivb[8];
	if(iv.type() == VC_INT)
	{
		// last half is rubbish
		*(int *)ivb = (int)iv;
	}
	else if(iv.type() == VC_STRING)
	{
		memcpy(ivb, (const char *)iv, iv.len() > 8 ? 8 : iv.len());
	}
	delete Be;
	delete Bef;
	delete Bd;
	delete Bdf;
	delete Cme;
	delete Cmd;
	delete CBCe;
	delete CBCd;
	Be = 0;
	Bef = 0;
	Bd = 0;
	Bdf = 0;
	Cme = 0;
	Cmd = 0;
	CBCe = 0;
	CBCd = 0;

	CBCe = new CBC_Mode<Blowfish>::Encryption;
	CBCe->SetKeyWithIV((const byte *)(const char *)key, key.len(), ivb);
	Bef = new StreamTransformationFilter(*CBCe);
	CBCd = new CBC_Mode<Blowfish>::Decryption;
	CBCd->SetKeyWithIV((const byte *)(const char *)key, key.len(), ivb);
	Bdf = new StreamTransformationFilter(*CBCd);

	return vctrue;
}

vc
lh_bf_init(vc other_public)
{
	if(other_public.is_nil())
		return vcnil;
	vc key = vclh_dh_agree(other_public);
    // this is a compat hack, just use the first 10 bytes
    // of the agreed value. this is horrifying and
    // this really needs to be ripped out and redone
    key = vc(VC_BSTRING, (const char *)key, 10);
	lh_bf_init_key(key);
	return vctrue;
}

static vc
bf_filter(vc v, BufferedTransformation *f)
{
	f->Put((const byte *)(const char *)v, v.len());
	f->MessageEnd();
    auto m = f->MaxRetrievable();
	byte *b = new byte[m];
	f->Get(b, m);
    vc ret(VC_BSTRING, (const char *)b, m);
	delete [] b;
	return ret;
}


static vc
simple_filter(vc v, BufferedTransformation *f)
{
	int n;
	vc out(VC_VECTOR);
    int i;

	switch(v.type())
	{
	case VC_VECTOR:
		n = v.num_elems();
		for(i = 0; i < n; ++i)
			out[i] = simple_filter(v[i], f);
		return out;
	case VC_STRING:
		return bf_filter(v, f);
	default:
		return v;
	}
}

vc
lh_bf_enc(vc v)
{
    Bef->Initialize();
	return simple_filter(v, Bef);
}

vc
lh_bf_dec(vc v)
{
    Bdf->Initialize();
	return simple_filter(v, Bdf);
}

vc
lh_bf_xfer_enc(vc v)
{
	vcxstream vcx((char *)0, (long)8192, vcxstream::CONTINUOUS);
	long len;
	vc_composite::new_dfs();
	if(!vcx.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
		return vcnil;
	if((len = v.xfer_out(vcx)) == -1)
		return vcnil;

	vc tmp(VC_BSTRING, vcx.buf, vcx.cur - vcx.buf);
	byte iv[8];
	if(!Rng)
		init_rng();
	Rng->GenerateBlock(iv, sizeof(iv));
	CBCe->Resynchronize(iv);

	tmp = lh_bf_enc(tmp);
	
	vc ret(VC_VECTOR);
	vc viv(VC_BSTRING, (char *)iv, sizeof(iv));
	ret.append(viv);
	ret.append(tmp);
	return ret;
}

vc
lh_bf_xfer_dec(vc v, vc out)
{
	if(out.type() != VC_STRING)
	{
		USER_BOMB("second arg to BF-xfer-dec must be a string to bind to", vcnil);
	}
	vc dec;
	if(bf_xfer_dec(v, dec).is_nil())
		return vcnil;
	Vcmap->local_add(out, dec);
	return vctrue;
}

vc
bf_xfer_dec(vc v, vc& out)
{
	if(v.type() != VC_VECTOR || v[0].type() != VC_STRING || v[0].len() != 8 ||
		v[1].type() != VC_STRING || v[1].len() == 0)
	{
		USER_BOMB("BF-xfer-dec arg must be vector(iv, string)", vcnil);
	}
	byte iv[8];
	memcpy(iv, (const char *)v[0], sizeof(iv));
	CBCd->Resynchronize(iv);
	vc tmp;
	try
	{
		tmp = lh_bf_dec(v[1]);
	}
	catch(...)
	{
		return vcnil;
	}

	vcxstream vcx((const char *)tmp, (long)tmp.len(), vcxstream::FIXED);

	vc item;
	long len;
	if(!vcx.open(vcxstream::READABLE))
		return vcnil;
	if((len = item.xfer_in(vcx)) < 0)
		return vcnil;
	out = item;
	return vctrue;
}


#if 0
// as far as i remember, this PK- stuff is a store-and-forward style
// of diffie hellman that is only used in our encrypted backup stuff.
// for now, i'm going to comment it out, and maybe try porting it
// up later.
vc
lh_dh_pubkey(vc pass)
{
	if(!MyDH || !Rng)
		USER_BOMB("DH not initialized", vcnil);
	vc exp = vclh_sha(pass);
	const byte *b = (const byte *)(const char *)exp;
	Integer e(b, 20);
	byte *pv1 = new byte[MyDH->PublicKeyLength()];
	MyDH->Setup(e, pv1);
	vc ret(VC_BSTRING, (const char *)pv1, (long)MyDH->PublicValueLength());
	delete [] pv1;
	return ret;
}

vc
lh_dh_pubkey_enc(vc pub, vc plain)
{
	vc pubs = vclh_dh_setup();
	vc key = vclh_dh_agree(pub);
	// normalize to 160 bits
	key = vclh_sha(key);
	byte *skey = new byte[10];
	Rng->GenerateBlock(skey, 10);
	lh_bf_init_key(vc(VC_BSTRING, (const char *)skey, 10));
	const byte *k = (const byte *)(const char *)key;
	for(int i = 0; i < 10; ++i)
		skey[i] ^= k[i];
	
	vc ret(VC_VECTOR);
	// encrypted output is
	// 0: session key encrypted with public key
	// 1: encrypted plaintext encrypted with blowfish using session key
	// 2: public DH value to decrypt
	ret.append(vc(VC_BSTRING, (const char *)skey, 10));
	ret.append(lh_bf_enc(plain));
	ret.append(pubs);
	delete [] skey;
	return ret;
}

vc
lh_dh_pubkey_dec(vc pass, vc cipher)
{
	if(!pass.is_nil())
	{
		vc exp = vclh_sha(pass);
		const byte *b = (const byte *)(const char *)exp;
		Integer e(b, 20);
		MyDH->Setup(e, 0);
	}
	vc key = vclh_dh_agree(cipher[2]);
	key = vclh_sha(key);
	byte *bkey = new byte[10];
	const byte *b1 = (const byte *)(const char *)key;
	const byte *b2 = (const byte *)(const char *)cipher[0];
	for(int i = 0; i < 10; ++i)
		bkey[i] = b1[i] ^ b2[i];

	lh_bf_init_key(vc(VC_BSTRING, (const char *)bkey, 10));
	delete [] bkey;
	return lh_bf_dec(cipher[1]);
	
}

vc 
lh_dh_pubkey_setup(vc pass)
{
	vc exp = vclh_sha(pass);
	const byte *b = (const byte *)(const char *)exp;
	Integer e(b, 20);
	MyDH->Setup(e, 0);
	return vctrue;
}
#endif

struct bf_ctx
{

	ECB_Mode<Blowfish>::Encryption *Be;
	StreamTransformationFilter *Bef;
	ECB_Mode<Blowfish>::Decryption *Bd;
	StreamTransformationFilter *Bdf;
	CTR_Mode<Blowfish>::Encryption *Cme;
	CTR_Mode<Blowfish>::Decryption *Cmd;
	CBC_Mode<Blowfish>::Encryption *CBCe;
	CBC_Mode<Blowfish>::Decryption *CBCd;

	bf_ctx();
	~bf_ctx();

	void init_key_stream(byte *key1, int len_key1, byte *key2, int len_key2);
	void init_key_cbc(byte *key1, int len_key1, byte *iv, int len_iv);
};

bf_ctx::bf_ctx()
{
	Be = 0;
	Bef = 0;
	Bd = 0;
	Bdf = 0;
	Cme = 0;
	Cmd = 0;
	CBCe = 0;
	CBCd = 0;
}

bf_ctx::~bf_ctx()
{
	delete Be;
	delete Bef;
	delete Bd;
	delete Bdf;
	delete Cme;
	delete Cmd;
	delete CBCe;
	delete CBCd;
}

void
bf_ctx::init_key_stream(byte *key1, int len_key1, byte *key2, int len_key2)
{
	delete Be;
	delete Bef;
	delete Bd;
	delete Bdf;
	delete Cme;
	delete Cmd;
	delete CBCe;
	delete CBCd;
	Be = 0;
	Bef = 0;
	Bd = 0;
	Bdf = 0;
	Cme = 0;
	Cmd = 0;
	CBCe = 0;
	CBCd = 0;
	Be = new ECB_Mode<Blowfish>::Encryption;
	Be->SetKey(key1, len_key1);
	Cme = new CTR_Mode<Blowfish>::Encryption;
    Cme->SetKeyWithIV(key1, len_key1, (byte *)"01234567", 8);
	Bef = new StreamTransformationFilter(*Cme);
	Cmd = new CTR_Mode<Blowfish>::Decryption;
    Cmd->SetKeyWithIV(key2, len_key2, (byte *)"01234567", 8);
	Bdf = new StreamTransformationFilter(*Cmd);
}

void
bf_ctx::init_key_cbc(byte *key1, int len_key1, byte *iv, int len_iv)
{
	delete Be;
	delete Bef;
	delete Bd;
	delete Bdf;
	delete Cme;
	delete Cmd;
	delete CBCe;
	delete CBCd;
	Be = 0;
	Bef = 0;
	Bd = 0;
	Bdf = 0;
	Cme = 0;
	Cmd = 0;
	CBCe = 0;
	CBCd = 0;
	CBCe = new CBC_Mode<Blowfish>::Encryption;
    CBCe->SetKeyWithIV(key1, len_key1, iv, len_iv);
	Bef = new StreamTransformationFilter(*CBCe);
	CBCd = new CBC_Mode<Blowfish>::Decryption;
    CBCd->SetKeyWithIV(key1, len_key1, iv, len_iv);
	Bdf = new StreamTransformationFilter(*CBCd);
}

static void
bf_dtor(void *ctx)
{
	if(!ctx) return;
	bf_ctx *d = (bf_ctx *)ctx;
	delete d;
}

vc
vclh_bf_open()
{
	bf_ctx *d = new bf_ctx;
    return vc(VC_INT_DTOR, bf_dtor, d);
}

vc
vclh_bf_close(vc ctx)
{
	ctx.close();
	return vctrue;
}

vc
vclh_bf_init_key_stream_ctx(vc ctx, vc key1, vc key2)
{
    bf_ctx *b = (bf_ctx *)(void *)ctx;
	if(!b)
	{
		USER_BOMB("bad blowfish context", vcnil);
	}
	
	if(key1.len() < 10 || key2.len() < 10)
	{
		USER_BOMB("key must be at least 80 bits", vcnil);
	}
	b->init_key_stream((byte *)(const char *)key1, key1.len(), (byte *)(const char *)key2, key2.len());
	return vctrue;
}

vc
vclh_bf_init_key_cbc_ctx(vc ctx, vc key, vc iv)
{
    bf_ctx *b = (bf_ctx *)(void *)ctx;
	if(!b)
	{
		USER_BOMB("bad blowfish context", vcnil);
	}
	
	if(key.len() < 10 || iv.type() != VC_INT)
	{
		USER_BOMB("key must be at least 80 bits and iv 0 (for now)", vcnil);
	}
	byte ivb[8];
	memset(ivb, 0, sizeof(ivb));
	if(iv.type() == VC_INT)
	{
		// last half is rubbish
		*(int *)ivb = (int)iv;
	}
	b->init_key_cbc((byte *)(const char *)key, key.len(), ivb, sizeof(ivb));
	return vctrue;
}

vc
vclh_bf_enc_ctx(vc ctx, vc v)
{
    bf_ctx *b = (bf_ctx *)(void *)ctx;
	if(!b)
	{
		USER_BOMB("bad blowfish context", vcnil);
	}
    b->Bef->Initialize();
	return simple_filter(v, b->Bef);
}

vc
vclh_bf_dec_ctx(vc ctx, vc v)
{
    bf_ctx *b = (bf_ctx *)(void *)ctx;
	if(!b)
	{
		USER_BOMB("bad blowfish context", vcnil);
	}
    b->Bdf->Initialize();
	return simple_filter(v, b->Bdf);
}

vc
vclh_bf_xfer_enc_ctx(vc ctx, vc v)
{
    bf_ctx *b = (bf_ctx *)(void *)ctx;
	if(!b)
	{
		USER_BOMB("bad blowfish context", vcnil);
	}
	vcxstream vcx((char *)0, (long)8192, vcxstream::CONTINUOUS);
	long len;
	vc_composite::new_dfs();
	if(!vcx.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
		return vcnil;
	if((len = v.xfer_out(vcx)) == -1)
		return vcnil;

	vc tmp(VC_BSTRING, vcx.buf, vcx.cur - vcx.buf);
	byte iv[8];
	if(!Rng)
		init_rng();
	Rng->GenerateBlock(iv, sizeof(iv));
	b->CBCe->Resynchronize(iv);

	tmp = vclh_bf_enc_ctx(ctx, tmp);
	
	vc ret(VC_VECTOR);
	vc viv(VC_BSTRING, (char *)iv, sizeof(iv));
	ret.append(viv);
	ret.append(tmp);
	return ret;
}

vc
bf_xfer_dec_ctx(vc ctx, vc v, vc& out)
{
    bf_ctx *b = (bf_ctx *)(void *)ctx;
	if(!b)
	{
		USER_BOMB("bad blowfish context", vcnil);
	}
	// note: this isn't obvious from the cryptopp stuff, but if you
	// send a zero length string to a Put to decrypt a
	// CBC blowfish string, it corrupts the stack and dies.
	// not sure what the contraints are exactly, but we just
	// test for 0 length string here and choke for the obvious case.
	if(v.type() != VC_VECTOR || v[0].type() != VC_STRING || v[0].len() != 8 ||
		v[1].type() != VC_STRING || v[1].len() == 0)
	{
		return vcnil;
		USER_BOMB("BF-xfer-dec arg must be vector(iv, string)", vcnil);
	}
	byte iv[8];
	memcpy(iv, (const char *)v[0], sizeof(iv));
	b->CBCd->Resynchronize(iv);
	vc tmp;
	try
	{
		tmp = vclh_bf_dec_ctx(ctx, v[1]);
	}
	catch(...)
	{
		return vcnil;
	}

	vcxstream vcx((const char *)tmp, (long)tmp.len(), vcxstream::FIXED);

	vc item;
	long len;
	if(!vcx.open(vcxstream::READABLE))
		return vcnil;
	if((len = item.xfer_in(vcx)) < 0)
		return vcnil;
	out = item;
	return vctrue;
}

vc
vclh_bf_xfer_dec_ctx(vc ctx, vc v, vc out)
{
	if(out.type() != VC_STRING)
	{
		USER_BOMB("second arg to BF-xfer-dec must be a string to bind to", vcnil);
	}
	vc dec;
	if(bf_xfer_dec_ctx(ctx, v, dec).is_nil())
		return vcnil;
	Vcmap->local_add(out, dec);
	return vctrue;
}

#define ENCDEC_TAGSIZE 12
struct encdec_ctx
{
	GCM<AES>::Encryption *Be;
	GCM<AES>::Decryption *Bd;
	AuthenticatedEncryptionFilter *Bef;
	AuthenticatedDecryptionFilter *Bdf;

	encdec_ctx();
	~encdec_ctx();

	void init_key(byte *key1, int len_key1, byte *iv, int len_iv);
};

encdec_ctx::encdec_ctx()
{
	Be = 0;
	Bef = 0;
	Bd = 0;
	Bdf = 0;
}

encdec_ctx::~encdec_ctx()
{
	delete Be;
	delete Bef;
	delete Bd;
	delete Bdf;
	Be = 0;
	Bef = 0;
	Bd = 0;
	Bdf = 0;
}


void
encdec_ctx::init_key(byte *key1, int len_key1, byte *iv, int len_iv)
{
	delete Be;
	delete Bef;
	delete Bd;
	delete Bdf;
	Be = 0;
	Bef = 0;
	Bd = 0;
	Bdf = 0;
	Be = new GCM<AES>::Encryption;
	Bd = new GCM<AES>::Decryption;
	Be->SetKeyWithIV(key1, len_key1, iv, len_iv);
	Bd->SetKeyWithIV(key1, len_key1, iv, len_iv);
    // note: this was completely wrong in previous version, so
    // make sure you check with old stuff... the tagsize was being
    // sent in as flags... oops. fortunately, the only primary thing
    // that was broken was probably that it wasn't raising an
    // exception on failed verification... which doesn't happen
    // all that much.
    Bef = new AuthenticatedEncryptionFilter(*Be, 0, false, ENCDEC_TAGSIZE);
    Bdf = new AuthenticatedDecryptionFilter(*Bd, 0, AuthenticatedDecryptionFilter::DEFAULT_FLAGS, ENCDEC_TAGSIZE);
}

static void
encdec_dtor(void *ctx)
{
	if(!ctx) return;
	encdec_ctx *d = (encdec_ctx *)ctx;
	delete d;
}

vc
vclh_encdec_open()
{
	encdec_ctx *d = new encdec_ctx;
    return vc(VC_INT_DTOR, encdec_dtor, d);
}

vc
vclh_encdec_close(vc ctx)
{
	ctx.close();
	return vctrue;
}

// this is a bit goofy.
// if you give this a string iv, it is used, but you should only
// use the context with the simple functions below (not the *xfer* functions).
// basically, you are responsible for getting the iv around in this case.
//
// if you use the *xfer* functions below, the IV is done for you, so the
// iv sent in here is ignored.
// 
vc
vclh_encdec_init_key_ctx(vc ctx, vc key, vc iv)
{
    encdec_ctx *b = (encdec_ctx *)(void *)ctx;
	if(!b)
	{
		USER_BOMB("bad encdec context", vcnil);
	}
	
	if(key.len() != 16 || iv.type() != VC_INT)
	{
		USER_BOMB("key must be 16 bytes and iv 0 (for now)", vcnil);
	}
	byte ivb[12];
	memset(ivb, 0, sizeof(ivb));
	if(iv.type() == VC_INT)
	{
		// last half is rubbish
		*(int *)ivb = (int)iv;
		b->init_key((byte *)(const char *)key, key.len(), ivb, sizeof(ivb));
	}
	else if(iv.type() == VC_STRING)
	{
		b->init_key((byte *)(const char *)key, key.len(), (byte *)(const char *)iv, iv.len());
	}
	else
	{
		USER_BOMB("iv must be 0, or a string", vcnil);
	}
	return vctrue;
}

vc
vclh_encdec_enc_ctx(vc ctx, vc v)
{
    encdec_ctx *b = (encdec_ctx *)(void *)ctx;
	if(!b || !b->Bef)
	{
		return vcnil;
		USER_BOMB("bad encdec context", vcnil);
	}
    b->Bef->Initialize();
	return simple_filter(v, b->Bef);
}

vc
vclh_encdec_dec_ctx(vc ctx, vc v)
{
    encdec_ctx *b = (encdec_ctx *)(void *)ctx;
	if(!b || !b->Bdf)
	{
		return vcnil;
		USER_BOMB("bad encdec context", vcnil);
	}
    b->Bdf->Initialize();
	return simple_filter(v, b->Bdf);
}

vc
vclh_encdec_xfer_enc_ctx(vc ctx, vc v)
{
    encdec_ctx *b = (encdec_ctx *)(void *)ctx;
	if(!b || !b->Be)
	{
		return vcnil;
		USER_BOMB("bad encdec context", vcnil);
	}
	vcxstream vcx((char *)0, (long)8192, vcxstream::CONTINUOUS);
	long len;
	vc_composite::new_dfs();
	if(!vcx.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
		return vcnil;
	if((len = v.xfer_out(vcx)) == -1)
		return vcnil;

	vc tmp(VC_BSTRING, vcx.buf, vcx.cur - vcx.buf);
	byte iv[12];
	if(!Rng)
		init_rng();
	Rng->GenerateBlock(iv, sizeof(iv));
	b->Be->Resynchronize(iv);

	tmp = vclh_encdec_enc_ctx(ctx, tmp);
	
	vc ret(VC_VECTOR);
	vc viv(VC_BSTRING, (char *)iv, sizeof(iv));
	ret.append(viv);
	ret.append(tmp);
	return ret;
}

vc
encdec_xfer_dec_ctx(vc ctx, vc v, vc& out)
{
    encdec_ctx *b = (encdec_ctx *)(void *)ctx;
	if(!b || !b->Bd)
	{
		return vcnil;
		USER_BOMB("bad encdec context", vcnil);
	}
	// note: this isn't obvious from the cryptopp stuff, but if you
	// send a zero length string to a Put to decrypt a
	// CBC encdec string, it corrupts the stack and dies.
	// not sure what the contraints are exactly, but we just
	// test for 0 length string here and choke for the obvious case.
	if(v.type() != VC_VECTOR || v[0].type() != VC_STRING || v[0].len() != 12 ||
		v[1].type() != VC_STRING || v[1].len() == 0)
	{
		return vcnil;
		USER_BOMB("GCM-xfer-dec arg must be vector(iv, string)", vcnil);
	}
	byte iv[12];
	memcpy(iv, (const char *)v[0], sizeof(iv));
	b->Bd->Resynchronize(iv);
	vc tmp;
	try
	{
		tmp = vclh_encdec_dec_ctx(ctx, v[1]);
	}
	catch(...)
	{
		return vcnil;
	}

    vcxstream vcx((const char *)tmp, tmp.len(), vcxstream::FIXED);

	vc item;
	long len;
	if(!vcx.open(vcxstream::READABLE))
		return vcnil;
	if((len = item.xfer_in(vcx)) < 0)
		return vcnil;
	out = item;
	return vctrue;
}

vc
vclh_encdec_xfer_dec_ctx(vc ctx, vc v, vc out)
{
	if(out.type() != VC_STRING)
	{
		USER_BOMB("second arg to GCM-xfer-dec must be a string to bind to", vcnil);
	}
	vc dec;
	if(encdec_xfer_dec_ctx(ctx, v, dec).is_nil())
		return vcnil;
	Vcmap->local_add(out, dec);
	return vctrue;
}

#endif
