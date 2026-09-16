/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#ifndef NO_VCCRYPTO
#include "vcmap.h"
#include "vccomp.h"
#include "cryptlib.h"
#include "xed25519.h"
#include "hex.h"
#include "files.h"
#include "randpool.h"
#include "secblock.h"
#include "algparam.h"
#include "argnames.h"
#include "vcec25519.h"
#ifdef LINUX
#include <unistd.h>
#include <fcntl.h>
#endif

using namespace CryptoPP;

static RandomPool *Rng;
// singleton ed25519 state, mirrors the DSA-* machinery in vccrypt2.
// the private key object is kept explicitly because ed25519Signer
// only exposes its key through a base class reference.
static ed25519::Signer *My_ed_sign;
static ed25519::Verifier *My_ed_ver;
static ed25519PrivateKey *My_ed_key;
static ed25519PublicKey *My_ed_pub;
// singleton x25519 state
static x25519 *MyX;
static SecByteBlock MyX_priv;
static SecByteBlock MyX_pub;

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

// -*- ed25519 singleton (global key) API, mirrors DSA-* -*-

vc
vclh_ed25519_init(vc file)
{
	if(!Rng)
		init_rng();
	if(My_ed_sign)
	{
		delete My_ed_sign;
		My_ed_sign = 0;
	}
	if(My_ed_ver)
	{
		delete My_ed_ver;
		My_ed_ver = 0;
	}
	if(My_ed_key)
	{
		delete My_ed_key;
		My_ed_key = 0;
	}
	if(My_ed_pub)
	{
		delete My_ed_pub;
		My_ed_pub = 0;
	}
	if(file.is_nil())
	{
		My_ed_key = new ed25519PrivateKey;
		My_ed_key->GenerateRandom(*Rng, g_nullNameValuePairs);
		My_ed_sign = new ed25519::Signer(*My_ed_key);
	}
	else
	{
		if(file.type() != VC_STRING)
			USER_BOMB("arg must be a filename or nil", vcnil);
		FileSource fs((const char *)file, true, new HexDecoder);
		My_ed_key = new ed25519PrivateKey;
		My_ed_key->Load(fs);
		My_ed_sign = new ed25519::Signer(*My_ed_key);
	}

	My_ed_ver = new ed25519::Verifier(*My_ed_sign);
	return vcnil;
}

vc
vclh_ed25519_save(vc priv_filename, vc pub_filename)
{
	if(!My_ed_key)
	{
		USER_BOMB("ED25519 not inited", vcnil);
	}
	if(priv_filename.type() != VC_STRING || pub_filename.type() != VC_STRING)
	{
		USER_BOMB("args must be filenames", vcnil);
	}
	CryptoPP::HexEncoder he(new FileSink((const char *)priv_filename));
	My_ed_key->Save(he);

	ed25519PublicKey pub;
	My_ed_key->MakePublicKey(pub);
	CryptoPP::HexEncoder he2(new FileSink((const char *)pub_filename));
	pub.Save(he2);
	return vcnil;
}

// use this before verifying signatures with a known public key
vc
vclh_ed25519_pub_init(vc pub_filename)
{
	if(pub_filename.type() != VC_STRING)
	{
		USER_BOMB("arg must be a filename", vcnil);
	}
	if(My_ed_sign)
	{
		delete My_ed_sign;
		My_ed_sign = 0;
	}
	if(My_ed_ver)
	{
		delete My_ed_ver;
		My_ed_ver = 0;
	}
	if(My_ed_key)
	{
		delete My_ed_key;
		My_ed_key = 0;
	}
	if(My_ed_pub)
	{
		delete My_ed_pub;
		My_ed_pub = 0;
	}
	FileSource fs((const char *)pub_filename, true, new HexDecoder);
	My_ed_pub = new ed25519PublicKey;
	My_ed_pub->Load(fs);
	My_ed_ver = new ed25519::Verifier(*My_ed_pub);
	return vcnil;
}

vc
vclh_ed25519_sign(vc m)
{
	if(!My_ed_sign)
	{
		USER_BOMB("ED25519 not inited", vcnil);
	}
	if(m.type() != VC_STRING)
	{
		USER_BOMB("arg must be a string", vcnil);
	}
	byte sig[ed25519::Signer::SIGNATURE_LENGTH];
	My_ed_sign->SignMessage(NullRNG(), (const byte *)(const char *)m, m.len(), sig);
	vc ret(VC_BSTRING, (const char *)sig, (long)ed25519::Signer::SIGNATURE_LENGTH);
	return ret;
}

