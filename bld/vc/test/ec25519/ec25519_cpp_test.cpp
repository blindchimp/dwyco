// EC25519 C++ direct-call test.
//
// Exercises the vclh_ed25519_* / vclh_x25519_* functions exactly as a
// C++ client of libvc.a would (same entry points the LH interpreter uses),
// plus RFC 8032 / RFC 7748 known-answer vectors.
//
// Build it with run_tests.sh, or by hand against the shadow build tree
// produced by vccmd/build.sh (see run_tests.sh for the link line).

#include "vcec25519.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <functional>
#include <iostream>

// declared in vcmap.h; when set, USER_BOMB raises (int) instead of
// printing a runtime error and returning
extern int Throw_user_panic;

[[noreturn]]
void
oopanic(const char *s)
{
	printf("panic: %s\n", s);
	fflush(stdout);
	exit(1);
}

// run a callable that is expected to fail; returns true if it raised
// a USER_BOMB (int) or any C++ exception, i.e. a graceful, non-crashing
// failure of the underlying builtin
static bool fails_gracefully(const std::function<void()>& fn)
{
	try {
		fn();
	} catch(int) {
		return true;
	} catch(const std::exception&) {
		return true;
	} catch(...) {
		return true;
	}
	return false;
}

static int fails = 0;
static void check(bool cond, const char *msg)
{
	if(!cond) {
		std::cout << "FAIL: " << msg << "\n";
		++fails;
	} else {
		std::cout << "ok: " << msg << "\n";
	}
}

static std::string to_str(const vc& v)
{
	return std::string((const char*)v, (size_t)v.len());
}

