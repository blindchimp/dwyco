#include "ezset.h"
#include "simplesql.h"
#include "simple_property.h"
#include "dwtree2.h"
#include "aconn.h"

using namespace dwyco;
#undef DWUIDECLVAL
#define DWUIDECLVAL(tp, nm, ival_str, ival_int) { tp, #nm, ival_str, ival_int}

struct init_settings
{
	enum vc_type tp;
	const char *name;
    const char *init_val_str;
    int init_val_int;
};

static init_settings Initial_settings[] =
{
    DWUIDECLVAL(VC_INT, net/primary_port, "", 6780),
    DWUIDECLVAL(VC_INT, net/secondary_port, "", 6781),
    DWUIDECLVAL(VC_INT, net/pal_port, "", 6782),
    DWUIDECLVAL(VC_INT, net/nat_primary_port, "", 6780),
    DWUIDECLVAL(VC_INT, net/nat_secondary_port, "", 6781),
    DWUIDECLVAL(VC_INT, net/nat_pal_port, "", 6782),
    DWUIDECLVAL(VC_INT, net/advertise_nat_ports, "", 0),
    DWUIDECLVAL(VC_INT, net/disable_upnp, "", 0),
    DWUIDECLVAL(VC_INT, net/call_setup_media_select, "", CSMS_TCP_ONLY),
    DWUIDECLVAL(VC_INT, net/listen, "", 1),
    DWUIDECLVAL(VC_INT, call_acceptance/max_audio, "", 4),
    DWUIDECLVAL(VC_INT, call_acceptance/max_chat, "", 4),
    DWUIDECLVAL(VC_INT, call_acceptance/max_video, "", 4),
    DWUIDECLVAL(VC_INT, call_acceptance/max_audio_recv, "", 4),
    DWUIDECLVAL(VC_INT, call_acceptance/max_video_recv, "", 4),
    DWUIDECLVAL(VC_INT, call_acceptance/max_pchat, "", 4),
    DWUIDECLVAL(VC_BSTRING, call_acceptance/pw, "", 0),
    DWUIDECLVAL(VC_INT, call_acceptance/auto_accept, "", 1),
    DWUIDECLVAL(VC_INT, call_acceptance/require_pw, "", 0),
    DWUIDECLVAL(VC_BSTRING, raw_files/raw_files_list, "", 0),
    DWUIDECLVAL(VC_BSTRING, raw_files/raw_files_pattern, "", 0),
    DWUIDECLVAL(VC_INT, raw_files/use_list_of_files, "", 0),
    DWUIDECLVAL(VC_INT, raw_files/use_pattern, "", 0),
    DWUIDECLVAL(VC_INT, raw_files/preload, "", 0),
    DWUIDECLVAL(VC_BSTRING, user/description, "", 0),
    DWUIDECLVAL(VC_BSTRING, user/username, "", 0),
    DWUIDECLVAL(VC_BSTRING, user/email, "", 0),
    DWUIDECLVAL(VC_BSTRING, user/location, "", 0),
    DWUIDECLVAL(VC_BSTRING, vid_input/device_name, "", 0),
    //DWUIDECLVAL(VC_INT, video_input/coded, "", 0),
    //DWUIDECLVAL(VC_INT, video_input/raw, "", 0),
    // "raw", or "camera" for now
    DWUIDECLVAL(VC_BSTRING, video_input/source, "", 0),
    DWUIDECLVAL(VC_INT, video_input/no_video, "", 1),
    DWUIDECLVAL(VC_INT, video_input/device_index, "", 0),
    DWUIDECLVAL(VC_INT, zap/always_server, "", 0),
    DWUIDECLVAL(VC_INT, zap/always_accept, "", 1),
    DWUIDECLVAL(VC_INT, zap/ignore, "", 0),
    DWUIDECLVAL(VC_INT, zap/use_old_timing, "", 0),
    DWUIDECLVAL(VC_INT, zap/save_sent, "", 1),
    DWUIDECLVAL(VC_INT, zap/no_forward_default, "", 0),
    DWUIDECLVAL(VC_INT, video_format/swap_rb, "", 0),

    DWUIDECLVAL(VC_INT, rate/kbits_per_sec_out, "", 128),
    DWUIDECLVAL(VC_INT, rate/kbits_per_sec_in, "", 128),
    DWUIDECLVAL(VC_INT, rate/max_fps, "", 20),
    {VC_NIL,0, 0, 0}
};

struct settings_sql : public SimpleSql
{
    settings_sql() : SimpleSql("set.sql") {}

    void init_schema(const DwString&) {
        sql_simple("create table if not exists settings ("
        "name text primary key not null, "
        "value not null "
		"on conflict replace"
                   ")");
    }


};

static settings_sql *Db;

#define sql Db->sql_simple

struct setting : public ssns::trackable
{
    vc name;
    sigprop<vc> value;

    void update_db(vc val) {
        sql("update settings set value = ?1 where name = ?2", val, name);
    }
};

static DwTreeKaz<setting *, vc> *Map;

void
init_sql_settings()
{
	if(Db)
		return;
    Db = new settings_sql;
    Db->init();
    Map = new DwTreeKaz<setting *, vc>(0);

    vc db = sql("select * from settings");
    for(int i = 0; i < db.num_elems(); ++i)
    {
        setting *s = new setting;
        s->name = db[i][0];
        s->value = db[i][1];
        s->value.value_changed.connect_memfun(s, &setting::update_db);
        Map->add(s->name, s);
    }
    Db->start_transaction();
    for(int i = 0; Initial_settings[i].name != 0; ++i)
    {
        if(Map->contains(Initial_settings[i].name))
            continue;
        auto is = &Initial_settings[i];
        sql("insert into settings (name, value) values(?1, ?2)", is->name, is->tp == VC_INT ? vc(is->init_val_int) : vc(is->init_val_str));
        setting *s = new setting;
        s->name = is->name;

        Map->add(s->name, s);
        switch(is->tp)
        {
        case VC_INT:
            s->value = is->init_val_int;
            break;
        case VC_BSTRING:
            s->value = is->init_val_str;
            break;
        default:
            oopanic("bad init setting table");

        }
        s->value.value_changed.connect_memfun(s, &setting::update_db);
    }
    Db->commit_transaction();
}

void
exit_sql_settings()
{
    if(!Db)
        return;
    delete Map;
    Db->exit();
    delete Db;
    Map = 0;
    Db = 0;
}

void
bind_sql_setting(vc name, void (*fn)(vc))
{
    setting *s;
    if(!Map->find(name, s))
        oopanic("bad setting");
    s->value.value_changed.connect_ptrfun(fn);

}

int
set_settings_value(const char *name, const char  *val)
{
    setting *s;
    if(!Map->find(name, s))
        oopanic("bad setting");
    vc tmp = s->value;
    switch(tmp.type())
    {
    case VC_INT:
        s->value = atol(val);
        break;
    case VC_STRING:
        s->value = val;
        break;
    default:
        oopanic("bad setting type");
    }
    return 1;
}

int
set_settings_value(const char *name, int val)
{
    setting *s;
    if(!Map->find(name, s))
        oopanic("bad setting");
    s->value = val;
    return 1;
}

int
set_settings_value(const char *name, vc val)
{
    setting *s;
    if(!Map->find(name, s))
        oopanic("bad setting");
    s->value = val;
    return 1;
}

vc
get_settings_value(const char *name)
{
    setting *s;
    if(!Map->find(name, s))
        oopanic("bad setting");
    return s->value;
}