vc
vclh_ed25519_verify(vc m, vc sig)
{
	if(!My_ed_ver)
	{
		USER_BOMB("ED25519 not inited", vcnil);
	}
	if(m.type() != VC_STRING || sig.type() != VC_STRING)
	{
		USER_BOMB("args must be strings", vcnil);
	}
	if(sig.len() != ed25519::Verifier::SIGNATURE_LENGTH)
	{
		return vcnil;
	}
	if(My_ed_ver->VerifyMessage((const byte *)(const char *)m, m.len(), (const byte *)(const char *)sig, sig.len()))
		return vctrue;
	return vcnil;
}

// -*- ed25519 stateless API, keys are explicit 32-byte BSTRINGs -*-

vc
vclh_ed25519_gen_key(vc)
{
	if(!Rng)
		init_rng();
	ed25519PrivateKey pvt;
	pvt.GenerateRandom(*Rng, g_nullNameValuePairs);
	vc ret(VC_VECTOR);
	ret[0] = vc(VC_BSTRING, (const char *)pvt.GetPrivateKeyBytePtr(), (long)ed25519PrivateKey::SECRET_KEYLENGTH);
	ret[1] = vc(VC_BSTRING, (const char *)pvt.GetPublicKeyBytePtr(), (long)ed25519PrivateKey::PUBLIC_KEYLENGTH);
	return ret;
}

vc
vclh_ed25519_pub_from_priv(vc priv)
{
	if(priv.type() != VC_STRING || priv.len() != ed25519PrivateKey::SECRET_KEYLENGTH)
	{
		USER_BOMB("arg must be a 32 byte private key", vcnil);
	}
	ed25519PrivateKey pk;
	pk.SetPrivateExponent((const byte *)(const char *)priv);
	ed25519PublicKey pub;
	pk.MakePublicKey(pub);
	vc ret(VC_BSTRING, (const char *)pub.GetPublicKeyBytePtr(), (long)ed25519PublicKey::PUBLIC_KEYLENGTH);
	return ret;
}

vc
vclh_ed25519_sign_key(vc m, vc priv)
{
	if(m.type() != VC_STRING)
	{
		USER_BOMB("arg must be a string", vcnil);
	}
	if(priv.type() != VC_STRING || priv.len() != ed25519::Signer::SECRET_KEYLENGTH)
	{
		USER_BOMB("priv key must be 32 bytes", vcnil);
	}
	ed25519::Signer signer((const byte *)(const char *)priv);
	byte sig[ed25519::Signer::SIGNATURE_LENGTH];
	signer.SignMessage(NullRNG(), (const byte *)(const char *)m, m.len(), sig);
	vc ret(VC_BSTRING, (const char *)sig, (long)ed25519::Signer::SIGNATURE_LENGTH);
	return ret;
}

vc
vclh_ed25519_verify_key(vc m, vc sig, vc pub)
{
	if(m.type() != VC_STRING || sig.type() != VC_STRING)
	{
		USER_BOMB("args must be strings", vcnil);
	}
	if(pub.type() != VC_STRING || pub.len() != ed25519::Verifier::PUBLIC_KEYLENGTH)
	{
		USER_BOMB("pub key must be 32 bytes", vcnil);
	}
	if(sig.len() != ed25519::Verifier::SIGNATURE_LENGTH)
	{
		return vcnil;
	}
	ed25519::Verifier verifier((const byte *)(const char *)pub);
	if(verifier.VerifyMessage((const byte *)(const char *)m, m.len(), (const byte *)(const char *)sig, sig.len()))
		return vctrue;
	return vcnil;
}

// -*- x25519 singleton (global key) API, mirrors DH-* -*-

vc
vclh_x25519_init(vc)
{
	if(MyX)
	{
		delete MyX;
		MyX = 0;
	}
	if(!Rng)
		init_rng();
	MyX = new x25519;
	MyX_priv.resize(MyX->PrivateKeyLength());
	MyX_pub.resize(MyX->PublicKeyLength());
	return vctrue;
}

vc
vclh_x25519_keygen()
{
	if(!MyX)
	{
		USER_BOMB("X25519 not inited", vcnil);
	}
	if(!Rng)
		init_rng();
	MyX->GeneratePrivateKey(*Rng, MyX_priv.BytePtr());
	MyX->GeneratePublicKey(*Rng, MyX_priv.BytePtr(), MyX_pub.BytePtr());
	vc ret(VC_BSTRING, (const char *)MyX_pub.BytePtr(), (long)MyX_pub.SizeInBytes());
	return ret;
}

