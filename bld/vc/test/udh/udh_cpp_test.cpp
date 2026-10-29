// UDH (Unified DH) C++ direct-call test.
//
// Exercises the udh_* / dh_store_and_forward_* entry points of vcudh.cpp
// exactly as a C++ client of libvc.a would, which is also how the LH
// interpreter reaches them (the seven UDH-* builtins registered at
// vclh.cpp:3529-3539 are thin wrappers around these).
//
// Three of the vcudh.cpp entry points -- udh_public_from_private,
// dh_store_and_forward_material2 and dh_store_and_forward_get_key2 -- have
// NO LH builtin at all and are only reachable from C++ (cdc32), so this file
// is not optional coverage; it is the only way to reach that code.
//
// Structure:
//   1. crash group   -- uninitialized entry points must die by signal, they
//                       must not quietly return a value
//   2. init branches -- udh_init / init_rng entropy and re-init paths
//   3. happy paths   -- keygen, agreement, store-and-forward, get-key2
//   4. KAT           -- captured known-answer vectors (--gen-kat)
//   5. negatives     -- argument validation on every early-return branch
//
// Build it with run_tests.sh, or by hand against the shadow build tree
// produced by vccmd/build.sh (see run_tests.sh for the link line).

#include "vcudh.h"
#include "vcmap.h"
#include "vcctx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

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

static int fails = 0;

