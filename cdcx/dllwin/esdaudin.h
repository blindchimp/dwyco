#ifndef ESDAUDIN_H
void DWYCOCALLCONV esd_new(void *, int buflen, int nbufs);
void DWYCOCALLCONV esd_delete(void *);
int DWYCOCALLCONV esd_init(void *);
int DWYCOCALLCONV esd_has_data(void *);
void DWYCOCALLCONV esd_need(void *);
void DWYCOCALLCONV esd_pass(void *);
void DWYCOCALLCONV esd_stop(void *);
void DWYCOCALLCONV esd_on(void *);
void DWYCOCALLCONV esd_off(void *);
void DWYCOCALLCONV esd_reset(void *);
void DWYCOCALLCONV esd_need(void *);
int DWYCOCALLCONV esd_status(void *);
void * DWYCOCALLCONV esd_get_data(void *, int *r, int *c);

#endif