static int hexval(char c)
{
	if(c >= '0' && c <= '9') return c - '0';
	if(c >= 'a' && c <= 'f') return c - 'a' + 10;
	if(c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

// hex string -> NUL-free byte string (stored in std::string)
static std::string from_hex(const char *hex)
{
	std::string out;
	size_t n = strlen(hex);
	for(size_t i = 0; i + 1 < n; i += 2) {
		int hi = hexval(hex[i]);
		int lo = hexval(hex[i + 1]);
		if(hi < 0 || lo < 0) {
			printf("bad hex digit in test vector near %s\n", hex + i);
			exit(1);
		}
		out.push_back((char)((hi << 4) | lo));
	}
	return out;
}

static vc to_vc_bytes(const std::string& s)
{
	return vc(VC_BSTRING, s.data(), (long)s.size());
}

// RFC 8032 7.1 TEST 1024 (1023-byte message)
static const char *KAT1024_SEC =
	"f5e5767cf153319517630f226876b86c8160cc583bc013744c6bf255f5cc0ee5";
static const char *KAT1024_PUB =
	"278117fc144c72340f67d0f2316e8386ceffbf2b2428c9c51fef7c597f1d426e";
static const char *KAT1024_MSG =
	"08b8b2b733424243760fe426a4b54908632110a66c2f6591eabd3345e3e4eb98"
	"fa6e264bf09efe12ee50f8f54e9f77b1e355f6c50544e23fb1433ddf73be84d8"
	"79de7c0046dc4996d9e773f4bc9efe5738829adb26c81b37c93a1b270b20329d"
	"658675fc6ea534e0810a4432826bf58c941efb65d57a338bbd2e26640f89ffbc"
	"1a858efcb8550ee3a5e1998bd177e93a7363c344fe6b199ee5d02e82d522c4fe"
	"ba15452f80288a821a579116ec6dad2b3b310da903401aa62100ab5d1a36553e"
	"06203b33890cc9b832f79ef80560ccb9a39ce767967ed628c6ad573cb116dbef"
	"efd75499da96bd68a8a97b928a8bbc103b6621fcde2beca1231d206be6cd9ec7"
	"aff6f6c94fcd7204ed3455c68c83f4a41da4af2b74ef5c53f1d8ac70bdcb7ed1"
	"85ce81bd84359d44254d95629e9855a94a7c1958d1f8ada5d0532ed8a5aa3fb2"
	"d17ba70eb6248e594e1a2297acbbb39d502f1a8c6eb6f1ce22b3de1a1f40cc24"
	"554119a831a9aad6079cad88425de6bde1a9187ebb6092cf67bf2b13fd65f270"
	"88d78b7e883c8759d2c4f5c65adb7553878ad575f9fad878e80a0c9ba63bcbcc"
	"2732e69485bbc9c90bfbd62481d9089beccf80cfe2df16a2cf65bd92dd597b07"
	"07e0917af48bbb75fed413d238f5555a7a569d80c3414a8d0859dc65a46128ba"
	"b27af87a71314f318c782b23ebfe808b82b0ce26401d2e22f04d83d1255dc51a"
	"ddd3b75a2b1ae0784504df543af8969be3ea7082ff7fc9888c144da2af58429e"
	"c96031dbcad3dad9af0dcbaaaf268cb8fcffead94f3c7ca495e056a9b47acdb7"
	"51fb73e666c6c655ade8297297d07ad1ba5e43f1bca32301651339e22904cc8c"
	"42f58c30c04aafdb038dda0847dd988dcda6f3bfd15c4b4c4525004aa06eeff8"
	"ca61783aacec57fb3d1f92b0fe2fd1a85f6724517b65e614ad6808d6f6ee34df"
	"f7310fdc82aebfd904b01e1dc54b2927094b2db68d6f903b68401adebf5a7e08"
	"d78ff4ef5d63653a65040cf9bfd4aca7984a74d37145986780fc0b16ac451649"
	"de6188a7dbdf191f64b5fc5e2ab47b57f7f7276cd419c17a3ca8e1b939ae49e4"
	"88acba6b965610b5480109c8b17b80e1b7b750dfc7598d5d5011fd2dcc5600a3"
	"2ef5b52a1ecc820e308aa342721aac0943bf6686b64b2579376504ccc493d97e"
	"6aed3fb0f9cd71a43dd497f01f17c0e2cb3797aa2a2f256656168e6c496afc5f"
	"b93246f6b1116398a346f1a641f3b041e989f7914f90cc2c7fff357876e506b5"
	"0d334ba77c225bc307ba537152f3f1610e4eafe595f6d9d90d11faa933a15ef1"
	"369546868a7f3a45a96768d40fd9d03412c091c6315cf4fde7cb68606937380d"
	"b2eaaa707b4c4185c32eddcdd306705e4dc1ffc872eeee475a64dfac86aba41c"
	"0618983f8741c5ef68d3a101e8a3b8cac60c905c15fc910840b94c00a0b9d0";
static const char *KAT1024_SIG =
	"0aab4c900501b3e24d7cdf4663326a3a87df5e4843b2cbdb67cbf6e460fec350"
	"aa5371b1508f9f4528ecea23c436d94b5e8fcd4f681e30a6ac00a9704a188a03";

// RFC 7748 6.1
static const char *X_APRIV =
	"77076d0a7318a57d3c16c17251b26645df4c2f87ebc0992ab177fba51db92c2a";
static const char *X_APUB =
	"8520f0098930a754748b7ddcb43ef75a0dbf3a0d26381af4eba4a98eaa9b4e6a";
static const char *X_BPRIV =
	"5dab087e624a8a4b79e17f8b83800ee66f3bb1292618b6fd1c2f8b27ff88e0eb";
static const char *X_BPUB =
	"de9edb7d7b7dc1b4d35b61c2ece435373f8343c85b78674dadfc7e146f882b4f";
static const char *X_SHARED =
	"4a5d9d5ba4ce2de1728e3bf480350f25e07e21c947d19e3376f09b3c1e161742";

int main()
{
	// ---- negative-path tests first: the process-global singletons are
	// not yet initialized here, so "before init" cases are real. Bogus
	// args and bogus files must fail gracefully through USER_BOMB /
	// exceptions (never crash) ----
	Throw_user_panic = 1;
	{
	vc ed_sec = to_vc_bytes(from_hex("9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60"));
	std::string ed1_pub = from_hex("d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a");
	std::string ed1_sig = from_hex("e5564300c360ac729086e2cc806e828a84877f1eb8e5d974d873e065224901555fb8821590a33bacc61e39701cf9b46bd25bf5f0595bbe24655141438e7a100b");
	vc x_bpriv = to_vc_bytes(from_hex(X_BPRIV));
	vc x_bpub = to_vc_bytes(from_hex(X_BPUB));

	FILE *fg = fopen("/tmp/ec25519_neg_garbage.hex", "wb");
	fputs("garbage!! this is @# not hex", fg);
	fclose(fg);
	FILE *fh = fopen("/tmp/ec25519_neg_hexish.hex", "wb");
	fputs("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", fh);
	fclose(fh);

	// ed25519 singleton: not inited / bad init inputs
	check(fails_gracefully([&]{ vclh_ed25519_sign(vc("m")); }), "neg: ed25519 sign before init");
	check(fails_gracefully([&]{ vclh_ed25519_verify(vc("m"), vc("s")); }), "neg: ed25519 verify before init");
	check(fails_gracefully([&]{ vclh_ed25519_save(vc("/tmp/x"), vc("/tmp/y")); }), "neg: ed25519 save before init");
	check(fails_gracefully([&]{ vclh_ed25519_init(vc(5)); }), "neg: ed25519 init non-string arg");
	check(fails_gracefully([&]{ vclh_ed25519_init(vc("/tmp/ec25519_nonexistent_zz.hex")); }), "neg: ed25519 init missing key file");
	check(fails_gracefully([&]{ vclh_ed25519_init(vc("/tmp/ec25519_neg_garbage.hex")); }), "neg: ed25519 init garbage key file");
	check(fails_gracefully([&]{ vclh_ed25519_init(vc("/tmp/ec25519_neg_hexish.hex")); }), "neg: ed25519 init non-key hex file");

	// ed25519 singleton: bad args after a fresh init
	check(vclh_ed25519_init(vcnil).is_nil(), "neg reset: ed25519 init nil");
	check(fails_gracefully([&]{ vclh_ed25519_sign(vc(5)); }), "neg: ed25519 sign non-string arg");
	check(vcnil == vclh_ed25519_verify(vc("m"), vc("short")), "neg: ed25519 verify short sig rejected");
	check(fails_gracefully([&]{ vclh_ed25519_verify(vc(5), vc("s")); }), "neg: ed25519 verify non-string args");
	check(fails_gracefully([&]{ vclh_ed25519_save(vc(5), vc(5)); }), "neg: ed25519 save non-string filenames");
	check(fails_gracefully([&]{ vclh_ed25519_save(vc("/nonexistent_dir_zz/ed.hex"), vc("/nonexistent_dir_zz/edpub.hex")); }), "neg: ed25519 save unwritable path");

	// ed25519 init-pub: bad args and bad files
	check(fails_gracefully([&]{ vclh_ed25519_pub_init(vc(5)); }), "neg: ed25519 init-pub non-string arg");
	check(fails_gracefully([&]{ vclh_ed25519_pub_init(vc("/tmp/ec25519_nonexistent_zz.hex")); }), "neg: ed25519 init-pub missing file");
	check(fails_gracefully([&]{ vclh_ed25519_pub_init(vc("/tmp/ec25519_neg_garbage.hex")); }), "neg: ed25519 init-pub garbage file");

	// ed25519 stateless: bad args
	check(fails_gracefully([&]{ vclh_ed25519_pub_from_priv(vc("x")); }), "neg: ed25519 pub-from-priv short key");
	check(fails_gracefully([&]{ vclh_ed25519_pub_from_priv(vc(5)); }), "neg: ed25519 pub-from-priv non-string");
	check(fails_gracefully([&]{ vclh_ed25519_sign_key(vc(5), ed_sec); }), "neg: ed25519 sign-key non-string msg");
	check(fails_gracefully([&]{ vclh_ed25519_sign_key(vc("x"), vc("short")); }), "neg: ed25519 sign-key short priv");
	check(fails_gracefully([&]{ vclh_ed25519_verify_key(vc(5), to_vc_bytes(ed1_sig), to_vc_bytes(ed1_pub)); }), "neg: ed25519 verify-key non-string msg");
	check(fails_gracefully([&]{ vclh_ed25519_verify_key(vc("x"), to_vc_bytes(ed1_sig), vc("short")); }), "neg: ed25519 verify-key short pub");
	check(vcnil == vclh_ed25519_verify_key(vc("m"), vc("short"), to_vc_bytes(ed1_pub)), "neg: ed25519 verify-key short sig rejected");

	// x25519 singleton: bad args and bad files
	check(fails_gracefully([&]{ vclh_x25519_keygen(); }), "neg: x25519 keygen before init");
	check(fails_gracefully([&]{ vclh_x25519_agree(x_bpub); }), "neg: x25519 agree before init");
	check(fails_gracefully([&]{ vclh_x25519_save(vc("/tmp/ec25519_neg_before.hex")); }), "neg: x25519 save before any init");
	check(vctrue == vclh_x25519_init(vcnil), "neg setup: x25519 init");
	check(fails_gracefully([&]{ vclh_x25519_agree(vc("short")); }), "neg: x25519 agree short pub");
	check(fails_gracefully([&]{ vclh_x25519_agree(vc(5)); }), "neg: x25519 agree non-string pub");
	(void)vclh_x25519_keygen();
	check(fails_gracefully([&]{ vclh_x25519_save(vc(5)); }), "neg: x25519 save non-string filename");
	check(fails_gracefully([&]{ vclh_x25519_save(vc("/nonexistent_dir_zz/x.hex")); }), "neg: x25519 save unwritable path");
	check(vclh_x25519_save(vc("/tmp/ec25519_neg_xkey.hex")).is_nil(), "neg setup: x25519 save valid");
	check(fails_gracefully([&]{ vclh_x25519_load(vc(5)); }), "neg: x25519 load non-string filename");
	check(fails_gracefully([&]{ vclh_x25519_load(vc("/tmp/ec25519_nonexistent_zz.hex")); }), "neg: x25519 load missing file");
	check(fails_gracefully([&]{ vclh_x25519_load(vc("/tmp/ec25519_neg_garbage.hex")); }), "neg: x25519 load garbage file");
	check(fails_gracefully([&]{ vclh_x25519_load(vc("/tmp/ec25519_neg_hexish.hex")); }), "neg: x25519 load non-key hex file");

	// x25519 stateless: bad args
	check(fails_gracefully([&]{ vclh_x25519_pub_from_priv(vc("short")); }), "neg: x25519 pub-from-priv short key");
	check(fails_gracefully([&]{ vclh_x25519_agree_key(vc("short"), x_bpub); }), "neg: x25519 agree-key short priv");
	check(fails_gracefully([&]{ vclh_x25519_agree_key(x_bpriv, vc("short")); }), "neg: x25519 agree-key short pub");

	(void)remove("/tmp/ec25519_neg_garbage.hex");
	(void)remove("/tmp/ec25519_neg_hexish.hex");
	(void)remove("/tmp/ec25519_neg_xkey.hex");
	(void)remove("/tmp/ec25519_neg_before.hex");
	}

	// ---- RFC 8032 ed25519 KATs ----
	vc ed_sec = to_vc_bytes(from_hex("9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60"));
	std::string ed1_pub = from_hex("d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a");
	std::string ed1_sig = from_hex("e5564300c360ac729086e2cc806e828a84877f1eb8e5d974d873e065224901555fb8821590a33bacc61e39701cf9b46bd25bf5f0595bbe24655141438e7a100b");

	check(to_str(vclh_ed25519_pub_from_priv(ed_sec)) == ed1_pub, "ed25519 RFC8032 T1 pub-from-priv");
	check(to_str(vclh_ed25519_sign_key(vc(""), ed_sec)) == ed1_sig, "ed25519 RFC8032 T1 sign empty msg");
	check(vctrue == vclh_ed25519_verify_key(vc(""), to_vc_bytes(ed1_sig), to_vc_bytes(ed1_pub)), "ed25519 RFC8032 T1 verify");

	vc ed2_sec = to_vc_bytes(from_hex("4ccd089b28ff96da9db6c346ec114e0f5b8a319f35aba624da8cf6ed4fb8a6fb"));
	std::string ed2_pub = from_hex("3d4017c3e843895a92b70aa74d1b7ebc9c982ccf2ec4968cc0cd55f12af4660c");
	std::string ed2_sig = from_hex("92a009a9f0d4cab8720e820b5f642540a2b27b5416503f8fb3762223ebdb69da085ac1e43e15996e458f3613d0f11d8c387b2eaeb4302aeeb00d291612bb0c00");
	std::string ed2_msg = from_hex("72");

	check(to_str(vclh_ed25519_pub_from_priv(ed2_sec)) == ed2_pub, "ed25519 RFC8032 T2 pub-from-priv");
	check(to_str(vclh_ed25519_sign_key(to_vc_bytes(ed2_msg), ed2_sec)) == ed2_sig, "ed25519 RFC8032 T2 sign 0x72");
	check(vctrue == vclh_ed25519_verify_key(to_vc_bytes(ed2_msg), to_vc_bytes(ed2_sig), to_vc_bytes(ed2_pub)), "ed25519 RFC8032 T2 verify");

	vc ed1024_sec = to_vc_bytes(from_hex(KAT1024_SEC));
	std::string ed1024_pub = from_hex(KAT1024_PUB);
	std::string ed1024_msg = from_hex(KAT1024_MSG);
	std::string ed1024_sig = from_hex(KAT1024_SIG);
	check(ed1024_msg.size() == 1023, "ed25519 RFC8032 T1024 msg length 1023");
	check(to_str(vclh_ed25519_pub_from_priv(ed1024_sec)) == ed1024_pub, "ed25519 RFC8032 T1024 pub-from-priv");
	check(to_str(vclh_ed25519_sign_key(to_vc_bytes(ed1024_msg), ed1024_sec)) == ed1024_sig, "ed25519 RFC8032 T1024 sign 1023-byte msg");
	check(vctrue == vclh_ed25519_verify_key(to_vc_bytes(ed1024_msg), to_vc_bytes(ed1024_sig), to_vc_bytes(ed1024_pub)), "ed25519 RFC8032 T1024 verify");

	// ---- ed25519 stateless functional ----
	vc k = vclh_ed25519_gen_key(vcnil);
	vc priv = k[(long)0];
	vc pub = k[(long)1];
	check(priv.len() == 32 && pub.len() == 32, "ed25519 gen_key 32-byte priv/pub");
	check(to_str(pub) == to_str(vclh_ed25519_pub_from_priv(priv)), "ed25519 pub-from-priv consistency");

	vc msg = vc("hello from C++");
	vc sig = vclh_ed25519_sign_key(msg, priv);
	check(sig.len() == 64, "ed25519 signature length 64");
	check(vctrue == vclh_ed25519_verify_key(msg, sig, pub), "ed25519 sign/verify roundtrip");
	check(vcnil == vclh_ed25519_verify_key(vc("tampered"), sig, pub), "ed25519 tampered msg rejected");
	check(vctrue == vclh_ed25519_verify_key(msg, vclh_ed25519_sign_key(msg, priv), pub), "ed25519 deterministic signature");

	// ---- ed25519 singleton save/init-pub ----
	check(vclh_ed25519_init(vcnil).is_nil(), "ed25519 init");
	vc s1 = vclh_ed25519_sign(vc("singleton msg"));
	check(vclh_ed25519_save(vc("/tmp/ec25519_cpp_edpriv.hex"), vc("/tmp/ec25519_cpp_edpub.hex")).is_nil(), "ed25519 save");
	check(vclh_ed25519_init(vc("/tmp/ec25519_cpp_edpriv.hex")).is_nil(), "ed25519 init from saved priv key file");
	check(vctrue == vclh_ed25519_verify(vc("singleton msg"), s1), "ed25519 verify after init from key file");
	vc s2 = vclh_ed25519_sign(vc("after reload from file"));
	check(vctrue == vclh_ed25519_verify(vc("after reload from file"), s2), "ed25519 sign/verify after init from key file");
	check(vclh_ed25519_pub_init(vc("/tmp/ec25519_cpp_edpub.hex")).is_nil(), "ed25519 init-pub");
	check(vctrue == vclh_ed25519_verify(vc("singleton msg"), s1), "ed25519 singleton sign then verify via saved pub");
	check(vclh_ed25519_pub_init(vc("/tmp/ec25519_cpp_edpub.hex")).is_nil(), "ed25519 re-init-pub replaces prior pub");
	check(vclh_ed25519_init(vcnil).is_nil(), "ed25519 re-init after init-pub discards pub");
	vc s3 = vclh_ed25519_sign(vc("fresh key after re-init"));
	check(vctrue == vclh_ed25519_verify(vc("fresh key after re-init"), s3), "ed25519 sign/verify after re-init");

	// ---- RFC 7748 x25519 DH KAT ----
	vc x_apriv = to_vc_bytes(from_hex(X_APRIV));
	vc x_bpriv = to_vc_bytes(from_hex(X_BPRIV));
	vc x_bpub = to_vc_bytes(from_hex(X_BPUB));
	check(to_str(vclh_x25519_pub_from_priv(x_apriv)) == from_hex(X_APUB), "x25519 RFC7748 Alice pub-from-priv");
	check(to_str(vclh_x25519_pub_from_priv(x_bpriv)) == from_hex(X_BPUB), "x25519 RFC7748 Bob pub-from-priv");
	check(to_str(vclh_x25519_agree_key(x_apriv, x_bpub)) == from_hex(X_SHARED), "x25519 RFC7748 shared secret");
	check(to_str(vclh_x25519_agree_key(x_bpriv, to_vc_bytes(from_hex(X_APUB)))) == from_hex(X_SHARED), "x25519 RFC7748 shared secret (other direction)");

	// ---- x25519 stateless functional ----
	vc ka = vclh_x25519_gen_key(vcnil);
	vc privA = ka[(long)0];
	vc pubA = ka[(long)1];
	vc kb = vclh_x25519_gen_key(vcnil);
	vc privB = kb[(long)0];
	vc pubB = kb[(long)1];
	check(privA.len() == 32 && pubA.len() == 32, "x25519 gen_key 32-byte keys");
	const char *pr = (const char*)privA;
	check(((pr[0] & 0x07) == 0) && ((pr[31] & 0x80) == 0) && ((pr[31] & 0x40) != 0), "x25519 generated private scalar is clamped");
	check(to_str(pubA) == to_str(vclh_x25519_pub_from_priv(privA)), "x25519 pub-from-priv consistency");
	vc sAB = vclh_x25519_agree_key(privA, pubB);
	vc sBA = vclh_x25519_agree_key(privB, pubA);
	check(sAB.len() == 32 && to_str(sAB) == to_str(sBA), "x25519 two-party agreement identical");

	// ---- x25519 singleton save/load/agree ----
	check(vctrue == vclh_x25519_init(vcnil), "x25519 init");
	vc pubS = vclh_x25519_keygen();
	check(pubS.len() == 32, "x25519 keygen pub len 32");
	check(vclh_x25519_save(vc("/tmp/ec25519_cpp_xkey.hex")).is_nil(), "x25519 save");
	check(vctrue == vclh_x25519_init(vcnil), "x25519 re-init");
	check(vctrue == vclh_x25519_load(vc("/tmp/ec25519_cpp_xkey.hex")), "x25519 load returns true");
	check(to_str(vclh_x25519_agree(pubB)) == to_str(vclh_x25519_agree_key(privB, pubS)), "x25519 singleton agrees with stateless party");

	(void)remove("/tmp/ec25519_cpp_edpriv.hex");
	(void)remove("/tmp/ec25519_cpp_edpub.hex");
	(void)remove("/tmp/ec25519_cpp_xkey.hex");

	if(fails == 0)
		std::cout << "ALL EC25519 C++ TESTS PASSED\n";
	return fails ? 1 : 0;
}