static void check(bool cond, const std::string& msg)
{
	if(!cond) {
		std::cout << "FAIL: " << msg << "\n";
		++fails;
	} else {
		std::cout << "ok: " << msg << "\n";
	}
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

// vcudh.cpp has no initialization guards, so an entry point reached before
// udh_init() dereferences a null EphDH/UDH/Rng. This asserts the crash
// actually happens instead of a bogus value coming back.
//
// The call runs in a forked child so one segfault cannot take down the rest
// of the suite. Note that a child killed by a signal never runs its atexit
// handlers, so it contributes no gcov data (and cannot corrupt the parent's
// counters) -- this group is a behaviour check, not a coverage source.
//
// This deliberately pins crash behaviour. If someone later adds a USER_BOMB
// guard to these functions, these tests go red on purpose: the child would
// then return normally and needs a decision, not a silent flip.
static void check_crash(const char *name, void (*fn)(void))
{
	fflush(stdout);
	fflush(stderr);
	pid_t pid = fork();
	if(pid < 0) {
		check(false, std::string("crash: ") + name + " (fork failed)");
		return;
	}
	if(pid == 0) {
		// make sure the null deref surfaces as a raw signal, not a
		// swallowed/handled fault
		signal(SIGSEGV, SIG_DFL);
		signal(SIGBUS, SIG_DFL);
		signal(SIGILL, SIG_DFL);
		try {
			fn();
		} catch(...) {
			_exit(0);
		}
		_exit(0);
	}
	int status = 0;
	if(waitpid(pid, &status, 0) < 0) {
		check(false, std::string("crash: ") + name + " (waitpid failed)");
		return;
	}
	if(WIFSIGNALED(status)) {
		std::cout << "ok: crash: " << name << " (signal " << WTERMSIG(status) << ")\n";
		return;
	}
	// anything other than a signal death is the failure mode we care
	// about: the call came back instead of crashing
	std::string why;
	if(WIFEXITED(status))
		why = "returned normally, exit code " + std::to_string(WEXITSTATUS(status));
	else
		why = "odd wait status " + std::to_string(status);
	check(false, std::string("crash: ") + name + " (did NOT crash: " + why + ")");
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

static std::string to_hex(const std::string& s)
{
	static const char *d = "0123456789abcdef";
	std::string out;
	out.reserve(s.size() * 2);
	for(size_t i = 0; i < s.size(); ++i) {
		unsigned char c = (unsigned char)s[i];
		out.push_back(d[c >> 4]);
		out.push_back(d[c & 0xf]);
	}
	return out;
}

static vc to_vc_bytes(const std::string& s)
{
	return vc(VC_BSTRING, s.data(), (long)s.size());
}

// the group baked into vcudh.cpp is a 2048-bit safe prime with a 2048-bit
// subgroup order, so every key is 256 bytes and a DH2 agreed value (two
// concatenated agreements) is 512. The SHA1 key-derivation digest is 20,
// and the store-and-forward session key is 16.
static const size_t DH_KEYLEN = 256;
static const size_t DH_AGREEDLEN = 512;
static const size_t KDK_LEN = 20;
static const size_t SKEY_LEN = 16;

// a correctly sized but numerically invalid DH public key. Anything at or
// above the group prime makes Crypto++ throw DL_BadElement, which
// DL_SimpleKeyAgreementDomainBase::Agree turns into a plain "return false"
// (pubkey.h). This is how the !Agree branches get hit without tripping the
// out-of-bounds read that a SHORT key would cause.
static std::string invalid_pub()
{
	return std::string(DH_KEYLEN, (char)0xFF);
}

// build a well-formed 2-element store-and-forward pack whose recipient
// public key is deliberately invalid, so get_key reaches the !Agree branch
static vc make_bad_pub_pack()
{
	vc pack(VC_VECTOR);
	pack[0] = to_vc_bytes(std::string(SKEY_LEN, (char)0x00));
	pack[1] = to_vc_bytes(invalid_pub());
	return pack;
}

// ---------------------------------------------------------------------------
// known-answer vectors
//
// Captured once with "udh_cpp_test --gen-kat" and pasted in. They pin the
// in-source 2048-bit group, the sha() key derivation, the session-key XOR and
// (via the material2 vector) the 3-byte AES-ECB key check string. Regenerate
// them if the group parameters in udh_init() ever change.
// ---------------------------------------------------------------------------

static const char *KAT_PRIV =
	"0000000000000000000000000000000000000000000000000000000000000000"
	"0000000000000000000000000000000000000000000000000000000000000000"
	"0000000000000000000000000000000000000000000000000000000000000000"
	"0000000000000000000000000000000000000000000000000000000000000000"
	"0000000000000000000000000000000000000000000000000000000000000000"
	"0000000000000000000000000000000000000000000000000000000000000000"
	"0000000000000000000000000000000000000000000000000000000000000000"
	"00000000d04d4f3e647ab2f0e23ed7c1693867ed0b7481ad2778f52d0f74548d";
static const char *KAT_PUB =
	"d23fc16d803951992de136eab0ffae8904c93cb9a9e13dabcc9557eeaee16b36"
	"8328ec21d9654263905300689bda1fc7a4945eebf72c9982177fe759c55bf008"
	"383a1efb45931b9aa337587f1c4cf54349f68eb5c919528d1cc1683b4cf3b0fe"
	"2a270c86a6e72f238e08cd102a83bca707ae4c9445a0baf5549a2e1167c3c8b6"
	"fdc3d700b507d24233f10d7c48cce79bc2004460be339d4950265b12a1b53411"
	"3d975e5fb0d477d243c3f1b2decf4589930c0f72bdc65294322bc20dc3529171"
	"ce55a13885717045a143c43bd0ce63556f8a521c2a8a0b57e7b82d3d1ba7674a"
	"c485197f36d96088c4bec00a68e8a3ff90d6efe53b13779f297984f7bfa4bbb2";
// legacy dh_store_and_forward_material pack
static const char *KAT_SF_SKENC =
 "50db5ebc3df21f69173d5cfe0c9db42e";
static const char *KAT_SF_PUB =
	"949357027c5f2e210adb5232d359f2de111068a014a32fab0ed12b6f6a183cc7"
	"36535d891596f70a830abb05fe4f6da72d82927667488e89a37a646070208aa1"
	"e89489dbb10a96101b968c1cee20f5afa5ca3cee8ff3c481f9a48942bd2e38b9"
	"1c90fa2050c738132e5695c672bc9fd3afc7f1d8271d9f1ffd98903f5eaea86e"
	"e86bb60883b136e552594ffa5c6c1d43edda87dc276705d31e42133a253ce3f0"
	"dccad31ea39c52703c25784215ec7afdfefef7579fe27f2aea65f5972c6591df"
	"d1008dba8e050ee0fcdc707d46f6bf984d88c954409fa5a688a5847f315889db"
	"9d7ad13ebc15f6c348930b3f03d97eb69b4ecc0b88a5a2a96039b5b4aa19d041";
static const char *KAT_SF_KEY =
 "f33f05eb41bdc69b1e289bacc6e14d30";
// 2-recipient dh_store_and_forward_material2 pack
static const char *KAT_M2_SK1 =
 "9265b4a4a8126a7216fe5531d590d493";
static const char *KAT_M2_PUB1 =
	"9c0ac112793bbdc283316317eda3027d262c9d493ea6a5dc521d249d85f6cd5a"
	"cd1fd13a1c2a3c087413609e032d75009e395a5efafe5360b60a77d721400d2b"
	"46ae19ba5ac0c263023f2deb0c3f9a000f72e7c4504d021db6e44e596a982cf0"
	"ef0eeba7093def96fa2334244e2159cd76e32e9de804d34481be90777a9e2b05"
	"5872d4e61da80bd82b1469a6b0d830f74251984e37e485fe2f666f0232c221f8"
	"7ea5746dce55c62e06f7b239f08172e27592129be6abad28b6ef27f7a2a42db9"
	"5bad50dfd7c530b7479b24c7285b1d26b0a4ef9a422e2e6b02ee93c3b245afb1"
	"2a16a3a0d8d14c5d6eb1346075088a6c2f158a75aef61063694312d67ccd10f0";
static const char *KAT_M2_SK2 =
 "daaec7ef7f11b9e67d05a3f248108308";
static const char *KAT_M2_PUB2 =
	"c931f6b398ea12852a43462f44d53bb97b19b4bba3edb557a47c31b4d0566abb"
	"4b959c150545165b3dd0926c038638e9fda65b7da015217d37d164302e7c5067"
	"4cab587afe816def1a49ff3badd97110657fe91dbc291bd11d0aa60f63c22bae"
	"75f5d36045e2eab3950906fad5c67e07b49ba0ad316f4f93cd3d7cb4968638c8"
	"a90d24798f64803b9651d3ccee0f05942a72039fad86718d65faa3911ff0e78f"
	"6d9107af69fdfcd7287779587d78790d13396704a7c051c80305aef8139b9de2"
	"6889fdb4348ff11ede231ca482180a9e3f958fdc0e4a359ffafaca530db547fa"
	"0d36781f7a3c6cbc7184efa6aa03f95767d78ee28dfb3eede1726186a67b751a";
static const char *KAT_M2_CHECK =
 "22dee4";
static const char *KAT_M2_KEY =
 "490e462fd2a73712298a3369bba1e35d";

static void
gen_kat()
{
	udh_init(vcnil);
	vc s = udh_new_static(vcnil);

	vc sess;
	vc m = dh_store_and_forward_material(s, sess);

	vc recip(VC_VECTOR);
	recip[0] = s;
	recip[1] = s;
	vc sess2;
	vc m2 = dh_store_and_forward_material2(recip, sess2);

	printf("KAT_PRIV    \"%s\"\n", to_hex(to_str(s[1])).c_str());
	printf("KAT_PUB     \"%s\"\n", to_hex(to_str(s[0])).c_str());
	printf("KAT_SF_SKENC \"%s\"\n", to_hex(to_str(m[0])).c_str());
	printf("KAT_SF_PUB  \"%s\"\n", to_hex(to_str(m[1])).c_str());
	printf("KAT_SF_KEY  \"%s\"\n", to_hex(to_str(sess)).c_str());
	printf("KAT_M2_SK1  \"%s\"\n", to_hex(to_str(m2[0])).c_str());
	printf("KAT_M2_PUB1 \"%s\"\n", to_hex(to_str(m2[1])).c_str());
	printf("KAT_M2_SK2  \"%s\"\n", to_hex(to_str(m2[2])).c_str());
	printf("KAT_M2_PUB2 \"%s\"\n", to_hex(to_str(m2[3])).c_str());
	printf("KAT_M2_CHECK \"%s\"\n", to_hex(to_str(m2[4])).c_str());
	printf("KAT_M2_KEY  \"%s\"\n", to_hex(to_str(sess2)).c_str());
}

// ---------------------------------------------------------------------------
// 1. crash group: uninitialized entry points
//
// These must run before ANY udh_init() call, since the statics are never
// reset. Every case passes well-formed arguments so that the only thing that
// can stop the call is the missing initialization.
// ---------------------------------------------------------------------------

static void crash_udh_new_static() { (void)udh_new_static(vcnil); }

static void crash_udh_public_from_private()
{
	// 256 bytes, so the fault is the null UDH and not a short-key read
	(void)udh_public_from_private(to_vc_bytes(std::string(DH_KEYLEN, (char)0x01)));
}

static void crash_udh_gen_keys()
{
	vc s(VC_VECTOR);
	s[0] = to_vc_bytes(std::string(DH_KEYLEN, (char)0x01));
	s[1] = to_vc_bytes(std::string(DH_KEYLEN, (char)0x02));
	(void)udh_gen_keys(s, vcnil);
}

static void crash_udh_agree_auth()
{
	vc our(VC_VECTOR);
	our[0] = to_vc_bytes(std::string(DH_KEYLEN, (char)0x01));
	our[1] = to_vc_bytes(std::string(DH_KEYLEN, (char)0x02));
	our[2] = to_vc_bytes(std::string(DH_KEYLEN, (char)0x03));
	our[3] = to_vc_bytes(std::string(DH_KEYLEN, (char)0x04));
	(void)udh_agree_auth(our, udh_just_publics(our));
}

static void crash_sf_material()
{
	vc other(VC_VECTOR);
	other[0] = to_vc_bytes(std::string(DH_KEYLEN, (char)0x01));
	vc key;
	(void)dh_store_and_forward_material(other, key);
}

static void crash_sf_material2()
{
	vc other(VC_VECTOR);
	other[0] = to_vc_bytes(std::string(DH_KEYLEN, (char)0x01));
	vc recip(VC_VECTOR);
	recip[0] = other;
	vc key;
	(void)dh_store_and_forward_material2(recip, key);
}

static void crash_sf_get_key()
{
	// must be a 2-element pack to get past the num_elems() early-out
	vc pack(VC_VECTOR);
	pack[0] = to_vc_bytes(std::string(SKEY_LEN, (char)0x00));
	pack[1] = to_vc_bytes(std::string(DH_KEYLEN, (char)0x01));
	vc our(VC_VECTOR);
	our[0] = to_vc_bytes(std::string(DH_KEYLEN, (char)0x01));
	our[1] = to_vc_bytes(std::string(DH_KEYLEN, (char)0x02));
	(void)dh_store_and_forward_get_key(pack, our);
}

static void crash_sf_get_key2()
{
	// 3 elements so the odd-parity check passes and the p2p branch calls
	// check_and_get_key, which is the thing that derefs EphDH
	vc pack(VC_VECTOR);
	pack[0] = to_vc_bytes(std::string(SKEY_LEN, (char)0x00));
	pack[1] = to_vc_bytes(std::string(DH_KEYLEN, (char)0x01));
	pack[2] = to_vc_bytes(std::string(3, (char)0x00));
	vc our(VC_VECTOR);
	our[0] = to_vc_bytes(std::string(DH_KEYLEN, (char)0x01));
	our[1] = to_vc_bytes(std::string(DH_KEYLEN, (char)0x02));
	(void)dh_store_and_forward_get_key2(pack, our);
}

static void crash_vclh_sf_material()
{
	// key_out MUST be a string: the existing non-string guard at
	// vcudh.cpp:362 fires first and would (correctly) bomb instead
	vc other(VC_VECTOR);
	other[0] = to_vc_bytes(std::string(DH_KEYLEN, (char)0x01));
	(void)vclh_sf_material(other, vc("some-key-name"));
}

static void run_crash_group()
{
	check_crash("udh_new_static", crash_udh_new_static);
	check_crash("udh_public_from_private", crash_udh_public_from_private);
	check_crash("udh_gen_keys", crash_udh_gen_keys);
	check_crash("udh_agree_auth", crash_udh_agree_auth);
	check_crash("dh_store_and_forward_material", crash_sf_material);
	check_crash("dh_store_and_forward_material2", crash_sf_material2);
	check_crash("dh_store_and_forward_get_key", crash_sf_get_key);
	check_crash("dh_store_and_forward_get_key2", crash_sf_get_key2);
	check_crash("vclh_sf_material", crash_vclh_sf_material);
}

int main(int argc, char **argv)
{
	if(argc > 1 && strcmp(argv[1], "--gen-kat") == 0) {
		// Vcmap is not needed for this path, but vcctx keeps some
		// code paths happy; keep it cheap and local
		Vcmap = new vcctx;
		Vcmap->open_ctx();
		gen_kat();
		return 0;
	}

	// ---- crash group, first, before anything inits the statics ----
	run_crash_group();

	// ---- negative-path tests: bogus args and bogus files must fail
	// gracefully through USER_BOMB / exceptions, never crash ----
	Throw_user_panic = 1;
	// USER_BOMB only degrades gracefully if either Throw_user_panic is set
	// (it throws before touching Vcmap) or a Vcmap exists. Set both so this
	// test does not depend on which path is taken.
	Vcmap = new vcctx;
	Vcmap->open_ctx();

	// ---- init_rng / udh_init branches ----
	// Three calls: re-parsing the 2048-bit group is not cheap, so this is
	// the minimum that still covers both short-circuits of
	// "entropy.type() == VC_STRING && entropy.len() >= 16" and the
	// delete-previous-globals paths (calls 2 and 3). The last one leaves the
	// statics live for everything that follows.
	check(vctrue == udh_init(vc(5)), "udh_init: non-string entropy");
	check(vctrue == udh_init(vc("short")), "udh_init: string entropy shorter than 16 bytes");
	check(vctrue == udh_init(vc("sixteen+byteentropy")), "udh_init: string entropy of 18 bytes");
	// With a real Vcmap and Throw_user_panic clear, USER_BOMB must not abort
	// the process (vcmap.h routes it through dobacktrace's "if(Vcmap)" branch
	// instead of oopanic). It reports the error, raises the backout flag that
	// the LH evaluator watches, and then returns.
	//
	// The return value here is nil, not a usable result: vc_default's
	// USER_BOMB paths return `vcbitbucket`, and that global is default
	// constructed, and a default vc is nil. That is why vclh_sf-material
	// must never be handed a non-vector other_pub -- the bomb turns the
	// index into nil, and nil is then handed straight to DH::Agree as a
	// 256-byte public key, i.e. a read from a null data pointer. Which of
	// those two things happens (a clean nil, or garbage out of Agree)
	// differed between -O2 and -O0 here, so it is deliberately NOT asserted
	// on; the probe is a bare atomic index to keep it deterministic.
	Throw_user_panic = 0;
	{
		vc probe = vc(5)[0];
		check(Vcmap->dbg_backout_in_progress(),
			"neg: USER_BOMB with a live Vcmap raises the backout flag instead of aborting");
		check(probe.is_nil(),
			"known hazard: indexing an atomic yields nil, which callers then pass on as a key");
	}
	Vcmap->clear_dbg_backout();
	Throw_user_panic = 1;

	// ---- key generation ----
	// Three statics: a and b are used as recipients, c is deliberately NOT a
	// recipient of anything, so it stands in for "our p2p key does not match
	// this pack" and forces the group-key loop in get_key2.
	vc a = udh_new_static(vcnil);
	vc b = udh_new_static(vc("entropy-for-a-different-static-key"));
	vc c = udh_new_static(vc("entropy-for-a-third-static-key"));
	check(a[0].len() == (long)DH_KEYLEN && a[1].len() == (long)DH_KEYLEN,
		"udh_new_static: 256-byte pub/priv");
	check(to_str(a[0]) != to_str(b[0]) && to_str(b[0]) != to_str(c[0]),
		"udh_new_static: three calls give different keys");

	// static public derivation is deterministic: priv -> pub, same pub
	check(to_str(udh_public_from_private(a[1])) == to_str(a[0]),
		"udh_public_from_private: matches the generated static public");

	// ---- ephemeral key generation ----
	vc ka = udh_gen_keys(a, vcnil);
	vc kb = udh_gen_keys(b, vc("entropy-for-ephemeral-keys"));
	for(int i = 0; i < 4; ++i) {
		check(ka[i].len() == (long)DH_KEYLEN && kb[i].len() == (long)DH_KEYLEN,
			std::string("udh_gen_keys: 256-byte slot ") + std::to_string(i));
	}
	check(to_str(ka[2]) == to_str(a[1]) && to_str(ka[3]) == to_str(a[0]),
		"udh_gen_keys: static material carried through at [2]/[3]");
	check(to_str(ka[0]) != to_str(ka[1]), "udh_gen_keys: ephemeral priv != pub");
	check(to_str(kb[2]) == to_str(b[1]) && to_str(kb[3]) == to_str(b[0]),
		"udh_gen_keys: static material carried through for b at [2]/[3]");

	// ---- udh_just_publics strips the private halves ----
	vc pa = udh_just_publics(ka);
	check(pa[0].is_nil() && pa[2].is_nil(), "udh_just_publics: [0] and [2] are nil");
	check(to_str(pa[1]) == to_str(ka[1]) && to_str(pa[3]) == to_str(ka[3]),
		"udh_just_publics: ephemeral pub and static pub pass through");
	check(to_str(pa[1]) != to_str(ka[0]), "udh_just_publics: ephemeral private is dropped");

	// ---- authenticated agreement ----
	vc sa = udh_agree_auth(ka, udh_just_publics(kb));
	vc sb = udh_agree_auth(kb, udh_just_publics(ka));
	check(sa.len() == (long)DH_AGREEDLEN, "udh_agree: 512-byte agreed value (2x256)");
	check(sa.type() == VC_STRING && sb.type() == VC_STRING && sa == sb,
		"udh_agree: both directions produce the same key");

	// ---- store and forward, single recipient (encrypted to b) ----
	vc sess, m;
	m = dh_store_and_forward_material(b, sess);
	check(m.type() == VC_VECTOR && m.num_elems() == 2, "sf_material: 2-element pack");
	check(m[0].len() == (long)SKEY_LEN, "sf_material: encrypted key is 16 bytes");
	check(m[1].len() == (long)DH_KEYLEN, "sf_material: our public DH value is 256 bytes");
	check(to_str(dh_store_and_forward_get_key(m, b)) == to_str(sess),
		"sf_get_key: recipient b recovers the session key");

	// vclh_dh_store_and_forward_get_key is the thin LH wrapper (bound as
	// UDH-sf-get-key). It takes key_out by value rather than by reference,
	// so it cannot bind the session key locally; it just forwards.
	check(to_str(vclh_dh_store_and_forward_get_key(m, b)) == to_str(sess),
		"vclh_sf_get_key: LH wrapper recovers the same session key");
	check(vclh_dh_store_and_forward_get_key(make_bad_pub_pack(), b).is_nil(),
		"neg: vclh_sf_get_key invalid public value");
	check(fails_gracefully([&]{ (void)vclh_dh_store_and_forward_get_key(vc("notapack"), b); }),
		"neg: vclh_sf_get_key string pack bombs");

	// a and c are not the intended recipient. They recover a *different* key
	// rather than nil: the legacy single-key pack carries no integrity check,
	// so a wrong static is only caught by the AES/GCM MAC further up. Pin
	// that, because "returns a key" here does not mean "returns the right key".
	check(to_str(dh_store_and_forward_get_key(m, a)) != to_str(sess),
		"sf_get_key: wrong static yields a different key (no MAC in this path)");
	check(to_str(dh_store_and_forward_get_key(m, c)) != to_str(sess),
		"sf_get_key: second wrong static also yields a different key");

	// ---- store and forward, multi recipient (recipients a and b) ----
	vc recip(VC_VECTOR);
	recip[0] = a;
	recip[1] = b;
	vc sess2;
	vc m2 = dh_store_and_forward_material2(recip, sess2);
	check(m2.type() == VC_VECTOR && m2.num_elems() == 5,
		"sf_material2: 2 pairs plus a 3-byte check string");
	check(m2[0].len() == (long)SKEY_LEN && m2[2].len() == (long)SKEY_LEN,
		"sf_material2: both encrypted keys are 16 bytes");
	check(m2[1].len() == (long)DH_KEYLEN && m2[3].len() == (long)DH_KEYLEN,
		"sf_material2: both public DH values are 256 bytes");
	check(m2[4].len() == 3, "sf_material2: 3-byte key check string");
	check(m2[1] != m2[3], "sf_material2: a fresh ephemeral key per recipient");

	// get_key2 checks the 3-byte AES-ECB key check string, so a pack that
	// was really encrypted to a/b can only be recovered by a or b. c is not
	// a recipient, so it must fail every slot.

	// p2p slot hit: our_material[0] is a, which is recipient 0
	vc om_p2p(VC_VECTOR);
	om_p2p[0] = a;
	check(to_str(dh_store_and_forward_get_key2(m2, om_p2p)) == to_str(sess2),
		"sf_get_key2: recipient 0 recovered from the p2p slot");

	// group-key loop: our_material[0] is c, which is NOT a recipient, so the
	// p2p attempt must miss on the check string and the loop over
	// our_material[1..n] must find b
	vc om_group(VC_VECTOR);
	om_group[0] = c;
	om_group[1] = b;
	check(to_str(dh_store_and_forward_get_key2(m2, om_group)) == to_str(sess2),
		"sf_get_key2: recipient 1 recovered via the group-key loop");

	// c is not a recipient in any slot, so nothing can be recovered
	vc om_nomatch(VC_VECTOR);
	om_nomatch[0] = c;
	check(dh_store_and_forward_get_key2(m2, om_nomatch).is_nil(),
		"sf_get_key2: no matching key check string returns nil");

	// ---- nil recipients ----
	// A nil recipient must produce nil slots, not abort, and must not stop
	// the remaining recipients from working. Note the position matters a
	// lot: get_key2 treats slot pair 0 as the p2p key and pair 1 as THE
	// group key, so a nil in pair 1 makes every later recipient unreachable.
	// Both cases are pinned below.
	vc om_b3(VC_VECTOR);
	om_b3[0] = c;    // c is not a recipient, so the p2p slot must miss
	om_b3[1] = b;    // and the group loop has to find b

	// nil at the tail: the group slot still holds b, so b is recoverable
	vc recip_nil_tail(VC_VECTOR);
	recip_nil_tail[0] = a;
	recip_nil_tail[1] = b;
	recip_nil_tail[2] = vcnil;
	vc sess3;
	vc m3 = dh_store_and_forward_material2(recip_nil_tail, sess3);
	check(m3.num_elems() == 7, "sf_material2: 3 pairs plus check string");
	check(m3[4].is_nil() && m3[5].is_nil(), "sf_material2: nil recipient yields nil slots");
	check(to_str(dh_store_and_forward_get_key2(m3, om_b3)) == to_str(sess3),
		"sf_get_key2: nil recipient skipped, later recipient still recovered");

	// nil in the group slot: the group branch is gated on
	// "sfpack[2] and sfpack[3] are strings", so the recipient sitting in
	// pair 2 becomes permanently unreachable. Pin that.
	vc recip_nil_mid(VC_VECTOR);
	recip_nil_mid[0] = a;
	recip_nil_mid[1] = vcnil;
	recip_nil_mid[2] = b;
	vc sess3b;
	vc m3b = dh_store_and_forward_material2(recip_nil_mid, sess3b);
	check(m3b.num_elems() == 7, "sf_material2: 3 pairs plus check string");
	check(m3b[2].is_nil() && m3b[3].is_nil(), "sf_material2: nil recipient yields nil slots");
	check(dh_store_and_forward_get_key2(m3b, om_b3).is_nil(),
		"known limitation: a nil in the group-key slot makes that pair unrecoverable");

	// ---- empty recipient vector: loop body never runs, only the check
	// string is produced ----
	vc recip_empty(VC_VECTOR);
	vc sess4;
	vc m4 = dh_store_and_forward_material2(recip_empty, sess4);
	check(m4.type() == VC_VECTOR && m4.num_elems() == 1 && m4[0].len() == 3,
		"sf_material2: empty recipient vector yields only the check string");
	check(sess4.len() == (long)SKEY_LEN, "sf_material2: session key still generated");
	check(dh_store_and_forward_get_key2(m4, om_p2p).is_nil(),
		"sf_get_key2: empty pack falls into the compat path and then returns nil");

	// ---- compatibility path: an even-length or too-short pack is retried
	// through the legacy single-key decoder on our_material[0]. The legacy
	// decoder has no key check string, so the right static is needed here --
	// m was encrypted to b, so our_material[0] must be b ----
	vc om_legacy(VC_VECTOR);
	om_legacy[0] = b;
	check(to_str(dh_store_and_forward_get_key2(m, om_legacy)) == to_str(sess),
		"sf_get_key2: 2-element pack retried through the legacy decoder");
	// and with the wrong static it still returns a key, just not the right one
	vc om_legacy_bad(VC_VECTOR);
	om_legacy_bad[0] = c;
	check(to_str(dh_store_and_forward_get_key2(m, om_legacy_bad)) != to_str(sess),
		"sf_get_key2: legacy retry with the wrong static yields a different key, not nil");

	// ---- known-answer tests ----
	if(KAT_PRIV[0]) {
		vc kpriv = to_vc_bytes(from_hex(KAT_PRIV));
		vc kpub = to_vc_bytes(from_hex(KAT_PUB));
		vc kstatic(VC_VECTOR);
		kstatic[0] = kpub;
		kstatic[1] = kpriv;
		check(to_str(udh_public_from_private(kpriv)) == from_hex(KAT_PUB),
			"KAT: static public derived from the captured private key");
		vc kg = udh_gen_keys(kstatic, vcnil);
		check(to_str(kg[3]) == from_hex(KAT_PUB),
			"KAT: udh_gen_keys carries the captured static public through");

		vc kpack(VC_VECTOR);
		kpack[0] = to_vc_bytes(from_hex(KAT_SF_SKENC));
		kpack[1] = to_vc_bytes(from_hex(KAT_SF_PUB));
		check(to_str(dh_store_and_forward_get_key(kpack, kstatic)) == from_hex(KAT_SF_KEY),
			"KAT: legacy store-and-forward pack recovers the captured session key");

		vc m2pack(VC_VECTOR);
		m2pack[0] = to_vc_bytes(from_hex(KAT_M2_SK1));
		m2pack[1] = to_vc_bytes(from_hex(KAT_M2_PUB1));
		m2pack[2] = to_vc_bytes(from_hex(KAT_M2_SK2));
		m2pack[3] = to_vc_bytes(from_hex(KAT_M2_PUB2));
		m2pack[4] = to_vc_bytes(from_hex(KAT_M2_CHECK));
		vc kom(VC_VECTOR);
		kom[0] = kstatic;
		check(to_str(dh_store_and_forward_get_key2(m2pack, kom)) == from_hex(KAT_M2_KEY),
			"KAT: multi-recipient pack recovers the captured session key");
	} else {
		check(false, "KAT: vectors not generated yet (run udh_cpp_test --gen-kat)");
	}

	// ---- negatives: dh_store_and_forward_get_key ----
	// sfpack with fewer than 2 elements. The first case needs a bomb-safe
	// call, so it goes through fails_gracefully rather than being evaluated
	// as a plain operand (a throw there would escape).
	check(dh_store_and_forward_get_key(vc(VC_VECTOR), b).is_nil(),
		"neg: sf_get_key empty pack");
	// a string pack reaches num_elems(), which is an atomic operation and
	// bombs before the type check can reject it
	check(fails_gracefully([&]{ (void)dh_store_and_forward_get_key(vc("notapack"), b); }),
		"neg: sf_get_key string pack (num_elems bombs on atomics)");
	// our_material not a vector
	check(dh_store_and_forward_get_key(m, vcnil).is_nil(),
		"neg: sf_get_key nil material");
	// our_material[1] not a string
	{
		vc our(VC_VECTOR);
		our[0] = b[0];
		our[1] = vcnil;
		check(dh_store_and_forward_get_key(m, our).is_nil(),
			"neg: sf_get_key material with no private key");
	}
	// sfpack is decomposable with >= 2 elements but not a vector. A string
	// cannot reach this check because num_elems() bombs first (it is an
	// atomic), so a map is the only way in. Note vc_map::operator[] is not
	// implemented (it oopanics), so build it with add_kv.
	{
		vc mp(VC_MAP);
		mp.add_kv(vc("a"), m[0]);
		mp.add_kv(vc("b"), m[1]);
		check(mp.num_elems() == 2, "neg: map pack has 2 elements");
		check(dh_store_and_forward_get_key(mp, b).is_nil(),
			"neg: sf_get_key map pack is rejected (not a vector)");
	}
	// sfpack slots that are not strings
	{
		vc p1(VC_VECTOR);
		p1[0] = vcnil;
		p1[1] = m[1];
		check(dh_store_and_forward_get_key(p1, b).is_nil(),
			"neg: sf_get_key pack[0] not a string");
		vc p2(VC_VECTOR);
		p2[0] = m[0];
		p2[1] = vcnil;
		check(dh_store_and_forward_get_key(p2, b).is_nil(),
			"neg: sf_get_key pack[1] not a string");
	}
	// encrypted key longer than the 20-byte SHA1 kdk. The bound is
	// sk_enc.len() <= kdk.len(), so 17 bytes is still fine -- it has to
	// exceed 20 to be rejected.
	{
		vc p(VC_VECTOR);
		p[0] = to_vc_bytes(std::string(KDK_LEN + 1, (char)0x00));
		p[1] = m[1];
		check(dh_store_and_forward_get_key(p, b).is_nil(),
			"neg: sf_get_key encrypted key longer than the kdk");
		// ... and exactly kdk.len() is still accepted
		vc ok(VC_VECTOR);
		ok[0] = to_vc_bytes(std::string(KDK_LEN, (char)0x00));
		ok[1] = m[1];
		check(!dh_store_and_forward_get_key(ok, b).is_nil(),
			"neg: sf_get_key encrypted key of exactly kdk length is accepted");
	}
	// invalid public value: DH::Agree returns false rather than throwing
	check(dh_store_and_forward_get_key(make_bad_pub_pack(), b).is_nil(),
		"neg: sf_get_key invalid public value");

	// ---- negatives: dh_store_and_forward_get_key2 ----
	// get_key2 checks sfpack.type() before touching num_elems(), so unlike
	// get_key a string pack is rejected cleanly instead of bombing
	check(dh_store_and_forward_get_key2(vc("notapack"), om_p2p).is_nil(),
		"neg: sf_get_key2 string pack rejected without bombing");
	check(dh_store_and_forward_get_key2(vc(5), om_p2p).is_nil(),
		"neg: sf_get_key2 non-vector pack");
	check(dh_store_and_forward_get_key2(m2, vcnil).is_nil(),
		"neg: sf_get_key2 nil material");
	check(dh_store_and_forward_get_key2(m2, vc(5)).is_nil(),
		"neg: sf_get_key2 non-vector material");
	{
		// p2p slot present but the check string does not match
		vc bad(VC_VECTOR);
		bad[0] = m2[0];
		bad[1] = m2[1];
		bad[2] = to_vc_bytes(std::string(3, (char)0xFF));
		check(dh_store_and_forward_get_key2(bad, om_p2p).is_nil(),
			"neg: sf_get_key2 wrong key check string rejected");
		// 3-element pack: the p2p slot is intact and there is no group slot
		// to fall back on, so a correct check string still recovers the key
		vc three(VC_VECTOR);
		three[0] = m2[0];
		three[1] = m2[1];
		three[2] = m2[4];
		check(to_str(dh_store_and_forward_get_key2(three, om_p2p)) == to_str(sess2),
			"sf_get_key2: 3-element pack recovered from the p2p slot alone");
		// same pack with a corrupted check string: nothing to fall back on
		vc three_bad(VC_VECTOR);
		three_bad[0] = m2[0];
		three_bad[1] = m2[1];
		three_bad[2] = to_vc_bytes(std::string(3, (char)0xFF));
		check(dh_store_and_forward_get_key2(three_bad, om_p2p).is_nil(),
			"neg: sf_get_key2 3-element pack with a mismatched check string");
		// group slot exists but neither material works. Note this BOMBS
		// rather than returning nil: get_key2 has no "our_material is a
		// vector" guard, so an empty material makes our_material[0] nil and
		// check_and_get_key then indexes that nil (an atomic), which is a
		// USER_BOMB. dh_store_and_forward_get_key does have that guard.
		check(fails_gracefully([&]{ (void)dh_store_and_forward_get_key2(m2, vc(VC_VECTOR)); }),
			"known limitation: sf_get_key2 with an empty material vector bombs instead of returning nil");
		// group slot present, but the check string is wrong so the loop over
		// our_material[1..n] cannot confirm anything either
		vc bad2(VC_VECTOR);
		bad2[0] = m2[0];
		bad2[1] = m2[1];
		bad2[2] = m2[2];
		bad2[3] = m2[3];
		bad2[4] = to_vc_bytes(std::string(3, (char)0xFF));
		check(dh_store_and_forward_get_key2(bad2, om_group).is_nil(),
			"neg: sf_get_key2 group slot with a wrong check string rejected");
		// sfpack[1] not a string: the p2p slot is skipped entirely
		vc bad3(VC_VECTOR);
		bad3[0] = m2[0];
		bad3[1] = vcnil;
		bad3[2] = m2[4];
		check(dh_store_and_forward_get_key2(bad3, om_p2p).is_nil(),
			"neg: sf_get_key2 p2p public slot not a string is skipped");
		// the same sk_enc.len() <= kdk.len() bound that the legacy path
		// enforces, reached through check_and_get_key this time. The public
		// value is valid (m2[1] pairs with a) so the agreement succeeds and
		// the length check is what rejects it.
		vc longkey(VC_VECTOR);
		longkey[0] = to_vc_bytes(std::string(KDK_LEN + 1, (char)0x00));
		longkey[1] = m2[1];
		longkey[2] = m2[4];
		check(dh_store_and_forward_get_key2(longkey, om_p2p).is_nil(),
			"neg: sf_get_key2 p2p encrypted key longer than the kdk");
	}
	// invalid public value inside the p2p slot
	{
		vc bad(VC_VECTOR);
		bad[0] = m2[0];
		bad[1] = to_vc_bytes(invalid_pub());
		bad[2] = m2[4];
		check(dh_store_and_forward_get_key2(bad, om_p2p).is_nil(),
			"neg: sf_get_key2 invalid public value in the p2p slot");
	}

	// ---- negatives: dh_store_and_forward_material / material2 ----
	{
		vc other(VC_VECTOR);
		other[0] = to_vc_bytes(invalid_pub());
		vc key;
		check(dh_store_and_forward_material(other, key).is_nil(),
			"neg: sf_material invalid public value");
		vc recip(VC_VECTOR);
		recip[0] = other;
		vc key2;
		check(dh_store_and_forward_material2(recip, key2).is_nil(),
			"neg: sf_material2 invalid public value");
		// a valid recipient first, invalid second: the loop must bail out of
		// the whole call rather than return a partial pack
		vc recip_mixed(VC_VECTOR);
		recip_mixed[0] = b;
		recip_mixed[1] = other;
		vc key3;
		check(dh_store_and_forward_material2(recip_mixed, key3).is_nil(),
			"neg: sf_material2 aborts on a bad recipient after a good one");
	}

	// ---- negatives: udh_agree_auth with an invalid peer public ----
	{
		vc bad(VC_VECTOR);
		bad[0] = vcnil;
		bad[1] = to_vc_bytes(invalid_pub());
		bad[2] = vcnil;
		bad[3] = to_vc_bytes(invalid_pub());
		check(udh_agree_auth(ka, bad).is_nil(),
			"neg: udh_agree invalid peer public");
		// bad only in the ephemeral slot
		vc bad_eph(VC_VECTOR);
		bad_eph[0] = vcnil;
		bad_eph[1] = to_vc_bytes(invalid_pub());
		bad_eph[2] = vcnil;
		bad_eph[3] = kb[3];
		check(udh_agree_auth(ka, bad_eph).is_nil(),
			"neg: udh_agree invalid peer ephemeral public");
	}

	// ---- negatives: argument types (USER_BOMB via the vc atomic rep) ----
	check(fails_gracefully([&]{ (void)vclh_sf_material(b[0], vc(5)); }),
		"neg: sf-material second arg must be a string");
	check(fails_gracefully([&]{ (void)udh_gen_keys(vc(5), vcnil); }),
		"neg: udh_gen_keys non-vector static material");
	check(fails_gracefully([&]{ (void)udh_just_publics(vc(5)); }),
		"neg: udh_just_publics non-vector arg");
	check(fails_gracefully([&]{ (void)udh_agree_auth(vc(5), vc(5)); }),
		"neg: udh_agree non-vector args");

	// entropy arguments of the wrong type are accepted (the code only asks
	// whether it is a string) and simply skip IncorporateEntropy
	check(udh_new_static(vc(5))[0].len() == (long)DH_KEYLEN,
		"neg: udh_new_static tolerates non-string entropy");
	check(udh_gen_keys(a, vc(5))[0].len() == (long)DH_KEYLEN,
		"neg: udh_gen_keys tolerates non-string entropy");

	Vcmap->close_ctx();
	delete Vcmap;
	Vcmap = 0;

	if(fails == 0)
		std::cout << "ALL UDH C++ TESTS PASSED\n";
	return fails ? 1 : 0;
}
