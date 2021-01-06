#include "ezset.h"
#include "simplesql.h"
#include "simple_property.h"
#include "dwtree2.h"

using namespace dwyco;


aconn.h:    DWUIDECLVAL(int, net/primary_port)
aconn.h:    DWUIDECLVAL(int, net/secondary_port)
aconn.h:    DWUIDECLVAL(int, net/pal_port)
aconn.h:    DWUIDECLVAL(int, net/nat_primary_port)
aconn.h:    DWUIDECLVAL(int, net/nat_secondary_port)
aconn.h:    DWUIDECLVAL(int, net/nat_pal_port)
aconn.h:    DWUIDECLVAL(bool, net/advertise_nat_ports)
aconn.h:    DWUIDECLVAL(int, net/disable_upnp)
aconn.h:    DWUIDECLVAL(int, net/call_setup_media_select)
aconn.h:    DWUIDECLVAL(int, net/listen)
cllaccpt.h:    DWUIDECLVAL_char_int(const char *, call_acceptance/max_audio)
cllaccpt.h:    DWUIDECLVAL_char_int(const char *, call_acceptance/max_chat)
cllaccpt.h:    DWUIDECLVAL_char_int(const char *, call_acceptance/max_video)
cllaccpt.h:    DWUIDECLVAL_char_int(const char *, call_acceptance/max_audio_recv)
cllaccpt.h:    DWUIDECLVAL_char_int(const char *, call_acceptance/max_video_recv)
cllaccpt.h:    DWUIDECLVAL_char_int(const char *, call_acceptance/max_pchat)
cllaccpt.h:    DWUIDECLVAL(const char *, call_acceptance/pw)
cllaccpt.h:    DWUIDECLVAL(bool, call_acceptance/auto_accept)
cllaccpt.h:    DWUIDECLVAL(bool, call_acceptance/require_pw)
rawfiles.h:    DWUIDECLVAL(const char *, raw_files/raw_files_list)
rawfiles.h:    DWUIDECLVAL(const char *, raw_files/raw_files_pattern)
rawfiles.h:    DWUIDECLVAL(bool, raw_files/use_list_of_files)
rawfiles.h:    DWUIDECLVAL(bool, raw_files/use_pattern)
rawfiles.h:    DWUIDECLVAL(bool, raw_files/preload)
usercnfg.h:    DWUIDECLVAL(const char *, user/description)
usercnfg.h:    DWUIDECLVAL(const char *, user/username)
usercnfg.h:    DWUIDECLVAL(const char *, user/email)
usercnfg.h:    DWUIDECLVAL(const char *, user/location)
vidinput.h:    DWUIDECLVAL(const char *, vid_input/device_name)
vidinput.h:    DWUIDECLVAL(bool, vid_input/coded)
vidinput.h:    DWUIDECLVAL(bool, vid_input/raw)
vidinput.h:    DWUIDECLVAL(bool, vid_input/vfw)
vidinput.h:    DWUIDECLVAL(bool, vid_input/no_video)
vidinput.h:    DWUIDECLVAL(int, vid_input/device_index)
zapadv.h:    DWUIDECLVAL(bool, zap/always_server)
zapadv.h:    DWUIDECLVAL(bool, zap/always_accept)
zapadv.h:    DWUIDECLVAL(bool, zap/ignore)
zapadv.h:    DWUIDECLVAL(bool, zap/use_old_timing)
zapadv.h:    DWUIDECLVAL(bool, zap/save_sent)
zapadv.h:    DWUIDECLVAL(bool, zap/no_forward_default)

struct settings_sql : public SimpleSql
{
    settings_sql() : SimpleSql("set.sql") {}

    void init_schema(const DwString&) {
        sql_simple("create table if not exists settings ("
		"name text not null primary key, "
		"value not null, "
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
        sql("update set value = ?1 where name = ?2", val, name);
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
set_settings_value(const char *name, const char *value)
{
    setting *s;
    if(!Map->find(name, s))
        oopanic("bad setting");
    s->value = vc(value);
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

