%module dwybg
%inline %{
int dwyco_background_processing(int port, int exit_if_outq_empty, const char *sys_pfx, const char *user_pfx, const char *tmp_pfx, const char *token);
void dwyco_set_aux_string(const char *str);
void dwyco_clear_contact_list();
int dwyco_add_contact(const char *name, const char *phone, const char *email);
void dwyco_signal_msg_cond();
void dwyco_wait_msg_cond(int ms);
void dwyco_write_token(const char *token);
%}
