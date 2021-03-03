#define gensym1() vcnil
// set-dynamic-binding
static const vc s_gensym2 = vc(VC_BSTRING, "\x73\x65\x74\x2d\x64\x79\x6e\x61\x6d\x69\x63\x2d\x62\x69\x6e\x64\x69\x6e\x67", 19);
#define gensym2() s_gensym2
static vc gensym3() {
    vc ret = dosetdynamic(gensym1());
    return ret;
}
// N
static const vc s_gensym4 = vc(VC_BSTRING, "\x4e", 1);
#define gensym4() s_gensym4
static const vc s_gensym5(1000);
#define gensym5() s_gensym5
// gbind
static const vc s_gensym6 = vc(VC_BSTRING, "\x67\x62\x69\x6e\x64", 5);
#define gensym6() s_gensym6
static vc gensym7() {
    vc ret;
    Vcmap->global_add(gensym4(), gensym5());
    return ret;
}
// Iter
static const vc s_gensym8 = vc(VC_BSTRING, "\x49\x74\x65\x72", 4);
#define gensym8() s_gensym8
static const vc s_gensym9(50);
#define gensym9() s_gensym9
// gbind
static const vc s_gensym10 = vc(VC_BSTRING, "\x67\x62\x69\x6e\x64", 5);
#define gensym10() s_gensym10
static vc gensym11() {
    vc ret;
    Vcmap->global_add(gensym8(), gensym9());
    return ret;
}
// Limit2
static const vc s_gensym12 = vc(VC_BSTRING, "\x4c\x69\x6d\x69\x74\x32", 6);
#define gensym12() s_gensym12
static const vc s_gensym13(4.000000);
#define gensym13() s_gensym13
// gbind
static const vc s_gensym14 = vc(VC_BSTRING, "\x67\x62\x69\x6e\x64", 5);
#define gensym14() s_gensym14
static vc gensym15() {
    vc ret;
    Vcmap->global_add(gensym12(), gensym13());
    return ret;
}
// stdout
static const vc s_gensym16 = vc(VC_BSTRING, "\x73\x74\x64\x6f\x75\x74", 6);
#define gensym16() s_gensym16
// foo.pnm
static const vc s_gensym17 = vc(VC_BSTRING, "\x66\x6f\x6f\x2e\x70\x6e\x6d", 7);
#define gensym17() s_gensym17
// wb
static const vc s_gensym18 = vc(VC_BSTRING, "\x77\x62", 2);
#define gensym18() s_gensym18
// openfile
static const vc s_gensym19 = vc(VC_BSTRING, "\x6f\x70\x65\x6e\x66\x69\x6c\x65", 8);
#define gensym19() s_gensym19
static vc gensym20() {
    vc ret = doopenfile(gensym17(),gensym18());
    return ret;
}
// gbind
static const vc s_gensym21 = vc(VC_BSTRING, "\x67\x62\x69\x6e\x64", 5);
#define gensym21() s_gensym21
static vc gensym22() {
    vc ret;
    Vcmap->global_add(gensym16(), gensym20());
    return ret;
}
// mandel
static const vc s_gensym23 = vc(VC_BSTRING, "\x6d\x61\x6e\x64\x65\x6c", 6);
#define gensym23() s_gensym23
// N1
static const vc s_gensym24 = vc(VC_BSTRING, "\x4e\x31", 2);
#define gensym24() s_gensym24
static vc gensym25() {
    static vc var = vc(VC_BSTRING, "\x4e", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym26(1);
#define gensym26() s_gensym26
// sub
static const vc s_gensym27 = vc(VC_BSTRING, "\x73\x75\x62", 3);
#define gensym27() s_gensym27
static vc gensym28() {
    vc ret = gensym25() - gensym26();
    return ret;
}
// lbind
static const vc s_gensym29 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym29() s_gensym29
static vc gensym30() {
    vc ret;
    Vcmap->local_add(gensym24(), gensym28());
    return ret;
}
// B
static const vc s_gensym31 = vc(VC_BSTRING, "\x42", 1);
#define gensym31() s_gensym31
static const vc s_gensym32(8);
#define gensym32() s_gensym32
// lbind
static const vc s_gensym33 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym33() s_gensym33
static vc gensym34() {
    vc ret;
    Vcmap->local_add(gensym31(), gensym32());
    return ret;
}
// B1
static const vc s_gensym35 = vc(VC_BSTRING, "\x42\x31", 2);
#define gensym35() s_gensym35
static vc gensym36() {
    static vc var = vc(VC_BSTRING, "\x42", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym37(1);
#define gensym37() s_gensym37
// sub
static const vc s_gensym38 = vc(VC_BSTRING, "\x73\x75\x62", 3);
#define gensym38() s_gensym38
static vc gensym39() {
    vc ret = gensym36() - gensym37();
    return ret;
}
// lbind
static const vc s_gensym40 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym40() s_gensym40
static vc gensym41() {
    vc ret;
    Vcmap->local_add(gensym35(), gensym39());
    return ret;
}
static vc gensym42() {
    static vc var = vc(VC_BSTRING, "\x73\x74\x64\x6f\x75\x74", 6);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// P4
static const vc s_gensym43 = vc(VC_BSTRING, "\x50\x34", 2);
#define gensym43() s_gensym43
//

static const vc s_gensym44 = vc(VC_BSTRING, "\x0a", 1);
#define gensym44() s_gensym44
static vc gensym45() {
    static vc var = vc(VC_BSTRING, "\x4e", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
//
static const vc s_gensym46 = vc(VC_BSTRING, "\x20", 1);
#define gensym46() s_gensym46
static vc gensym47() {
    static vc var = vc(VC_BSTRING, "\x4e", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
//

static const vc s_gensym48 = vc(VC_BSTRING, "\x0a", 1);
#define gensym48() s_gensym48
static vc gensym49() {
    DwString val;
    val += gensym43().peek_str();
    val += gensym44().peek_str();
    val += gensym45().peek_str();
    val += gensym46().peek_str();
    val += gensym47().peek_str();
    val += gensym48().peek_str();
    return val.c_str();
}
// fputs
static const vc s_gensym50 = vc(VC_BSTRING, "\x66\x70\x75\x74\x73", 5);
#define gensym50() s_gensym50
static vc gensym51() {
    vc ret = dofputs(gensym42(),gensym49());
    return ret;
}
// r
static const vc s_gensym52 = vc(VC_BSTRING, "\x72", 1);
#define gensym52() s_gensym52
static const vc s_gensym53(0);
#define gensym53() s_gensym53
static vc gensym54() {
    static vc var = vc(VC_BSTRING, "\x4e\x31", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// c
static const vc s_gensym55 = vc(VC_BSTRING, "\x63", 1);
#define gensym55() s_gensym55
static const vc s_gensym56(0);
#define gensym56() s_gensym56
// lbind
static const vc s_gensym57 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym57() s_gensym57
static vc gensym58() {
    vc ret;
    Vcmap->local_add(gensym55(), gensym56());
    return ret;
}
// byte-acc
static const vc s_gensym59 = vc(VC_BSTRING, "\x62\x79\x74\x65\x2d\x61\x63\x63", 8);
#define gensym59() s_gensym59
static const vc s_gensym60(0);
#define gensym60() s_gensym60
// lbind
static const vc s_gensym61 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym61() s_gensym61
static vc gensym62() {
    vc ret;
    Vcmap->local_add(gensym59(), gensym60());
    return ret;
}
// bit-num
static const vc s_gensym63 = vc(VC_BSTRING, "\x62\x69\x74\x2d\x6e\x75\x6d", 7);
#define gensym63() s_gensym63
static const vc s_gensym64(0);
#define gensym64() s_gensym64
// lbind
static const vc s_gensym65 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym65() s_gensym65
static vc gensym66() {
    vc ret;
    Vcmap->local_add(gensym63(), gensym64());
    return ret;
}
static vc gensym67() {
    static vc var = vc(VC_BSTRING, "\x63", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym68() {
    static vc var = vc(VC_BSTRING, "\x4e", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// lt
static const vc s_gensym69 = vc(VC_BSTRING, "\x6c\x74", 2);
#define gensym69() s_gensym69
static vc gensym70() {
    vc ret = ltfun(gensym67(),gensym68());
    return ret;
}
// zi
static const vc s_gensym71 = vc(VC_BSTRING, "\x7a\x69", 2);
#define gensym71() s_gensym71
static vc gensym72() {
    static vc var = vc(VC_BSTRING, "\x42", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym73(0.000000);
#define gensym73() s_gensym73
// vector2
static const vc s_gensym74 = vc(VC_BSTRING, "\x76\x65\x63\x74\x6f\x72\x32", 7);
#define gensym74() s_gensym74
static vc gensym75() {
    VCArglist a;
    a.set_size(2);
    a.append(gensym72());
    a.append(gensym73());
    vc ret = dovectorsize(&a);
    return ret;
}
// lbind
static const vc s_gensym76 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym76() s_gensym76
static vc gensym77() {
    vc ret;
    Vcmap->local_add(gensym71(), gensym75());
    return ret;
}
// zr
static const vc s_gensym78 = vc(VC_BSTRING, "\x7a\x72", 2);
#define gensym78() s_gensym78
static vc gensym79() {
    static vc var = vc(VC_BSTRING, "\x42", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym80(0.000000);
#define gensym80() s_gensym80
// vector2
static const vc s_gensym81 = vc(VC_BSTRING, "\x76\x65\x63\x74\x6f\x72\x32", 7);
#define gensym81() s_gensym81
static vc gensym82() {
    VCArglist a;
    a.set_size(2);
    a.append(gensym79());
    a.append(gensym80());
    vc ret = dovectorsize(&a);
    return ret;
}
// lbind
static const vc s_gensym83 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym83() s_gensym83
static vc gensym84() {
    vc ret;
    Vcmap->local_add(gensym78(), gensym82());
    return ret;
}
// cr
static const vc s_gensym85 = vc(VC_BSTRING, "\x63\x72", 2);
#define gensym85() s_gensym85
static vc gensym86() {
    static vc var = vc(VC_BSTRING, "\x42", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// vector2
static const vc s_gensym87 = vc(VC_BSTRING, "\x76\x65\x63\x74\x6f\x72\x32", 7);
#define gensym87() s_gensym87
static vc gensym88() {
    VCArglist a;
    a.set_size(1);
    a.append(gensym86());
    vc ret = dovectorsize(&a);
    return ret;
}
// lbind
static const vc s_gensym89 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym89() s_gensym89
static vc gensym90() {
    vc ret;
    Vcmap->local_add(gensym85(), gensym88());
    return ret;
}
// ci
static const vc s_gensym91 = vc(VC_BSTRING, "\x63\x69", 2);
#define gensym91() s_gensym91
static vc gensym92() {
    static vc var = vc(VC_BSTRING, "\x42", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// vector2
static const vc s_gensym93 = vc(VC_BSTRING, "\x76\x65\x63\x74\x6f\x72\x32", 7);
#define gensym93() s_gensym93
static vc gensym94() {
    VCArglist a;
    a.set_size(1);
    a.append(gensym92());
    vc ret = dovectorsize(&a);
    return ret;
}
// lbind
static const vc s_gensym95 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym95() s_gensym95
static vc gensym96() {
    vc ret;
    Vcmap->local_add(gensym91(), gensym94());
    return ret;
}
// tr
static const vc s_gensym97 = vc(VC_BSTRING, "\x74\x72", 2);
#define gensym97() s_gensym97
static vc gensym98() {
    static vc var = vc(VC_BSTRING, "\x42", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym99(0.000000);
#define gensym99() s_gensym99
// vector2
static const vc s_gensym100 = vc(VC_BSTRING, "\x76\x65\x63\x74\x6f\x72\x32", 7);
#define gensym100() s_gensym100
static vc gensym101() {
    VCArglist a;
    a.set_size(2);
    a.append(gensym98());
    a.append(gensym99());
    vc ret = dovectorsize(&a);
    return ret;
}
// lbind
static const vc s_gensym102 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym102() s_gensym102
static vc gensym103() {
    vc ret;
    Vcmap->local_add(gensym97(), gensym101());
    return ret;
}
// ti
static const vc s_gensym104 = vc(VC_BSTRING, "\x74\x69", 2);
#define gensym104() s_gensym104
static vc gensym105() {
    static vc var = vc(VC_BSTRING, "\x42", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym106(0.000000);
#define gensym106() s_gensym106
// vector2
static const vc s_gensym107 = vc(VC_BSTRING, "\x76\x65\x63\x74\x6f\x72\x32", 7);
#define gensym107() s_gensym107
static vc gensym108() {
    VCArglist a;
    a.set_size(2);
    a.append(gensym105());
    a.append(gensym106());
    vc ret = dovectorsize(&a);
    return ret;
}
// lbind
static const vc s_gensym109 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym109() s_gensym109
static vc gensym110() {
    vc ret;
    Vcmap->local_add(gensym104(), gensym108());
    return ret;
}
// i
static const vc s_gensym111 = vc(VC_BSTRING, "\x69", 1);
#define gensym111() s_gensym111
static const vc s_gensym112(0);
#define gensym112() s_gensym112
static vc gensym113() {
    static vc var = vc(VC_BSTRING, "\x42\x31", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym114() {
    static vc var = vc(VC_BSTRING, "\x63\x72", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym115() {
    static vc var = vc(VC_BSTRING, "\x69", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym116(2.000000);
#define gensym116() s_gensym116
static vc gensym117() {
    static vc var = vc(VC_BSTRING, "\x63", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym118() {
    static vc var = vc(VC_BSTRING, "\x69", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// add
static const vc s_gensym119 = vc(VC_BSTRING, "\x61\x64\x64", 3);
#define gensym119() s_gensym119
static vc gensym120() {
    vc ret = gensym117() + gensym118();
    return ret;
}
// mul
static const vc s_gensym121 = vc(VC_BSTRING, "\x6d\x75\x6c", 3);
#define gensym121() s_gensym121
static vc gensym122() {
    vc ret = gensym116() * gensym120();
    return ret;
}
static vc gensym123() {
    static vc var = vc(VC_BSTRING, "\x4e", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// div
static const vc s_gensym124 = vc(VC_BSTRING, "\x64\x69\x76", 3);
#define gensym124() s_gensym124
static vc gensym125() {
    vc ret = gensym122() / gensym123();
    return ret;
}
static const vc s_gensym126(1.500000);
#define gensym126() s_gensym126
// sub
static const vc s_gensym127 = vc(VC_BSTRING, "\x73\x75\x62", 3);
#define gensym127() s_gensym127
static vc gensym128() {
    vc ret = gensym125() - gensym126();
    return ret;
}
// putidx
static const vc s_gensym129 = vc(VC_BSTRING, "\x70\x75\x74\x69\x64\x78", 6);
#define gensym129() s_gensym129
static vc gensym130() {
    vc ret = doputindex(gensym114(),gensym115(),gensym128());
    return ret;
}
static vc gensym131() {
    static vc var = vc(VC_BSTRING, "\x63\x69", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym132() {
    static vc var = vc(VC_BSTRING, "\x69", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym133(2.000000);
#define gensym133() s_gensym133
static vc gensym134() {
    static vc var = vc(VC_BSTRING, "\x72", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// mul
static const vc s_gensym135 = vc(VC_BSTRING, "\x6d\x75\x6c", 3);
#define gensym135() s_gensym135
static vc gensym136() {
    vc ret = gensym133() * gensym134();
    return ret;
}
static vc gensym137() {
    static vc var = vc(VC_BSTRING, "\x4e", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// div
static const vc s_gensym138 = vc(VC_BSTRING, "\x64\x69\x76", 3);
#define gensym138() s_gensym138
static vc gensym139() {
    vc ret = gensym136() / gensym137();
    return ret;
}
static const vc s_gensym140(1.000000);
#define gensym140() s_gensym140
// sub
static const vc s_gensym141 = vc(VC_BSTRING, "\x73\x75\x62", 3);
#define gensym141() s_gensym141
static vc gensym142() {
    vc ret = gensym139() - gensym140();
    return ret;
}
// putidx
static const vc s_gensym143 = vc(VC_BSTRING, "\x70\x75\x74\x69\x64\x78", 6);
#define gensym143() s_gensym143
static vc gensym144() {
    vc ret = doputindex(gensym131(),gensym132(),gensym142());
    return ret;
}
// prog
static const vc s_gensym145 = vc(VC_BSTRING, "\x70\x72\x6f\x67", 4);
#define gensym145() s_gensym145
static vc gensym146() {
    vc ret;
    gensym130();
    gensym144();
    return ret;
}
// for
static const vc s_gensym147 = vc(VC_BSTRING, "\x66\x6f\x72", 3);
#define gensym147() s_gensym147
static vc gensym148() {
    vc ret;
    long l = gensym112();
    long h = gensym113();
    vc var = gensym111();
    try {
        long i = l;
        for(; i <= h; ++i) {
            Vcmap->local_add(var, vc(i));
            gensym146();
        }
    }
    catch(VcBreak& b) {
        --b.lev;
        if(b.lev > 0) throw;
    }
    return vcnil;
    return ret;
}
// ms
static const vc s_gensym149 = vc(VC_BSTRING, "\x6d\x73", 2);
#define gensym149() s_gensym149
static vc gensym150() {
    static vc var = vc(VC_BSTRING, "\x42", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym151(0);
#define gensym151() s_gensym151
// vector2
static const vc s_gensym152 = vc(VC_BSTRING, "\x76\x65\x63\x74\x6f\x72\x32", 7);
#define gensym152() s_gensym152
static vc gensym153() {
    VCArglist a;
    a.set_size(2);
    a.append(gensym150());
    a.append(gensym151());
    vc ret = dovectorsize(&a);
    return ret;
}
// lbind
static const vc s_gensym154 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym154() s_gensym154
static vc gensym155() {
    vc ret;
    Vcmap->local_add(gensym149(), gensym153());
    return ret;
}
// iter
static const vc s_gensym156 = vc(VC_BSTRING, "\x69\x74\x65\x72", 4);
#define gensym156() s_gensym156
static vc gensym157() {
    static vc var = vc(VC_BSTRING, "\x49\x74\x65\x72", 4);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// lbind
static const vc s_gensym158 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym158() s_gensym158
static vc gensym159() {
    vc ret;
    Vcmap->local_add(gensym156(), gensym157());
    return ret;
}
static vc gensym160() {
    static vc var = vc(VC_BSTRING, "\x69\x74\x65\x72", 4);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym161(0);
#define gensym161() s_gensym161
// gt
static const vc s_gensym162 = vc(VC_BSTRING, "\x67\x74", 2);
#define gensym162() s_gensym162
static vc gensym163() {
    vc ret = gtfun(gensym160(),gensym161());
    return ret;
}
// zi
static const vc s_gensym164 = vc(VC_BSTRING, "\x7a\x69", 2);
#define gensym164() s_gensym164
static const vc s_gensym165(2.000000);
#define gensym165() s_gensym165
static vc gensym166() {
    static vc var = vc(VC_BSTRING, "\x7a\x72", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym167() {
    static vc var = vc(VC_BSTRING, "\x7a\x69", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// mul
static const vc s_gensym168 = vc(VC_BSTRING, "\x6d\x75\x6c", 3);
#define gensym168() s_gensym168
static vc gensym169() {
    vc ret = gensym166() * gensym167();
    return ret;
}
// mul
static const vc s_gensym170 = vc(VC_BSTRING, "\x6d\x75\x6c", 3);
#define gensym170() s_gensym170
static vc gensym171() {
    vc ret = gensym165() * gensym169();
    return ret;
}
static vc gensym172() {
    static vc var = vc(VC_BSTRING, "\x63\x69", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// add
static const vc s_gensym173 = vc(VC_BSTRING, "\x61\x64\x64", 3);
#define gensym173() s_gensym173
static vc gensym174() {
    vc ret = gensym171() + gensym172();
    return ret;
}
// lbind
static const vc s_gensym175 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym175() s_gensym175
static vc gensym176() {
    vc ret;
    Vcmap->local_add(gensym164(), gensym174());
    return ret;
}
// zr
static const vc s_gensym177 = vc(VC_BSTRING, "\x7a\x72", 2);
#define gensym177() s_gensym177
static vc gensym178() {
    static vc var = vc(VC_BSTRING, "\x74\x72", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym179() {
    static vc var = vc(VC_BSTRING, "\x74\x69", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// sub
static const vc s_gensym180 = vc(VC_BSTRING, "\x73\x75\x62", 3);
#define gensym180() s_gensym180
static vc gensym181() {
    vc ret = gensym178() - gensym179();
    return ret;
}
static vc gensym182() {
    static vc var = vc(VC_BSTRING, "\x63\x72", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// add
static const vc s_gensym183 = vc(VC_BSTRING, "\x61\x64\x64", 3);
#define gensym183() s_gensym183
static vc gensym184() {
    vc ret = gensym181() + gensym182();
    return ret;
}
// lbind
static const vc s_gensym185 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym185() s_gensym185
static vc gensym186() {
    vc ret;
    Vcmap->local_add(gensym177(), gensym184());
    return ret;
}
// tr
static const vc s_gensym187 = vc(VC_BSTRING, "\x74\x72", 2);
#define gensym187() s_gensym187
static vc gensym188() {
    static vc var = vc(VC_BSTRING, "\x7a\x72", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym189() {
    static vc var = vc(VC_BSTRING, "\x7a\x72", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// mul
static const vc s_gensym190 = vc(VC_BSTRING, "\x6d\x75\x6c", 3);
#define gensym190() s_gensym190
static vc gensym191() {
    vc ret = gensym188() * gensym189();
    return ret;
}
// lbind
static const vc s_gensym192 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym192() s_gensym192
static vc gensym193() {
    vc ret;
    Vcmap->local_add(gensym187(), gensym191());
    return ret;
}
// ti
static const vc s_gensym194 = vc(VC_BSTRING, "\x74\x69", 2);
#define gensym194() s_gensym194
static vc gensym195() {
    static vc var = vc(VC_BSTRING, "\x7a\x69", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym196() {
    static vc var = vc(VC_BSTRING, "\x7a\x69", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// mul
static const vc s_gensym197 = vc(VC_BSTRING, "\x6d\x75\x6c", 3);
#define gensym197() s_gensym197
static vc gensym198() {
    vc ret = gensym195() * gensym196();
    return ret;
}
// lbind
static const vc s_gensym199 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym199() s_gensym199
static vc gensym200() {
    vc ret;
    Vcmap->local_add(gensym194(), gensym198());
    return ret;
}
// ta
static const vc s_gensym201 = vc(VC_BSTRING, "\x74\x61", 2);
#define gensym201() s_gensym201
static vc gensym202() {
    static vc var = vc(VC_BSTRING, "\x74\x72", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym203() {
    static vc var = vc(VC_BSTRING, "\x74\x69", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// add
static const vc s_gensym204 = vc(VC_BSTRING, "\x61\x64\x64", 3);
#define gensym204() s_gensym204
static vc gensym205() {
    vc ret = gensym202() + gensym203();
    return ret;
}
// lbind
static const vc s_gensym206 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym206() s_gensym206
static vc gensym207() {
    vc ret;
    Vcmap->local_add(gensym201(), gensym205());
    return ret;
}
// i
static const vc s_gensym208 = vc(VC_BSTRING, "\x69", 1);
#define gensym208() s_gensym208
static const vc s_gensym209(0);
#define gensym209() s_gensym209
// lbind
static const vc s_gensym210 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym210() s_gensym210
static vc gensym211() {
    vc ret;
    Vcmap->local_add(gensym208(), gensym209());
    return ret;
}
// all
static const vc s_gensym212 = vc(VC_BSTRING, "\x61\x6c\x6c", 3);
#define gensym212() s_gensym212
static const vc s_gensym213(0);
#define gensym213() s_gensym213
// lbind
static const vc s_gensym214 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym214() s_gensym214
static vc gensym215() {
    vc ret;
    Vcmap->local_add(gensym212(), gensym213());
    return ret;
}
// v
static const vc s_gensym216 = vc(VC_BSTRING, "\x76", 1);
#define gensym216() s_gensym216
static vc gensym217() {
    static vc var = vc(VC_BSTRING, "\x74\x61", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym218() {
    static vc var = vc(VC_BSTRING, "\x76", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym219() {
    static vc var = vc(VC_BSTRING, "\x4c\x69\x6d\x69\x74\x32", 6);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// gt
static const vc s_gensym220 = vc(VC_BSTRING, "\x67\x74", 2);
#define gensym220() s_gensym220
static vc gensym221() {
    vc ret = gtfun(gensym218(),gensym219());
    return ret;
}
// all
static const vc s_gensym222 = vc(VC_BSTRING, "\x61\x6c\x6c", 3);
#define gensym222() s_gensym222
static vc gensym223() {
    static vc var = vc(VC_BSTRING, "\x61\x6c\x6c", 3);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym224(1);
#define gensym224() s_gensym224
// add
static const vc s_gensym225 = vc(VC_BSTRING, "\x61\x64\x64", 3);
#define gensym225() s_gensym225
static vc gensym226() {
    vc ret = gensym223() + gensym224();
    return ret;
}
// lbind
static const vc s_gensym227 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym227() s_gensym227
static vc gensym228() {
    vc ret;
    Vcmap->local_add(gensym222(), gensym226());
    return ret;
}
static vc gensym229() {
    static vc var = vc(VC_BSTRING, "\x6d\x73", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym230() {
    static vc var = vc(VC_BSTRING, "\x69", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym231(1);
#define gensym231() s_gensym231
// putidx
static const vc s_gensym232 = vc(VC_BSTRING, "\x70\x75\x74\x69\x64\x78", 6);
#define gensym232() s_gensym232
static vc gensym233() {
    vc ret = doputindex(gensym229(),gensym230(),gensym231());
    return ret;
}
// prog
static const vc s_gensym234 = vc(VC_BSTRING, "\x70\x72\x6f\x67", 4);
#define gensym234() s_gensym234
static vc gensym235() {
    vc ret;
    gensym228();
    gensym233();
    return ret;
}
// if
static const vc s_gensym236 = vc(VC_BSTRING, "\x69\x66", 2);
#define gensym236() s_gensym236
static vc gensym237() {
    vc ret;
    if(!gensym221().is_nil()) {
        ret = gensym235();
    }
    return ret;
}
// i
static const vc s_gensym238 = vc(VC_BSTRING, "\x69", 1);
#define gensym238() s_gensym238
static vc gensym239() {
    static vc var = vc(VC_BSTRING, "\x69", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym240(1);
#define gensym240() s_gensym240
// add
static const vc s_gensym241 = vc(VC_BSTRING, "\x61\x64\x64", 3);
#define gensym241() s_gensym241
static vc gensym242() {
    vc ret = gensym239() + gensym240();
    return ret;
}
// lbind
static const vc s_gensym243 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym243() s_gensym243
static vc gensym244() {
    vc ret;
    Vcmap->local_add(gensym238(), gensym242());
    return ret;
}
// prog
static const vc s_gensym245 = vc(VC_BSTRING, "\x70\x72\x6f\x67", 4);
#define gensym245() s_gensym245
static vc gensym246() {
    vc ret;
    gensym237();
    gensym244();
    return ret;
}
// foreach
static const vc s_gensym247 = vc(VC_BSTRING, "\x66\x6f\x72\x65\x61\x63\x68", 7);
#define gensym247() s_gensym247
static void gensym249(vc var, vc item) {
    Vcmap->local_add(var, item);
    gensym246();
}
static vc gensym248() {
    vc ret;
    vc var = gensym216();
    vc set = gensym217();
    try {
        set.foreach(var, gensym249);
    } catch (VcBreak& b)
    {
        --b.lev;
        if(b.lev > 0) throw;
    }
    return ret;
}
static vc gensym250() {
    static vc var = vc(VC_BSTRING, "\x61\x6c\x6c", 3);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym251() {
    static vc var = vc(VC_BSTRING, "\x42", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// eq
static const vc s_gensym252 = vc(VC_BSTRING, "\x65\x71", 2);
#define gensym252() s_gensym252
static vc gensym253() {
    vc ret = eqfun(gensym250(),gensym251());
    return ret;
}
static const vc s_gensym254(1);
#define gensym254() s_gensym254
// break
static const vc s_gensym255 = vc(VC_BSTRING, "\x62\x72\x65\x61\x6b", 5);
#define gensym255() s_gensym255
static vc gensym256() {
    vc ret;
    vc lev = gensym254();
    throw VcBreak((int)lev);
    return vcnil;
    return ret;
}
// if
static const vc s_gensym257 = vc(VC_BSTRING, "\x69\x66", 2);
#define gensym257() s_gensym257
static vc gensym258() {
    vc ret;
    if(!gensym253().is_nil()) {
        ret = gensym256();
    }
    return ret;
}
// iter
static const vc s_gensym259 = vc(VC_BSTRING, "\x69\x74\x65\x72", 4);
#define gensym259() s_gensym259
static vc gensym260() {
    static vc var = vc(VC_BSTRING, "\x69\x74\x65\x72", 4);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym261(1);
#define gensym261() s_gensym261
// sub
static const vc s_gensym262 = vc(VC_BSTRING, "\x73\x75\x62", 3);
#define gensym262() s_gensym262
static vc gensym263() {
    vc ret = gensym260() - gensym261();
    return ret;
}
// lbind
static const vc s_gensym264 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym264() s_gensym264
static vc gensym265() {
    vc ret;
    Vcmap->local_add(gensym259(), gensym263());
    return ret;
}
// prog
static const vc s_gensym266 = vc(VC_BSTRING, "\x70\x72\x6f\x67", 4);
#define gensym266() s_gensym266
static vc gensym267() {
    vc ret;
    gensym176();
    gensym186();
    gensym193();
    gensym200();
    gensym207();
    gensym211();
    gensym215();
    gensym248();
    gensym258();
    gensym265();
    return ret;
}
// while
static const vc s_gensym268 = vc(VC_BSTRING, "\x77\x68\x69\x6c\x65", 5);
#define gensym268() s_gensym268
static vc gensym269() {
    vc ret;
    try {
        vc cond = gensym163();
        while(!cond.is_nil()) {
            gensym267();
            cond = gensym163();
        }
    }
    catch(VcBreak& b) {
        --b.lev;
        if(b.lev > 0) {
            throw;
        }
    }
    return vcnil;
    return ret;
}
// b
static const vc s_gensym270 = vc(VC_BSTRING, "\x62", 1);
#define gensym270() s_gensym270
static vc gensym271() {
    static vc var = vc(VC_BSTRING, "\x6d\x73", 2);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// byte-acc
static const vc s_gensym272 = vc(VC_BSTRING, "\x62\x79\x74\x65\x2d\x61\x63\x63", 8);
#define gensym272() s_gensym272
static vc gensym273() {
    static vc var = vc(VC_BSTRING, "\x62\x79\x74\x65\x2d\x61\x63\x63", 8);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym274(1);
#define gensym274() s_gensym274
// blsl
static const vc s_gensym275 = vc(VC_BSTRING, "\x62\x6c\x73\x6c", 4);
#define gensym275() s_gensym275
static vc gensym276() {
    vc ret = blslfun(gensym273(),gensym274());
    return ret;
}
static vc gensym277() {
    static vc var = vc(VC_BSTRING, "\x62", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// bor
static const vc s_gensym278 = vc(VC_BSTRING, "\x62\x6f\x72", 3);
#define gensym278() s_gensym278
static vc gensym279() {
    vc ret = borfun(gensym276(),gensym277());
    return ret;
}
// lbind
static const vc s_gensym280 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym280() s_gensym280
static vc gensym281() {
    vc ret;
    Vcmap->local_add(gensym272(), gensym279());
    return ret;
}
// prog
static const vc s_gensym282 = vc(VC_BSTRING, "\x70\x72\x6f\x67", 4);
#define gensym282() s_gensym282
static vc gensym283() {
    vc ret;
    gensym281();
    return ret;
}
// foreach
static const vc s_gensym284 = vc(VC_BSTRING, "\x66\x6f\x72\x65\x61\x63\x68", 7);
#define gensym284() s_gensym284
static void gensym286(vc var, vc item) {
    Vcmap->local_add(var, item);
    gensym283();
}
static vc gensym285() {
    vc ret;
    vc var = gensym270();
    vc set = gensym271();
    try {
        set.foreach(var, gensym286);
    } catch (VcBreak& b)
    {
        --b.lev;
        if(b.lev > 0) throw;
    }
    return ret;
}
// bit-num
static const vc s_gensym287 = vc(VC_BSTRING, "\x62\x69\x74\x2d\x6e\x75\x6d", 7);
#define gensym287() s_gensym287
static vc gensym288() {
    static vc var = vc(VC_BSTRING, "\x62\x69\x74\x2d\x6e\x75\x6d", 7);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym289() {
    static vc var = vc(VC_BSTRING, "\x42", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// add
static const vc s_gensym290 = vc(VC_BSTRING, "\x61\x64\x64", 3);
#define gensym290() s_gensym290
static vc gensym291() {
    vc ret = gensym288() + gensym289();
    return ret;
}
// lbind
static const vc s_gensym292 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym292() s_gensym292
static vc gensym293() {
    vc ret;
    Vcmap->local_add(gensym287(), gensym291());
    return ret;
}
static vc gensym294() {
    static vc var = vc(VC_BSTRING, "\x62\x69\x74\x2d\x6e\x75\x6d", 7);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym295(8);
#define gensym295() s_gensym295
// eq
static const vc s_gensym296 = vc(VC_BSTRING, "\x65\x71", 2);
#define gensym296() s_gensym296
static vc gensym297() {
    vc ret = eqfun(gensym294(),gensym295());
    return ret;
}
static vc gensym298() {
    static vc var = vc(VC_BSTRING, "\x73\x74\x64\x6f\x75\x74", 6);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym299() {
    static vc var = vc(VC_BSTRING, "\x62\x79\x74\x65\x2d\x61\x63\x63", 8);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// string
static const vc s_gensym300 = vc(VC_BSTRING, "\x73\x74\x72\x69\x6e\x67", 6);
#define gensym300() s_gensym300
static vc gensym301() {
    VCArglist a;
    a.set_size(1);
    a.append(gensym299());
    vc ret = dostring(&a);
    return ret;
}
// fputs
static const vc s_gensym302 = vc(VC_BSTRING, "\x66\x70\x75\x74\x73", 5);
#define gensym302() s_gensym302
static vc gensym303() {
    vc ret = dofputs(gensym298(),gensym301());
    return ret;
}
// byte-acc
static const vc s_gensym304 = vc(VC_BSTRING, "\x62\x79\x74\x65\x2d\x61\x63\x63", 8);
#define gensym304() s_gensym304
static const vc s_gensym305(0);
#define gensym305() s_gensym305
// lbind
static const vc s_gensym306 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym306() s_gensym306
static vc gensym307() {
    vc ret;
    Vcmap->local_add(gensym304(), gensym305());
    return ret;
}
// bit-num
static const vc s_gensym308 = vc(VC_BSTRING, "\x62\x69\x74\x2d\x6e\x75\x6d", 7);
#define gensym308() s_gensym308
static const vc s_gensym309(0);
#define gensym309() s_gensym309
// lbind
static const vc s_gensym310 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym310() s_gensym310
static vc gensym311() {
    vc ret;
    Vcmap->local_add(gensym308(), gensym309());
    return ret;
}
// prog
static const vc s_gensym312 = vc(VC_BSTRING, "\x70\x72\x6f\x67", 4);
#define gensym312() s_gensym312
static vc gensym313() {
    vc ret;
    gensym303();
    gensym307();
    gensym311();
    return ret;
}
// prog
static const vc s_gensym314 = vc(VC_BSTRING, "\x70\x72\x6f\x67", 4);
#define gensym314() s_gensym314
static vc gensym315() {
    vc ret;
    return ret;
}
// if
static const vc s_gensym316 = vc(VC_BSTRING, "\x69\x66", 2);
#define gensym316() s_gensym316
static vc gensym317() {
    vc ret;
    if(!gensym297().is_nil()) {
        ret = gensym313();
    }
    else {
        ret = gensym315();
    }
    return ret;
}
// c
static const vc s_gensym318 = vc(VC_BSTRING, "\x63", 1);
#define gensym318() s_gensym318
static vc gensym319() {
    static vc var = vc(VC_BSTRING, "\x63", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym320() {
    static vc var = vc(VC_BSTRING, "\x42", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// add
static const vc s_gensym321 = vc(VC_BSTRING, "\x61\x64\x64", 3);
#define gensym321() s_gensym321
static vc gensym322() {
    vc ret = gensym319() + gensym320();
    return ret;
}
// lbind
static const vc s_gensym323 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym323() s_gensym323
static vc gensym324() {
    vc ret;
    Vcmap->local_add(gensym318(), gensym322());
    return ret;
}
// prog
static const vc s_gensym325 = vc(VC_BSTRING, "\x70\x72\x6f\x67", 4);
#define gensym325() s_gensym325
static vc gensym326() {
    vc ret;
    gensym77();
    gensym84();
    gensym90();
    gensym96();
    gensym103();
    gensym110();
    gensym148();
    gensym155();
    gensym159();
    gensym269();
    gensym285();
    gensym293();
    gensym317();
    gensym324();
    return ret;
}
// while
static const vc s_gensym327 = vc(VC_BSTRING, "\x77\x68\x69\x6c\x65", 5);
#define gensym327() s_gensym327
static vc gensym328() {
    vc ret;
    try {
        vc cond = gensym70();
        while(!cond.is_nil()) {
            gensym326();
            cond = gensym70();
        }
    }
    catch(VcBreak& b) {
        --b.lev;
        if(b.lev > 0) {
            throw;
        }
    }
    return vcnil;
    return ret;
}
static vc gensym329() {
    static vc var = vc(VC_BSTRING, "\x4e", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym330(8);
#define gensym330() s_gensym330
// mod
static const vc s_gensym331 = vc(VC_BSTRING, "\x6d\x6f\x64", 3);
#define gensym331() s_gensym331
static vc gensym332() {
    vc ret = domod(gensym329(),gensym330());
    return ret;
}
static const vc s_gensym333(0);
#define gensym333() s_gensym333
// ne
static const vc s_gensym334 = vc(VC_BSTRING, "\x6e\x65", 2);
#define gensym334() s_gensym334
static vc gensym335() {
    vc ret = nefun(gensym332(),gensym333());
    return ret;
}
// byte-acc
static const vc s_gensym336 = vc(VC_BSTRING, "\x62\x79\x74\x65\x2d\x61\x63\x63", 8);
#define gensym336() s_gensym336
static vc gensym337() {
    static vc var = vc(VC_BSTRING, "\x62\x79\x74\x65\x2d\x61\x63\x63", 8);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym338(8);
#define gensym338() s_gensym338
static vc gensym339() {
    static vc var = vc(VC_BSTRING, "\x4e", 1);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static const vc s_gensym340(8);
#define gensym340() s_gensym340
// mod
static const vc s_gensym341 = vc(VC_BSTRING, "\x6d\x6f\x64", 3);
#define gensym341() s_gensym341
static vc gensym342() {
    vc ret = domod(gensym339(),gensym340());
    return ret;
}
// sub
static const vc s_gensym343 = vc(VC_BSTRING, "\x73\x75\x62", 3);
#define gensym343() s_gensym343
static vc gensym344() {
    vc ret = gensym338() - gensym342();
    return ret;
}
// blsl
static const vc s_gensym345 = vc(VC_BSTRING, "\x62\x6c\x73\x6c", 4);
#define gensym345() s_gensym345
static vc gensym346() {
    vc ret = blslfun(gensym337(),gensym344());
    return ret;
}
// lbind
static const vc s_gensym347 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym347() s_gensym347
static vc gensym348() {
    vc ret;
    Vcmap->local_add(gensym336(), gensym346());
    return ret;
}
static vc gensym349() {
    static vc var = vc(VC_BSTRING, "\x73\x74\x64\x6f\x75\x74", 6);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
static vc gensym350() {
    static vc var = vc(VC_BSTRING, "\x62\x79\x74\x65\x2d\x61\x63\x63", 8);
    static vc *range;
    static unsigned long cached_when;
    if(range && cached_when >= vc_cvar::Lookup_cache_counter) {
        return *range;
    }
    cached_when = vc_cvar::Lookup_cache_counter;
    return Vcmap->get2(var, range);
}
// string
static const vc s_gensym351 = vc(VC_BSTRING, "\x73\x74\x72\x69\x6e\x67", 6);
#define gensym351() s_gensym351
static vc gensym352() {
    VCArglist a;
    a.set_size(1);
    a.append(gensym350());
    vc ret = dostring(&a);
    return ret;
}
// fputs
static const vc s_gensym353 = vc(VC_BSTRING, "\x66\x70\x75\x74\x73", 5);
#define gensym353() s_gensym353
static vc gensym354() {
    vc ret = dofputs(gensym349(),gensym352());
    return ret;
}
// byte-acc
static const vc s_gensym355 = vc(VC_BSTRING, "\x62\x79\x74\x65\x2d\x61\x63\x63", 8);
#define gensym355() s_gensym355
static const vc s_gensym356(0);
#define gensym356() s_gensym356
// lbind
static const vc s_gensym357 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym357() s_gensym357
static vc gensym358() {
    vc ret;
    Vcmap->local_add(gensym355(), gensym356());
    return ret;
}
// bit-num
static const vc s_gensym359 = vc(VC_BSTRING, "\x62\x69\x74\x2d\x6e\x75\x6d", 7);
#define gensym359() s_gensym359
static const vc s_gensym360(0);
#define gensym360() s_gensym360
// lbind
static const vc s_gensym361 = vc(VC_BSTRING, "\x6c\x62\x69\x6e\x64", 5);
#define gensym361() s_gensym361
static vc gensym362() {
    vc ret;
    Vcmap->local_add(gensym359(), gensym360());
    return ret;
}
// prog
static const vc s_gensym363 = vc(VC_BSTRING, "\x70\x72\x6f\x67", 4);
#define gensym363() s_gensym363
static vc gensym364() {
    vc ret;
    gensym348();
    gensym354();
    gensym358();
    gensym362();
    return ret;
}
// if
static const vc s_gensym365 = vc(VC_BSTRING, "\x69\x66", 2);
#define gensym365() s_gensym365
static vc gensym366() {
    vc ret;
    if(!gensym335().is_nil()) {
        ret = gensym364();
    }
    return ret;
}
// prog
static const vc s_gensym367 = vc(VC_BSTRING, "\x70\x72\x6f\x67", 4);
#define gensym367() s_gensym367
static vc gensym368() {
    vc ret;
    gensym58();
    gensym62();
    gensym66();
    gensym328();
    gensym366();
    return ret;
}
// for
static const vc s_gensym369 = vc(VC_BSTRING, "\x66\x6f\x72", 3);
#define gensym369() s_gensym369
static vc gensym370() {
    vc ret;
    long l = gensym53();
    long h = gensym54();
    vc var = gensym52();
    try {
        long i = l;
        for(; i <= h; ++i) {
            Vcmap->local_add(var, vc(i));
            gensym368();
        }
    }
    catch(VcBreak& b) {
        --b.lev;
        if(b.lev > 0) throw;
    }
    return vcnil;
    return ret;
}
static vc real_gensym371() {
    gensym30();
    gensym34();
    gensym41();
    gensym51();
    vc ret = gensym370();
    return ret;
}
static vc gensym371() {
    return (long)real_gensym371;
}
// compile
static const vc s_gensym372 = vc(VC_BSTRING, "\x63\x6f\x6d\x70\x69\x6c\x65", 7);
#define gensym372() s_gensym372
static vc gensym373() {
    vc ret;
    vc fnm = gensym23();
    VCArglist na;
    na.set_size(0);
    vc fdef = gensym371();
    vc_trans_fundef *f = new vc_trans_fundef(fnm, &na, fdef, VC_FUNC_NORMAL);
    vc v;
    v.redefine(f);
    fnm.bind(v);
    return ret;
}
// mandel
static const vc s_gensym374 = vc(VC_BSTRING, "\x6d\x61\x6e\x64\x65\x6c", 6);
#define gensym374() s_gensym374
static vc gensym375() {
    vc ret;
    VCArglist a;
    a.set_size(0);
    vc fobj = gensym374();
    ret = fobj(&a);
    return ret;
}
static vc real_gensym376() {
    gensym3();
    gensym7();
    gensym11();
    gensym15();
    gensym22();
    gensym373();
    vc ret = gensym375();
    return ret;
}
static vc gensym376() {
    return (long)real_gensym376;
}
vc top() {
    return gensym376();
}