vc
vclh_x25519_agree(vc other_public)
{
	if(!MyX)
	{
		USER_BOMB("X25519 not inited", vcnil);
	}
	if(other_public.type() != VC_STRING || other_public.len() != MyX->PublicKeyLength())
	{
		USER_BOMB("arg must be a 32 byte public key", vcnil);
	}
	SecByteBlock shared(MyX->AgreedValueLength());
	if(!MyX->Agree(shared.BytePtr(), MyX_priv.BytePtr(), (const byte *)(const char *)other_public))
	{
		USER_BOMB("x25519 agree failed", vcnil);
	}
	vc ret(VC_BSTRING, (const char *)shared.BytePtr(), (long)shared.SizeInBytes());
	return ret;
}

vc
vclh_x25519_save(vc filename)
{
	if(MyX_priv.empty())
	{
		USER_BOMB("no x25519 key generated", vcnil);
	}
	if(filename.type() != VC_STRING)
	{
		USER_BOMB("first arg must be a filename", vcnil);
	}
	x25519 x(MyX_priv.BytePtr());
	CryptoPP::HexEncoder he(new FileSink((const char *)filename));
	x.Save(he);
	return vcnil;
}

vc
vclh_x25519_load(vc filename)
{
	if(filename.type() != VC_STRING)
	{
		USER_BOMB("first arg must be a filename", vcnil);
	}
	if(MyX)
	{
		delete MyX;
		MyX = 0;
	}
	if(!Rng)
		init_rng();
	FileSource fs((const char *)filename, true, new HexDecoder);
	MyX = new x25519;
	MyX->Load(fs);
	ConstByteArrayParameter pk;
	if(!MyX->GetVoidValue(Name::PrivateExponent(), typeid(ConstByteArrayParameter), &pk))
	{
		USER_BOMB("no private key in file", vcnil);
	}
	MyX_priv.resize(MyX->PrivateKeyLength());
	memcpy(MyX_priv.BytePtr(), pk.begin(), MyX_priv.SizeInBytes());
	MyX_pub.resize(MyX->PublicKeyLength());
	MyX->GeneratePublicKey(*Rng, MyX_priv.BytePtr(), MyX_pub.BytePtr());
	return vctrue;
}

// -*- x25519 stateless API, keys are explicit 32-byte BSTRINGs -*-

vc
vclh_x25519_gen_key(vc)
{
	if(!Rng)
		init_rng();
	x25519 x;
	SecByteBlock priv(x.PrivateKeyLength());
	SecByteBlock pub(x.PublicKeyLength());
	x.GeneratePrivateKey(*Rng, priv.BytePtr());
	x.GeneratePublicKey(*Rng, priv.BytePtr(), pub.BytePtr());
	vc ret(VC_VECTOR);
	ret[0] = vc(VC_BSTRING, (const char *)priv.BytePtr(), (long)priv.SizeInBytes());
	ret[1] = vc(VC_BSTRING, (const char *)pub.BytePtr(), (long)pub.SizeInBytes());
	return ret;
}

vc
vclh_x25519_pub_from_priv(vc priv)
{
	if(priv.type() != VC_STRING || priv.len() != (long)x25519().PublicKeyLength())
	{
		USER_BOMB("arg must be a 32 byte private key", vcnil);
	}
	if(!Rng)
		init_rng();
	x25519 x;
	SecByteBlock pub(x.PublicKeyLength());
	x.GeneratePublicKey(*Rng, (const byte *)(const char *)priv, pub.BytePtr());
	vc ret(VC_BSTRING, (const char *)pub.BytePtr(), (long)pub.SizeInBytes());
	return ret;
}

vc
vclh_x25519_agree_key(vc our_priv, vc other_public)
{
	if(our_priv.type() != VC_STRING || our_priv.len() != (long)x25519().PrivateKeyLength())
	{
		USER_BOMB("priv key must be 32 bytes", vcnil);
	}
	if(other_public.type() != VC_STRING || other_public.len() != (long)x25519().PublicKeyLength())
	{
		USER_BOMB("pub key must be 32 bytes", vcnil);
	}
	x25519 x;
	SecByteBlock shared(x.AgreedValueLength());
	if(!x.Agree(shared.BytePtr(), (const byte *)(const char *)our_priv, (const byte *)(const char *)other_public))
	{
		USER_BOMB("x25519 agree failed", vcnil);
	}
	vc ret(VC_BSTRING, (const char *)shared.BytePtr(), (long)shared.SizeInBytes());
	return ret;
}

#endif