
#include "vc.h"
#include <curses.h>
#include "hacked_cvt_curses.xml.cpp"

#if 0
	static vc
wrap_trace(VCArglist *a)
{
	trace(cvt_vc__unsigned_int((*a)[0]));
return(vcnil);

}
static vc
wrap__tracemouse(VCArglist *a)
{
	return cvt_p_char(_tracemouse(cvt_vc_pq_MEVENT_((*a)[0])));

}
static vc
wrap__tracecchar_t2(VCArglist *a)
{
	return cvt_p_char(_tracecchar_t2(cvt_vc__int((*a)[0]),
cvt_vc_pq_cchar_t_((*a)[1])));

}
static vc
wrap__tracecchar_t(VCArglist *a)
{
	return cvt_p_char(_tracecchar_t(cvt_vc_pq_cchar_t_((*a)[0])));

}
static vc
wrap__tracechtype2(VCArglist *a)
{
	return cvt_p_char(_tracechtype2(cvt_vc__int((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));

}
static vc
wrap__tracechtype(VCArglist *a)
{
	return cvt_p_char(_tracechtype(cvt_vc__long_unsigned_int((*a)[0])));

}
static vc
wrap__tracechar(VCArglist *a)
{
	return cvt_p_char(_tracechar(cvt_vc__int((*a)[0])));

}
static vc
wrap__nc_tracebits(VCArglist *a)
{
	return cvt_p_char(_nc_tracebits());

}
static vc
wrap__traceattr2(VCArglist *a)
{
	return cvt_p_char(_traceattr2(cvt_vc__int((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));

}
static vc
wrap__traceattr(VCArglist *a)
{
	return cvt_p_char(_traceattr(cvt_vc__long_unsigned_int((*a)[0])));

}
static vc
wrap__tracedump(VCArglist *a)
{
	_tracedump(cvt_vc_pq_char((*a)[0]),
cvt_vc_p__win_st_((*a)[1]));
return(vcnil);

}
static vc
wrap__tracef(VCArglist *a)
{
	_tracef(cvt_vc_pq_char((*a)[0]),
);
return(vcnil);

}
#endif
static vc
wrap_has_key(VCArglist *a)
{
	return cvt__int(has_key(cvt_vc__int((*a)[0])));

}
static vc
wrap_mcprint(VCArglist *a)
{
	return cvt__int(mcprint(cvt_vc_p_char((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_mouse_trafo(VCArglist *a)
{
	return cvt__bool(mouse_trafo(cvt_vc_p_int((*a)[0]),
cvt_vc_p_int((*a)[1]),
cvt_vc__bool((*a)[2])));

}
static vc
wrap_wmouse_trafo(VCArglist *a)
{
	return cvt__bool(wmouse_trafo(cvt_vc_pq__win_st_((*a)[0]),
cvt_vc_p_int((*a)[1]),
cvt_vc_p_int((*a)[2]),
cvt_vc__bool((*a)[3])));

}
static vc
wrap_mouseinterval(VCArglist *a)
{
	return cvt__int(mouseinterval(cvt_vc__int((*a)[0])));

}
static vc
wrap_wenclose(VCArglist *a)
{
	return cvt__bool(wenclose(cvt_vc_pq__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_mousemask(VCArglist *a)
{
	return cvt__long_unsigned_int(mousemask(cvt_vc__long_unsigned_int((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));

}
static vc
wrap_ungetmouse(VCArglist *a)
{
	return cvt__int(ungetmouse(cvt_vc_p_MEVENT_((*a)[0])));

}
static vc
wrap_getmouse(VCArglist *a)
{
	MEVENT m;
	int ret = getmouse(&m);
	vc vm(VC_VECTOR);
	vm[0] = m.id;
	vm[1] = m.x;
	vm[2] = m.y;
	vm[3] = m.z;
	vm[4] = (int)m.bstate;
	(*a)[0].local_bind(vm);
	return ret;
}

static vc
wrap_wvline(VCArglist *a)
{
	return cvt__int(wvline(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_wtouchln(VCArglist *a)
{
	return cvt__int(wtouchln(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3])));

}
static vc
wrap_wtimeout(VCArglist *a)
{
	wtimeout(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]));
return(vcnil);

}
static vc
wrap_wsyncup(VCArglist *a)
{
	wsyncup(cvt_vc_p__win_st_((*a)[0]));
return(vcnil);

}
static vc
wrap_wsyncdown(VCArglist *a)
{
	wsyncdown(cvt_vc_p__win_st_((*a)[0]));
return(vcnil);

}
static vc
wrap_wstandend(VCArglist *a)
{
	return cvt__int(wstandend(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_wstandout(VCArglist *a)
{
	return cvt__int(wstandout(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_wsetscrreg(VCArglist *a)
{
	return cvt__int(wsetscrreg(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_wscrl(VCArglist *a)
{
	return cvt__int(wscrl(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1])));

}
#if 0
static vc
wrap_wscanw(VCArglist *a)
{
	return cvt__int(wscanw(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_p_char((*a)[1]),
));

}
#endif
static vc
wrap_wrefresh(VCArglist *a)
{
	return cvt__int(wrefresh(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_wredrawln(VCArglist *a)
{
	return cvt__int(wredrawln(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));

}
#if 0
static vc
wrap_wprintw(VCArglist *a)
{
	return cvt__int(wprintw(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_pq_char((*a)[1]),
));

}
#endif
static vc
wrap_wnoutrefresh(VCArglist *a)
{
	return cvt__int(wnoutrefresh(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_wmove(VCArglist *a)
{
	return cvt__int(wmove(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_winstr(VCArglist *a)
{
	return cvt__int(winstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_p_char((*a)[1])));

}
static vc
wrap_winsstr(VCArglist *a)
{
	return cvt__int(winsstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_pq_char((*a)[1])));

}
static vc
wrap_winsnstr(VCArglist *a)
{
	return cvt__int(winsnstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_winsertln(VCArglist *a)
{
	return cvt__int(winsertln(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_winsdelln(VCArglist *a)
{
	return cvt__int(winsdelln(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_winsch(VCArglist *a)
{
	return cvt__int(winsch(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));

}
static vc
wrap_winnstr(VCArglist *a)
{
	return cvt__int(winnstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_winchstr(VCArglist *a)
{
	return cvt__int(winchstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1])));

}
static vc
wrap_winchnstr(VCArglist *a)
{
	return cvt__int(winchnstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_winch(VCArglist *a)
{
	return cvt__long_unsigned_int(winch(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_whline(VCArglist *a)
{
	return cvt__int(whline(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_wgetstr(VCArglist *a)
{
	return cvt__int(wgetstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_p_char((*a)[1])));

}
static vc
wrap_wgetnstr(VCArglist *a)
{
	return cvt__int(wgetnstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_wgetch(VCArglist *a)
{
	return cvt__int(wgetch(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_werase(VCArglist *a)
{
	return cvt__int(werase(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_wechochar(VCArglist *a)
{
	return cvt__int(wechochar(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));

}
static vc
wrap_wdeleteln(VCArglist *a)
{
	return cvt__int(wdeleteln(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_wdelch(VCArglist *a)
{
	return cvt__int(wdelch(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_wcursyncup(VCArglist *a)
{
	wcursyncup(cvt_vc_p__win_st_((*a)[0]));
return(vcnil);

}
static vc
wrap_wcolor_set(VCArglist *a)
{
	return cvt__int(wcolor_set(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__short_int((*a)[1]),
cvt_vc_p_void((*a)[2])));

}
static vc
wrap_wclrtoeol(VCArglist *a)
{
	return cvt__int(wclrtoeol(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_wclrtobot(VCArglist *a)
{
	return cvt__int(wclrtobot(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_wclear(VCArglist *a)
{
	return cvt__int(wclear(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_wchgat(VCArglist *a)
{
	return cvt__int(wchgat(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2]),
cvt_vc__short_int((*a)[3]),
cvt_vc_pq_void((*a)[4])));

}
static vc
wrap_wborder(VCArglist *a)
{
	return cvt__int(wborder(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2]),
cvt_vc__long_unsigned_int((*a)[3]),
cvt_vc__long_unsigned_int((*a)[4]),
cvt_vc__long_unsigned_int((*a)[5]),
cvt_vc__long_unsigned_int((*a)[6]),
cvt_vc__long_unsigned_int((*a)[7]),
cvt_vc__long_unsigned_int((*a)[8])));

}
static vc
wrap_wbkgdset(VCArglist *a)
{
	wbkgdset(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]));
return(vcnil);

}
static vc
wrap_wbkgd(VCArglist *a)
{
	return cvt__int(wbkgd(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));

}
static vc
wrap_wattr_set(VCArglist *a)
{
	return cvt__int(wattr_set(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]),
cvt_vc__short_int((*a)[2]),
cvt_vc_p_void((*a)[3])));

}
static vc
wrap_wattr_off(VCArglist *a)
{
	return cvt__int(wattr_off(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]),
cvt_vc_p_void((*a)[2])));

}
static vc
wrap_wattr_on(VCArglist *a)
{
	return cvt__int(wattr_on(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]),
cvt_vc_p_void((*a)[2])));

}
static vc
wrap_wattr_get(VCArglist *a)
{
	return cvt__int(wattr_get(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_p_long_unsigned_int((*a)[1]),
cvt_vc_p_short_int((*a)[2]),
cvt_vc_p_void((*a)[3])));

}
static vc
wrap_wattrset(VCArglist *a)
{
	return cvt__int(wattrset(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_wattroff(VCArglist *a)
{
	return cvt__int(wattroff(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_wattron(VCArglist *a)
{
	return cvt__int(wattron(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_waddstr(VCArglist *a)
{
	return cvt__int(waddstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_pq_char((*a)[1])));

}
static vc
wrap_waddnstr(VCArglist *a)
{
	return cvt__int(waddnstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_waddchstr(VCArglist *a)
{
	return cvt__int(waddchstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_pq_long_unsigned_int((*a)[1])));

}
static vc
wrap_waddchnstr(VCArglist *a)
{
	return cvt__int(waddchnstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_pq_long_unsigned_int((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_waddch(VCArglist *a)
{
	return cvt__int(waddch(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));

}
#if 0
static vc
wrap_vw_scanw(VCArglist *a)
{
	return cvt__int(vw_scanw(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc_p___va_list_tag_((*a)[2])));

}
static vc
wrap_vwscanw(VCArglist *a)
{
	return cvt__int(vwscanw(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_p_char((*a)[1]),
cvt_vc_p___va_list_tag_((*a)[2])));

}
static vc
wrap_vw_printw(VCArglist *a)
{
	return cvt__int(vw_printw(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc_p___va_list_tag_((*a)[2])));

}
static vc
wrap_vwprintw(VCArglist *a)
{
	return cvt__int(vwprintw(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc_p___va_list_tag_((*a)[2])));

}
#endif
static vc
wrap_vline(VCArglist *a)
{
	return cvt__int(vline(cvt_vc__long_unsigned_int((*a)[0]),
cvt_vc__int((*a)[1])));

}
#if 0
static vc
wrap_vidputs(VCArglist *a)
{
	return cvt__int(vidputs(cvt_vc__long_unsigned_int((*a)[0]),
cvt_vc_pF__int__int((*a)[1])));

}
#endif
static vc
wrap_vidattr(VCArglist *a)
{
	return cvt__int(vidattr(cvt_vc__long_unsigned_int((*a)[0])));

}
static vc
wrap_use_env(VCArglist *a)
{
	use_env(cvt_vc__bool((*a)[0]));
return(vcnil);

}
static vc
wrap_untouchwin(VCArglist *a)
{
	return cvt__int(untouchwin(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_ungetch(VCArglist *a)
{
	return cvt__int(ungetch(cvt_vc__int((*a)[0])));

}
static vc
wrap_typeahead(VCArglist *a)
{
	return cvt__int(typeahead(cvt_vc__int((*a)[0])));

}
#if 0
static vc
wrap_tparm(VCArglist *a)
{
	return cvt_p_char(tparm(cvt_vc_p_char((*a)[0]),
));

}
#endif
static vc
wrap_touchwin(VCArglist *a)
{
	return cvt__int(touchwin(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_touchline(VCArglist *a)
{
	return cvt__int(touchline(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_timeout(VCArglist *a)
{
	timeout(cvt_vc__int((*a)[0]));
return(vcnil);

}
static vc
wrap_tigetstr(VCArglist *a)
{
	return cvt_p_char(tigetstr(cvt_vc_p_char((*a)[0])));

}
static vc
wrap_tigetnum(VCArglist *a)
{
	return cvt__int(tigetnum(cvt_vc_p_char((*a)[0])));

}
static vc
wrap_tigetflag(VCArglist *a)
{
	return cvt__int(tigetflag(cvt_vc_p_char((*a)[0])));

}
static vc
wrap_termname(VCArglist *a)
{
	return cvt_p_char(termname());

}
static vc
wrap_termattrs(VCArglist *a)
{
	return cvt__long_unsigned_int(termattrs());

}
static vc
wrap_syncok(VCArglist *a)
{
	return cvt__int(syncok(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__bool((*a)[1])));

}
static vc
wrap_subwin(VCArglist *a)
{
	return cvt_p__win_st_(subwin(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4])));

}
static vc
wrap_subpad(VCArglist *a)
{
	return cvt_p__win_st_(subpad(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4])));

}
static vc
wrap_start_color(VCArglist *a)
{
	return cvt__int(start_color());

}
static vc
wrap_standend(VCArglist *a)
{
	return cvt__int(standend());

}
static vc
wrap_standout(VCArglist *a)
{
	return cvt__int(standout());

}
static vc
wrap_slk_touch(VCArglist *a)
{
	return cvt__int(slk_touch());

}
static vc
wrap_slk_set(VCArglist *a)
{
	return cvt__int(slk_set(cvt_vc__int((*a)[0]),
cvt_vc_pq_char((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_slk_restore(VCArglist *a)
{
	return cvt__int(slk_restore());

}
static vc
wrap_slk_refresh(VCArglist *a)
{
	return cvt__int(slk_refresh());

}
static vc
wrap_slk_noutrefresh(VCArglist *a)
{
	return cvt__int(slk_noutrefresh());

}
static vc
wrap_slk_label(VCArglist *a)
{
	return cvt_p_char(slk_label(cvt_vc__int((*a)[0])));

}
static vc
wrap_slk_init(VCArglist *a)
{
	return cvt__int(slk_init(cvt_vc__int((*a)[0])));

}
static vc
wrap_slk_color(VCArglist *a)
{
	return cvt__int(slk_color(cvt_vc__short_int((*a)[0])));

}
static vc
wrap_slk_clear(VCArglist *a)
{
	return cvt__int(slk_clear());

}
static vc
wrap_slk_attr_set(VCArglist *a)
{
	return cvt__int(slk_attr_set(cvt_vc__long_unsigned_int((*a)[0]),
cvt_vc__short_int((*a)[1]),
cvt_vc_p_void((*a)[2])));

}
static vc
wrap_slk_attr(VCArglist *a)
{
	return cvt__long_unsigned_int(slk_attr());

}
static vc
wrap_slk_attrset(VCArglist *a)
{
	return cvt__int(slk_attrset(cvt_vc__long_unsigned_int((*a)[0])));

}
static vc
wrap_slk_attr_on(VCArglist *a)
{
	return cvt__int(slk_attr_on(cvt_vc__long_unsigned_int((*a)[0]),
cvt_vc_p_void((*a)[1])));

}
static vc
wrap_slk_attron(VCArglist *a)
{
	return cvt__int(slk_attron(cvt_vc__long_unsigned_int((*a)[0])));

}
static vc
wrap_slk_attr_off(VCArglist *a)
{
	return cvt__int(slk_attr_off(cvt_vc__long_unsigned_int((*a)[0]),
cvt_vc_p_void((*a)[1])));

}
static vc
wrap_slk_attroff(VCArglist *a)
{
	return cvt__int(slk_attroff(cvt_vc__long_unsigned_int((*a)[0])));

}
static vc
wrap_set_term(VCArglist *a)
{
	return cvt_p_screen_(set_term(cvt_vc_p_screen_((*a)[0])));

}
static vc
wrap_setscrreg(VCArglist *a)
{
	return cvt__int(setscrreg(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_scr_set(VCArglist *a)
{
	return cvt__int(scr_set(cvt_vc_pq_char((*a)[0])));

}
static vc
wrap_scr_restore(VCArglist *a)
{
	return cvt__int(scr_restore(cvt_vc_pq_char((*a)[0])));

}
static vc
wrap_scrollok(VCArglist *a)
{
	return cvt__int(scrollok(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__bool((*a)[1])));

}
static vc
wrap_scroll(VCArglist *a)
{
	return cvt__int(scroll(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_scrl(VCArglist *a)
{
	return cvt__int(scrl(cvt_vc__int((*a)[0])));

}
static vc
wrap_scr_init(VCArglist *a)
{
	return cvt__int(scr_init(cvt_vc_pq_char((*a)[0])));

}
static vc
wrap_scr_dump(VCArglist *a)
{
	return cvt__int(scr_dump(cvt_vc_pq_char((*a)[0])));

}
#if 0
static vc
wrap_scanw(VCArglist *a)
{
	return cvt__int(scanw(cvt_vc_p_char((*a)[0]),
));

}
#endif
static vc
wrap_savetty(VCArglist *a)
{
	return cvt__int(savetty());

}
#if 0
static vc
wrap_ripoffline(VCArglist *a)
{
	return cvt__int(ripoffline(cvt_vc__int((*a)[0]),
cvt_vc_pF__int_p__win_st__x__int((*a)[1])));

}
#endif
static vc
wrap_reset_shell_mode(VCArglist *a)
{
	return cvt__int(reset_shell_mode());

}
static vc
wrap_reset_prog_mode(VCArglist *a)
{
	return cvt__int(reset_prog_mode());

}
static vc
wrap_resetty(VCArglist *a)
{
	return cvt__int(resetty());

}
static vc
wrap_refresh(VCArglist *a)
{
	return cvt__int(refresh());

}
static vc
wrap_redrawwin(VCArglist *a)
{
	return cvt__int(redrawwin(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_raw(VCArglist *a)
{
	return cvt__int(raw());

}
static vc
wrap_qiflush(VCArglist *a)
{
	qiflush();
return(vcnil);

}
#if 0
static vc
wrap_putwin(VCArglist *a)
{
	return cvt__int(putwin(cvt_vc_p__win_st_((*a)[0]),
cvt_vc_p__IO_FILE_((*a)[1])));

}
#endif
static vc
wrap_putp(VCArglist *a)
{
	return cvt__int(putp(cvt_vc_pq_char((*a)[0])));

}
#if 0
static vc
wrap_printw(VCArglist *a)
{
	return cvt__int(printw(cvt_vc_pq_char((*a)[0]),
));

}
#endif
static vc
wrap_prefresh(VCArglist *a)
{
	return cvt__int(prefresh(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc__int((*a)[6])));

}
static vc
wrap_pnoutrefresh(VCArglist *a)
{
	return cvt__int(pnoutrefresh(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc__int((*a)[6])));

}
static vc
wrap_pechochar(VCArglist *a)
{
	return cvt__int(pechochar(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1])));

}
static vc
wrap_PAIR_NUMBER(VCArglist *a)
{
	return cvt__int(PAIR_NUMBER(cvt_vc__int((*a)[0])));

}
static vc
wrap_pair_content(VCArglist *a)
{
	return cvt__int(pair_content(cvt_vc__short_int((*a)[0]),
cvt_vc_p_short_int((*a)[1]),
cvt_vc_p_short_int((*a)[2])));

}
static vc
wrap_overwrite(VCArglist *a)
{
	return cvt__int(overwrite(cvt_vc_pq__win_st_((*a)[0]),
cvt_vc_p__win_st_((*a)[1])));

}
static vc
wrap_overlay(VCArglist *a)
{
	return cvt__int(overlay(cvt_vc_pq__win_st_((*a)[0]),
cvt_vc_p__win_st_((*a)[1])));

}
static vc
wrap_notimeout(VCArglist *a)
{
	return cvt__int(notimeout(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__bool((*a)[1])));

}
static vc
wrap_noraw(VCArglist *a)
{
	return cvt__int(noraw());

}
static vc
wrap_noqiflush(VCArglist *a)
{
	noqiflush();
return(vcnil);

}
static vc
wrap_nonl(VCArglist *a)
{
	return cvt__int(nonl());

}
static vc
wrap_noecho(VCArglist *a)
{
	return cvt__int(noecho());

}
static vc
wrap_nodelay(VCArglist *a)
{
	return cvt__int(nodelay(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__bool((*a)[1])));

}
static vc
wrap_nocbreak(VCArglist *a)
{
	return cvt__int(nocbreak());

}
static vc
wrap_nl(VCArglist *a)
{
	return cvt__int(nl());

}
static vc
wrap_newwin(VCArglist *a)
{
	return cvt_p__win_st_(newwin(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3])));

}
#if 0
static vc
wrap_newterm(VCArglist *a)
{
	return cvt_p_screen_(newterm(cvt_vc_p_char((*a)[0]),
cvt_vc_p__IO_FILE_((*a)[1]),
cvt_vc_p__IO_FILE_((*a)[2])));

}
#endif
static vc
wrap_newpad(VCArglist *a)
{
	return cvt_p__win_st_(newpad(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_napms(VCArglist *a)
{
	return cvt__int(napms(cvt_vc__int((*a)[0])));

}
static vc
wrap_mvwvline(VCArglist *a)
{
	return cvt__int(mvwvline(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__long_unsigned_int((*a)[3]),
cvt_vc__int((*a)[4])));

}
#if 0
static vc
wrap_mvwscanw(VCArglist *a)
{
	return cvt__int(mvwscanw(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3]),
));

}
static vc
wrap_mvwprintw(VCArglist *a)
{
	return cvt__int(mvwprintw(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pq_char((*a)[3]),
));

}
#endif
static vc
wrap_mvwinstr(VCArglist *a)
{
	return cvt__int(mvwinstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3])));

}
static vc
wrap_mvwinsstr(VCArglist *a)
{
	return cvt__int(mvwinsstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pq_char((*a)[3])));

}
static vc
wrap_mvwinsnstr(VCArglist *a)
{
	return cvt__int(mvwinsnstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pq_char((*a)[3]),
cvt_vc__int((*a)[4])));

}
static vc
wrap_mvwinsch(VCArglist *a)
{
	return cvt__int(mvwinsch(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__long_unsigned_int((*a)[3])));

}
static vc
wrap_mvwinnstr(VCArglist *a)
{
	return cvt__int(mvwinnstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3]),
cvt_vc__int((*a)[4])));

}
static vc
wrap_mvwinchstr(VCArglist *a)
{
	return cvt__int(mvwinchstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_long_unsigned_int((*a)[3])));

}
static vc
wrap_mvwinchnstr(VCArglist *a)
{
	return cvt__int(mvwinchnstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_long_unsigned_int((*a)[3]),
cvt_vc__int((*a)[4])));

}
static vc
wrap_mvwinch(VCArglist *a)
{
	return cvt__long_unsigned_int(mvwinch(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_mvwin(VCArglist *a)
{
	return cvt__int(mvwin(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_mvwhline(VCArglist *a)
{
	return cvt__int(mvwhline(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__long_unsigned_int((*a)[3]),
cvt_vc__int((*a)[4])));

}
static vc
wrap_mvwgetstr(VCArglist *a)
{
	return cvt__int(mvwgetstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3])));

}
static vc
wrap_mvwgetnstr(VCArglist *a)
{
	return cvt__int(mvwgetnstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_p_char((*a)[3]),
cvt_vc__int((*a)[4])));

}
static vc
wrap_mvwgetch(VCArglist *a)
{
	return cvt__int(mvwgetch(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_mvwdelch(VCArglist *a)
{
	return cvt__int(mvwdelch(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_mvwchgat(VCArglist *a)
{
	return cvt__int(mvwchgat(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__long_unsigned_int((*a)[4]),
cvt_vc__short_int((*a)[5]),
cvt_vc_pq_void((*a)[6])));

}
static vc
wrap_mvwaddstr(VCArglist *a)
{
	return cvt__int(mvwaddstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pq_char((*a)[3])));

}
static vc
wrap_mvwaddnstr(VCArglist *a)
{
	return cvt__int(mvwaddnstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pq_char((*a)[3]),
cvt_vc__int((*a)[4])));

}
static vc
wrap_mvwaddchstr(VCArglist *a)
{
	return cvt__int(mvwaddchstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pq_long_unsigned_int((*a)[3])));

}
static vc
wrap_mvwaddchnstr(VCArglist *a)
{
	return cvt__int(mvwaddchnstr(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc_pq_long_unsigned_int((*a)[3]),
cvt_vc__int((*a)[4])));

}
static vc
wrap_mvwaddch(VCArglist *a)
{
	return cvt__int(mvwaddch(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__long_unsigned_int((*a)[3])));

}
static vc
wrap_mvvline(VCArglist *a)
{
	return cvt__int(mvvline(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2]),
cvt_vc__int((*a)[3])));

}
#if 0
static vc
wrap_mvscanw(VCArglist *a)
{
	return cvt__int(mvscanw(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_p_char((*a)[2]),
));

}
static vc
wrap_mvprintw(VCArglist *a)
{
	return cvt__int(mvprintw(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_char((*a)[2]),
));

}
#endif
static vc
wrap_mvinstr(VCArglist *a)
{
	return cvt__int(mvinstr(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_p_char((*a)[2])));

}
static vc
wrap_mvinsstr(VCArglist *a)
{
	return cvt__int(mvinsstr(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_char((*a)[2])));

}
static vc
wrap_mvinsnstr(VCArglist *a)
{
	return cvt__int(mvinsnstr(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__int((*a)[3])));

}
static vc
wrap_mvinsch(VCArglist *a)
{
	return cvt__int(mvinsch(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2])));

}
static vc
wrap_mvinnstr(VCArglist *a)
{
	return cvt__int(mvinnstr(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_p_char((*a)[2]),
cvt_vc__int((*a)[3])));

}
static vc
wrap_mvinchstr(VCArglist *a)
{
	return cvt__int(mvinchstr(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_p_long_unsigned_int((*a)[2])));

}
static vc
wrap_mvinchnstr(VCArglist *a)
{
	return cvt__int(mvinchnstr(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_p_long_unsigned_int((*a)[2]),
cvt_vc__int((*a)[3])));

}
static vc
wrap_mvinch(VCArglist *a)
{
	return cvt__long_unsigned_int(mvinch(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_mvhline(VCArglist *a)
{
	return cvt__int(mvhline(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2]),
cvt_vc__int((*a)[3])));

}
static vc
wrap_mvgetstr(VCArglist *a)
{
	return cvt__int(mvgetstr(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_p_char((*a)[2])));

}
static vc
wrap_mvgetnstr(VCArglist *a)
{
	return cvt__int(mvgetnstr(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_p_char((*a)[2]),
cvt_vc__int((*a)[3])));

}
static vc
wrap_mvgetch(VCArglist *a)
{
	return cvt__int(mvgetch(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_mvderwin(VCArglist *a)
{
	return cvt__int(mvderwin(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_mvdelch(VCArglist *a)
{
	return cvt__int(mvdelch(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_mvcur(VCArglist *a)
{
	return cvt__int(mvcur(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3])));

}
static vc
wrap_mvchgat(VCArglist *a)
{
	return cvt__int(mvchgat(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__long_unsigned_int((*a)[3]),
cvt_vc__short_int((*a)[4]),
cvt_vc_pq_void((*a)[5])));

}
static vc
wrap_mvaddstr(VCArglist *a)
{
	return cvt__int(mvaddstr(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_char((*a)[2])));

}
static vc
wrap_mvaddnstr(VCArglist *a)
{
	return cvt__int(mvaddnstr(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_char((*a)[2]),
cvt_vc__int((*a)[3])));

}
static vc
wrap_mvaddchstr(VCArglist *a)
{
	return cvt__int(mvaddchstr(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_long_unsigned_int((*a)[2])));

}
static vc
wrap_mvaddchnstr(VCArglist *a)
{
	return cvt__int(mvaddchnstr(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc_pq_long_unsigned_int((*a)[2]),
cvt_vc__int((*a)[3])));

}
static vc
wrap_mvaddch(VCArglist *a)
{
	return cvt__int(mvaddch(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2])));

}
static vc
wrap_move(VCArglist *a)
{
	return cvt__int(move(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_meta(VCArglist *a)
{
	return cvt__int(meta(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__bool((*a)[1])));

}
static vc
wrap_longname(VCArglist *a)
{
	return cvt_p_char(longname());

}
static vc
wrap_leaveok(VCArglist *a)
{
	return cvt__int(leaveok(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__bool((*a)[1])));

}
static vc
wrap_killchar(VCArglist *a)
{
	return cvt__char(killchar());

}
static vc
wrap_keypad(VCArglist *a)
{
	return cvt__int(keypad(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__bool((*a)[1])));

}
static vc
wrap_keyname(VCArglist *a)
{
	return cvt_p_char(keyname(cvt_vc__int((*a)[0])));

}
static vc
wrap_is_wintouched(VCArglist *a)
{
	return cvt__bool(is_wintouched(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_is_linetouched(VCArglist *a)
{
	return cvt__bool(is_linetouched(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_isendwin(VCArglist *a)
{
	return cvt__bool(isendwin());

}
static vc
wrap_intrflush(VCArglist *a)
{
	return cvt__int(intrflush(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__bool((*a)[1])));

}
static vc
wrap_instr(VCArglist *a)
{
	return cvt__int(instr(cvt_vc_p_char((*a)[0])));

}
static vc
wrap_insstr(VCArglist *a)
{
	return cvt__int(insstr(cvt_vc_pq_char((*a)[0])));

}
static vc
wrap_insnstr(VCArglist *a)
{
	return cvt__int(insnstr(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_insertln(VCArglist *a)
{
	return cvt__int(insertln());

}
static vc
wrap_insdelln(VCArglist *a)
{
	return cvt__int(insdelln(cvt_vc__int((*a)[0])));

}
static vc
wrap_insch(VCArglist *a)
{
	return cvt__int(insch(cvt_vc__long_unsigned_int((*a)[0])));

}
static vc
wrap_innstr(VCArglist *a)
{
	return cvt__int(innstr(cvt_vc_p_char((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_init_pair(VCArglist *a)
{
	return cvt__int(init_pair(cvt_vc__short_int((*a)[0]),
cvt_vc__short_int((*a)[1]),
cvt_vc__short_int((*a)[2])));

}
static vc
wrap_init_color(VCArglist *a)
{
	return cvt__int(init_color(cvt_vc__short_int((*a)[0]),
cvt_vc__short_int((*a)[1]),
cvt_vc__short_int((*a)[2]),
cvt_vc__short_int((*a)[3])));

}
static vc
wrap_initscr(VCArglist *a)
{
	return cvt_p__win_st_(initscr());

}
static vc
wrap_inchstr(VCArglist *a)
{
	return cvt__int(inchstr(cvt_vc_p_long_unsigned_int((*a)[0])));

}
static vc
wrap_inchnstr(VCArglist *a)
{
	return cvt__int(inchnstr(cvt_vc_p_long_unsigned_int((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_inch(VCArglist *a)
{
	return cvt__long_unsigned_int(inch());

}
static vc
wrap_immedok(VCArglist *a)
{
	immedok(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__bool((*a)[1]));
return(vcnil);

}
static vc
wrap_idlok(VCArglist *a)
{
	return cvt__int(idlok(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__bool((*a)[1])));

}
static vc
wrap_idcok(VCArglist *a)
{
	idcok(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__bool((*a)[1]));
return(vcnil);

}
static vc
wrap_hline(VCArglist *a)
{
	return cvt__int(hline(cvt_vc__long_unsigned_int((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_has_il(VCArglist *a)
{
	return cvt__bool(has_il());

}
static vc
wrap_has_ic(VCArglist *a)
{
	return cvt__bool(has_ic());

}
static vc
wrap_has_colors(VCArglist *a)
{
	return cvt__bool(has_colors());

}
static vc
wrap_halfdelay(VCArglist *a)
{
	return cvt__int(halfdelay(cvt_vc__int((*a)[0])));

}
#if 0
static vc
wrap_getwin(VCArglist *a)
{
	return cvt_p__win_st_(getwin(cvt_vc_p__IO_FILE_((*a)[0])));

}
#endif
static vc
wrap_getstr(VCArglist *a)
{
	return cvt__int(getstr(cvt_vc_p_char((*a)[0])));

}
static vc
wrap_getnstr(VCArglist *a)
{
	return cvt__int(getnstr(cvt_vc_p_char((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_getch(VCArglist *a)
{
	return cvt__int(getch());

}
static vc
wrap_getbkgd(VCArglist *a)
{
	return cvt__long_unsigned_int(getbkgd(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_flushinp(VCArglist *a)
{
	return cvt__int(flushinp());

}
static vc
wrap_flash(VCArglist *a)
{
	return cvt__int(flash());

}
static vc
wrap_filter(VCArglist *a)
{
	filter();
return(vcnil);

}
static vc
wrap_erasechar(VCArglist *a)
{
	return cvt__char(erasechar());

}
static vc
wrap_endwin(VCArglist *a)
{
	return cvt__int(endwin());

}
static vc
wrap_erase(VCArglist *a)
{
	return cvt__int(erase());

}
static vc
wrap_echochar(VCArglist *a)
{
	return cvt__int(echochar(cvt_vc__long_unsigned_int((*a)[0])));

}
static vc
wrap_echo(VCArglist *a)
{
	return cvt__int(echo());

}
static vc
wrap_dupwin(VCArglist *a)
{
	return cvt_p__win_st_(dupwin(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_doupdate(VCArglist *a)
{
	return cvt__int(doupdate());

}
static vc
wrap_derwin(VCArglist *a)
{
	return cvt_p__win_st_(derwin(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4])));

}
static vc
wrap_deleteln(VCArglist *a)
{
	return cvt__int(deleteln());

}
static vc
wrap_delwin(VCArglist *a)
{
	return cvt__int(delwin(cvt_vc_p__win_st_((*a)[0])));

}
static vc
wrap_delscreen(VCArglist *a)
{
	delscreen(cvt_vc_p_screen_((*a)[0]));
return(vcnil);

}
static vc
wrap_delch(VCArglist *a)
{
	return cvt__int(delch());

}
static vc
wrap_delay_output(VCArglist *a)
{
	return cvt__int(delay_output(cvt_vc__int((*a)[0])));

}
static vc
wrap_def_shell_mode(VCArglist *a)
{
	return cvt__int(def_shell_mode());

}
static vc
wrap_def_prog_mode(VCArglist *a)
{
	return cvt__int(def_prog_mode());

}
static vc
wrap_curs_set(VCArglist *a)
{
	return cvt__int(curs_set(cvt_vc__int((*a)[0])));

}
static vc
wrap_copywin(VCArglist *a)
{
	return cvt__int(copywin(cvt_vc_pq__win_st_((*a)[0]),
cvt_vc_p__win_st_((*a)[1]),
cvt_vc__int((*a)[2]),
cvt_vc__int((*a)[3]),
cvt_vc__int((*a)[4]),
cvt_vc__int((*a)[5]),
cvt_vc__int((*a)[6]),
cvt_vc__int((*a)[7]),
cvt_vc__int((*a)[8])));

}
static vc
wrap_COLOR_PAIR(VCArglist *a)
{
	return cvt__int(COLOR_PAIR(cvt_vc__int((*a)[0])));

}
static vc
wrap_color_set(VCArglist *a)
{
	return cvt__int(color_set(cvt_vc__short_int((*a)[0]),
cvt_vc_p_void((*a)[1])));

}
static vc
wrap_color_content(VCArglist *a)
{
	return cvt__int(color_content(cvt_vc__short_int((*a)[0]),
cvt_vc_p_short_int((*a)[1]),
cvt_vc_p_short_int((*a)[2]),
cvt_vc_p_short_int((*a)[3])));

}
static vc
wrap_clrtoeol(VCArglist *a)
{
	return cvt__int(clrtoeol());

}
static vc
wrap_clrtobot(VCArglist *a)
{
	return cvt__int(clrtobot());

}
static vc
wrap_clearok(VCArglist *a)
{
	return cvt__int(clearok(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__bool((*a)[1])));

}
static vc
wrap_clear(VCArglist *a)
{
	return cvt__int(clear());

}
static vc
wrap_chgat(VCArglist *a)
{
	return cvt__int(chgat(cvt_vc__int((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]),
cvt_vc__short_int((*a)[2]),
cvt_vc_pq_void((*a)[3])));

}
static vc
wrap_cbreak(VCArglist *a)
{
	return cvt__int(cbreak());

}
static vc
wrap_can_change_color(VCArglist *a)
{
	return cvt__bool(can_change_color());

}
static vc
wrap_box(VCArglist *a)
{
	return cvt__int(box(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2])));

}
static vc
wrap_border(VCArglist *a)
{
	return cvt__int(border(cvt_vc__long_unsigned_int((*a)[0]),
cvt_vc__long_unsigned_int((*a)[1]),
cvt_vc__long_unsigned_int((*a)[2]),
cvt_vc__long_unsigned_int((*a)[3]),
cvt_vc__long_unsigned_int((*a)[4]),
cvt_vc__long_unsigned_int((*a)[5]),
cvt_vc__long_unsigned_int((*a)[6]),
cvt_vc__long_unsigned_int((*a)[7])));

}
static vc
wrap_bkgdset(VCArglist *a)
{
	bkgdset(cvt_vc__long_unsigned_int((*a)[0]));
return(vcnil);

}
static vc
wrap_bkgd(VCArglist *a)
{
	return cvt__int(bkgd(cvt_vc__long_unsigned_int((*a)[0])));

}
static vc
wrap_beep(VCArglist *a)
{
	return cvt__int(beep());

}
static vc
wrap_baudrate(VCArglist *a)
{
	return cvt__int(baudrate());

}
static vc
wrap_attr_set(VCArglist *a)
{
	return cvt__int(attr_set(cvt_vc__long_unsigned_int((*a)[0]),
cvt_vc__short_int((*a)[1]),
cvt_vc_p_void((*a)[2])));

}
static vc
wrap_attr_on(VCArglist *a)
{
	return cvt__int(attr_on(cvt_vc__long_unsigned_int((*a)[0]),
cvt_vc_p_void((*a)[1])));

}
static vc
wrap_attr_off(VCArglist *a)
{
	return cvt__int(attr_off(cvt_vc__long_unsigned_int((*a)[0]),
cvt_vc_p_void((*a)[1])));

}
static vc
wrap_attr_get(VCArglist *a)
{
	return cvt__int(attr_get(cvt_vc_p_long_unsigned_int((*a)[0]),
cvt_vc_p_short_int((*a)[1]),
cvt_vc_p_void((*a)[2])));

}
static vc
wrap_attrset(VCArglist *a)
{
	return cvt__int(attrset(cvt_vc__int((*a)[0])));

}
static vc
wrap_attron(VCArglist *a)
{
	return cvt__int(attron(cvt_vc__int((*a)[0])));

}
static vc
wrap_attroff(VCArglist *a)
{
	return cvt__int(attroff(cvt_vc__int((*a)[0])));

}
static vc
wrap_addstr(VCArglist *a)
{
	return cvt__int(addstr(cvt_vc_pq_char((*a)[0])));

}
static vc
wrap_addnstr(VCArglist *a)
{
	return cvt__int(addnstr(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_addchstr(VCArglist *a)
{
	return cvt__int(addchstr(cvt_vc_pq_long_unsigned_int((*a)[0])));

}
static vc
wrap_addchnstr(VCArglist *a)
{
	return cvt__int(addchnstr(cvt_vc_pq_long_unsigned_int((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_addch(VCArglist *a)
{
	return cvt__int(addch(cvt_vc__long_unsigned_int((*a)[0])));

}
static vc
wrap_wresize(VCArglist *a)
{
	return cvt__int(wresize(cvt_vc_p__win_st_((*a)[0]),
cvt_vc__int((*a)[1]),
cvt_vc__int((*a)[2])));

}
static vc
wrap_use_extended_names(VCArglist *a)
{
	return cvt__int(use_extended_names(cvt_vc__bool((*a)[0])));

}
static vc
wrap_use_default_colors(VCArglist *a)
{
	return cvt__int(use_default_colors());

}
static vc
wrap_resizeterm(VCArglist *a)
{
	return cvt__int(resizeterm(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_resize_term(VCArglist *a)
{
	return cvt__int(resize_term(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_keyok(VCArglist *a)
{
	return cvt__int(keyok(cvt_vc__int((*a)[0]),
cvt_vc__bool((*a)[1])));

}
static vc
wrap_key_defined(VCArglist *a)
{
	return cvt__int(key_defined(cvt_vc_pq_char((*a)[0])));

}
static vc
wrap_define_key(VCArglist *a)
{
	return cvt__int(define_key(cvt_vc_pq_char((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_assume_default_colors(VCArglist *a)
{
	return cvt__int(assume_default_colors(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_curses_version(VCArglist *a)
{
	return cvt_pq_char(curses_version());

}
static vc
wrap_keybound(VCArglist *a)
{
	return cvt_p_char(keybound(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_is_term_resized(VCArglist *a)
{
	return cvt__bool(is_term_resized(cvt_vc__int((*a)[0]),
cvt_vc__int((*a)[1])));

}
static vc
wrap_unctrl(VCArglist *a)
{
	return cvt_p_char(unctrl(cvt_vc__long_unsigned_int((*a)[0])));

}

static void
makefun(const char *name, const vc& fun)
{
    vc(name).bind(fun);
}
void
wrapper_init_curses_xml ()
{

vc("wrap_stdscr").global_bind(cvt_p__win_st_(stdscr));
vc("wrap_curscr").global_bind(cvt_p__win_st_(curscr));
vc("wrap_newscr").global_bind(cvt_p__win_st_(newscr));

makefun("wrap_addchstr", vc(wrap_addchstr, "wrap_addchstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_attron", vc(wrap_attron, "wrap_attron", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_bkgdset", vc(wrap_bkgdset, "wrap_bkgdset", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_cbreak", vc(wrap_cbreak, "wrap_cbreak", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_clear", vc(wrap_clear, "wrap_clear", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getstr", vc(wrap_getstr, "wrap_getstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_idlok", vc(wrap_idlok, "wrap_idlok", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_intrflush", vc(wrap_intrflush, "wrap_intrflush", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_leaveok", vc(wrap_leaveok, "wrap_leaveok", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwdelch", vc(wrap_mvwdelch, "wrap_mvwdelch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwinstr", vc(wrap_mvwinstr, "wrap_mvwinstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_noecho", vc(wrap_noecho, "wrap_noecho", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_start_color", vc(wrap_start_color, "wrap_start_color", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_vw_printw", vc(wrap_vw_printw, "wrap_vw_printw", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wclrtobot", vc(wrap_wclrtobot, "wrap_wclrtobot", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wechochar", vc(wrap_wechochar, "wrap_wechochar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mcprint", vc(wrap_mcprint, "wrap_mcprint", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_attroff", vc(wrap_attroff, "wrap_attroff", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_clearok", vc(wrap_clearok, "wrap_clearok", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_curs_set", vc(wrap_curs_set, "wrap_curs_set", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_nl", vc(wrap_nl, "wrap_nl", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_slk_label", vc(wrap_slk_label, "wrap_slk_label", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_tigetstr", vc(wrap_tigetstr, "wrap_tigetstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wchgat", vc(wrap_wchgat, "wrap_wchgat", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wgetstr", vc(wrap_wgetstr, "wrap_wgetstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wvline", vc(wrap_wvline, "wrap_wvline", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap__tracecchar_t2", vc(wrap__tracecchar_t2, "wrap__tracecchar_t2", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvcur", vc(wrap_mvcur, "wrap_mvcur", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwhline", vc(wrap_mvwhline, "wrap_mvwhline", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_vwscanw", vc(wrap_vwscanw, "wrap_vwscanw", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_winnstr", vc(wrap_winnstr, "wrap_winnstr", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap__tracechar", vc(wrap__tracechar, "wrap__tracechar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_define_key", vc(wrap_define_key, "wrap_define_key", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_clrtoeol", vc(wrap_clrtoeol, "wrap_clrtoeol", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_def_shell_mode", vc(wrap_def_shell_mode, "wrap_def_shell_mode", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvdelch", vc(wrap_mvdelch, "wrap_mvdelch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_raw", vc(wrap_raw, "wrap_raw", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_scanw", vc(wrap_scanw, "wrap_scanw", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_werase", vc(wrap_werase, "wrap_werase", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_attr_get", vc(wrap_attr_get, "wrap_attr_get", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_attr_off", vc(wrap_attr_off, "wrap_attr_off", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_deleteln", vc(wrap_deleteln, "wrap_deleteln", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_filter", vc(wrap_filter, "wrap_filter", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_flushinp", vc(wrap_flushinp, "wrap_flushinp", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_meta", vc(wrap_meta, "wrap_meta", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_noqiflush", vc(wrap_noqiflush, "wrap_noqiflush", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_notimeout", vc(wrap_notimeout, "wrap_notimeout", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_printw", vc(wrap_printw, "wrap_printw", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_putwin", vc(wrap_putwin, "wrap_putwin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_slk_init", vc(wrap_slk_init, "wrap_slk_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_touchline", vc(wrap_touchline, "wrap_touchline", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wcolor_set", vc(wrap_wcolor_set, "wrap_wcolor_set", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_baudrate", vc(wrap_baudrate, "wrap_baudrate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_chgat", vc(wrap_chgat, "wrap_chgat", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_derwin", vc(wrap_derwin, "wrap_derwin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvinchnstr", vc(wrap_mvinchnstr, "wrap_mvinchnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wattroff", vc(wrap_wattroff, "wrap_wattroff", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_winstr", vc(wrap_winstr, "wrap_winstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wtimeout", vc(wrap_wtimeout, "wrap_wtimeout", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mouse_trafo", vc(wrap_mouse_trafo, "wrap_mouse_trafo", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_resizeterm", vc(wrap_resizeterm, "wrap_resizeterm", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wresize", vc(wrap_wresize, "wrap_wresize", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_addch", vc(wrap_addch, "wrap_addch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_attr_on", vc(wrap_attr_on, "wrap_attr_on", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_pnoutrefresh", vc(wrap_pnoutrefresh, "wrap_pnoutrefresh", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_scrl", vc(wrap_scrl, "wrap_scrl", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_slk_attrset", vc(wrap_slk_attrset, "wrap_slk_attrset", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_subwin", vc(wrap_subwin, "wrap_subwin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_typeahead", vc(wrap_typeahead, "wrap_typeahead", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wdelch", vc(wrap_wdelch, "wrap_wdelch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wredrawln", vc(wrap_wredrawln, "wrap_wredrawln", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wstandend", vc(wrap_wstandend, "wrap_wstandend", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wenclose", vc(wrap_wenclose, "wrap_wenclose", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_erasechar", vc(wrap_erasechar, "wrap_erasechar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_insch", vc(wrap_insch, "wrap_insch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvgetnstr", vc(wrap_mvgetnstr, "wrap_mvgetnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvinsnstr", vc(wrap_mvinsnstr, "wrap_mvinsnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwchgat", vc(wrap_mvwchgat, "wrap_mvwchgat", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_mvwscanw", vc(wrap_mvwscanw, "wrap_mvwscanw", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_nonl", vc(wrap_nonl, "wrap_nonl", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_noraw", vc(wrap_noraw, "wrap_noraw", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_resetty", vc(wrap_resetty, "wrap_resetty", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_slk_restore", vc(wrap_slk_restore, "wrap_slk_restore", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_syncok", vc(wrap_syncok, "wrap_syncok", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_touchwin", vc(wrap_touchwin, "wrap_touchwin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wattron", vc(wrap_wattron, "wrap_wattron", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_winchnstr", vc(wrap_winchnstr, "wrap_winchnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getbkgd", vc(wrap_getbkgd, "wrap_getbkgd", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwaddchstr", vc(wrap_mvwaddchstr, "wrap_mvwaddchstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_scr_set", vc(wrap_scr_set, "wrap_scr_set", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_untouchwin", vc(wrap_untouchwin, "wrap_untouchwin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wattr_get", vc(wrap_wattr_get, "wrap_wattr_get", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wgetch", vc(wrap_wgetch, "wrap_wgetch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_flash", vc(wrap_flash, "wrap_flash", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_idcok", vc(wrap_idcok, "wrap_idcok", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvaddchnstr", vc(wrap_mvaddchnstr, "wrap_mvaddchnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvchgat", vc(wrap_mvchgat, "wrap_mvchgat", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvderwin", vc(wrap_mvderwin, "wrap_mvderwin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvgetch", vc(wrap_mvgetch, "wrap_mvgetch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwaddchnstr", vc(wrap_mvwaddchnstr, "wrap_mvwaddchnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwgetstr", vc(wrap_mvwgetstr, "wrap_mvwgetstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwinsstr", vc(wrap_mvwinsstr, "wrap_mvwinsstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_reset_prog_mode", vc(wrap_reset_prog_mode, "wrap_reset_prog_mode", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_set_term", vc(wrap_set_term, "wrap_set_term", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_wscanw", vc(wrap_wscanw, "wrap_wscanw", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_clrtobot", vc(wrap_clrtobot, "wrap_clrtobot", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_color_content", vc(wrap_color_content, "wrap_color_content", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_halfdelay", vc(wrap_halfdelay, "wrap_halfdelay", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_has_ic", vc(wrap_has_ic, "wrap_has_ic", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_inchnstr", vc(wrap_inchnstr, "wrap_inchnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_is_linetouched", vc(wrap_is_linetouched, "wrap_is_linetouched", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwaddstr", vc(wrap_mvwaddstr, "wrap_mvwaddstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwin", vc(wrap_mvwin, "wrap_mvwin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_vidattr", vc(wrap_vidattr, "wrap_vidattr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wborder", vc(wrap_wborder, "wrap_wborder", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wnoutrefresh", vc(wrap_wnoutrefresh, "wrap_wnoutrefresh", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_can_change_color", vc(wrap_can_change_color, "wrap_can_change_color", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_delwin", vc(wrap_delwin, "wrap_delwin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_erase", vc(wrap_erase, "wrap_erase", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_immedok", vc(wrap_immedok, "wrap_immedok", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_initscr", vc(wrap_initscr, "wrap_initscr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_instr", vc(wrap_instr, "wrap_instr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_putp", vc(wrap_putp, "wrap_putp", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_vwprintw", vc(wrap_vwprintw, "wrap_vwprintw", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mouseinterval", vc(wrap_mouseinterval, "wrap_mouseinterval", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_has_key", vc(wrap_has_key, "wrap_has_key", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap__tracemouse", vc(wrap__tracemouse, "wrap__tracemouse", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_has_colors", vc(wrap_has_colors, "wrap_has_colors", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwinsnstr", vc(wrap_mvwinsnstr, "wrap_mvwinsnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_slk_refresh", vc(wrap_slk_refresh, "wrap_slk_refresh", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_tigetflag", vc(wrap_tigetflag, "wrap_tigetflag", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_waddnstr", vc(wrap_waddnstr, "wrap_waddnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wstandout", vc(wrap_wstandout, "wrap_wstandout", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_endwin", vc(wrap_endwin, "wrap_endwin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_inchstr", vc(wrap_inchstr, "wrap_inchstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvinsch", vc(wrap_mvinsch, "wrap_mvinsch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwaddnstr", vc(wrap_mvwaddnstr, "wrap_mvwaddnstr", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_newterm", vc(wrap_newterm, "wrap_newterm", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_slk_attron", vc(wrap_slk_attron, "wrap_slk_attron", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_slk_clear", vc(wrap_slk_clear, "wrap_slk_clear", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wbkgdset", vc(wrap_wbkgdset, "wrap_wbkgdset", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_whline", vc(wrap_whline, "wrap_whline", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wscrl", vc(wrap_wscrl, "wrap_wscrl", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mousemask", vc(wrap_mousemask, "wrap_mousemask", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_mvscanw", vc(wrap_mvscanw, "wrap_mvscanw", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_slk_attroff", vc(wrap_slk_attroff, "wrap_slk_attroff", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap__tracechtype", vc(wrap__tracechtype, "wrap__tracechtype", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_delay_output", vc(wrap_delay_output, "wrap_delay_output", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_echo", vc(wrap_echo, "wrap_echo", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_insnstr", vc(wrap_insnstr, "wrap_insnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvaddnstr", vc(wrap_mvaddnstr, "wrap_mvaddnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvgetstr", vc(wrap_mvgetstr, "wrap_mvgetstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvinsstr", vc(wrap_mvinsstr, "wrap_mvinsstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_setscrreg", vc(wrap_setscrreg, "wrap_setscrreg", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_termname", vc(wrap_termname, "wrap_termname", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_vw_scanw", vc(wrap_vw_scanw, "wrap_vw_scanw", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_waddstr", vc(wrap_waddstr, "wrap_waddstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_winchstr", vc(wrap_winchstr, "wrap_winchstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wsyncup", vc(wrap_wsyncup, "wrap_wsyncup", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_trace", vc(wrap_trace, "wrap_trace", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_unctrl", vc(wrap_unctrl, "wrap_unctrl", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getch", vc(wrap_getch, "wrap_getch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_hline", vc(wrap_hline, "wrap_hline", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_keypad", vc(wrap_keypad, "wrap_keypad", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvaddchstr", vc(wrap_mvaddchstr, "wrap_mvaddchstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwgetnstr", vc(wrap_mvwgetnstr, "wrap_mvwgetnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_tigetnum", vc(wrap_tigetnum, "wrap_tigetnum", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_tparm", vc(wrap_tparm, "wrap_tparm", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wattr_off", vc(wrap_wattr_off, "wrap_wattr_off", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap__tracechtype2", vc(wrap__tracechtype2, "wrap__tracechtype2", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_assume_default_colors", vc(wrap_assume_default_colors, "wrap_assume_default_colors", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_key_defined", vc(wrap_key_defined, "wrap_key_defined", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_attr_set", vc(wrap_attr_set, "wrap_attr_set", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_def_prog_mode", vc(wrap_def_prog_mode, "wrap_def_prog_mode", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_keyname", vc(wrap_keyname, "wrap_keyname", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvaddch", vc(wrap_mvaddch, "wrap_mvaddch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwinchnstr", vc(wrap_mvwinchnstr, "wrap_mvwinchnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_scr_init", vc(wrap_scr_init, "wrap_scr_init", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_slk_attr_off", vc(wrap_slk_attr_off, "wrap_slk_attr_off", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_slk_noutrefresh", vc(wrap_slk_noutrefresh, "wrap_slk_noutrefresh", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_standout", vc(wrap_standout, "wrap_standout", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_ungetmouse", vc(wrap_ungetmouse, "wrap_ungetmouse", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_addnstr", vc(wrap_addnstr, "wrap_addnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_color_set", vc(wrap_color_set, "wrap_color_set", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_init_pair", vc(wrap_init_pair, "wrap_init_pair", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_innstr", vc(wrap_innstr, "wrap_innstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_insdelln", vc(wrap_insdelln, "wrap_insdelln", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_PAIR_NUMBER", vc(wrap_PAIR_NUMBER, "wrap_PAIR_NUMBER", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_vidputs", vc(wrap_vidputs, "wrap_vidputs", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_waddch", vc(wrap_waddch, "wrap_waddch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wattrset", vc(wrap_wattrset, "wrap_wattrset", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wclear", vc(wrap_wclear, "wrap_wclear", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap__tracedump", vc(wrap__tracedump, "wrap__tracedump", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_curses_version", vc(wrap_curses_version, "wrap_curses_version", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_napms", vc(wrap_napms, "wrap_napms", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_slk_attr_on", vc(wrap_slk_attr_on, "wrap_slk_attr_on", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_use_env", vc(wrap_use_env, "wrap_use_env", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wclrtoeol", vc(wrap_wclrtoeol, "wrap_wclrtoeol", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wcursyncup", vc(wrap_wcursyncup, "wrap_wcursyncup", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_winsnstr", vc(wrap_winsnstr, "wrap_winsnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wmove", vc(wrap_wmove, "wrap_wmove", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_beep", vc(wrap_beep, "wrap_beep", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_copywin", vc(wrap_copywin, "wrap_copywin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_isendwin", vc(wrap_isendwin, "wrap_isendwin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwvline", vc(wrap_mvwvline, "wrap_mvwvline", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_scr_restore", vc(wrap_scr_restore, "wrap_scr_restore", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_waddchstr", vc(wrap_waddchstr, "wrap_waddchstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wattr_on", vc(wrap_wattr_on, "wrap_wattr_on", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_keybound", vc(wrap_keybound, "wrap_keybound", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_addstr", vc(wrap_addstr, "wrap_addstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvinnstr", vc(wrap_mvinnstr, "wrap_mvinnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvvline", vc(wrap_mvvline, "wrap_mvvline", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_prefresh", vc(wrap_prefresh, "wrap_prefresh", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_redrawwin", vc(wrap_redrawwin, "wrap_redrawwin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_standend", vc(wrap_standend, "wrap_standend", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_ungetch", vc(wrap_ungetch, "wrap_ungetch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wbkgd", vc(wrap_wbkgd, "wrap_wbkgd", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_winch", vc(wrap_winch, "wrap_winch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_bkgd", vc(wrap_bkgd, "wrap_bkgd", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_border", vc(wrap_border, "wrap_border", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_insstr", vc(wrap_insstr, "wrap_insstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_is_wintouched", vc(wrap_is_wintouched, "wrap_is_wintouched", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvhline", vc(wrap_mvhline, "wrap_mvhline", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_winsstr", vc(wrap_winsstr, "wrap_winsstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_addchnstr", vc(wrap_addchnstr, "wrap_addchnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_delscreen", vc(wrap_delscreen, "wrap_delscreen", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_dupwin", vc(wrap_dupwin, "wrap_dupwin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvaddstr", vc(wrap_mvaddstr, "wrap_mvaddstr", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_mvprintw", vc(wrap_mvprintw, "wrap_mvprintw", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwinnstr", vc(wrap_mvwinnstr, "wrap_mvwinnstr", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_mvwprintw", vc(wrap_mvwprintw, "wrap_mvwprintw", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_overlay", vc(wrap_overlay, "wrap_overlay", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_winsertln", vc(wrap_winsertln, "wrap_winsertln", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_wprintw", vc(wrap_wprintw, "wrap_wprintw", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap__nc_tracebits", vc(wrap__nc_tracebits, "wrap__nc_tracebits", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_is_term_resized", vc(wrap_is_term_resized, "wrap_is_term_resized", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_keyok", vc(wrap_keyok, "wrap_keyok", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_attrset", vc(wrap_attrset, "wrap_attrset", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_echochar", vc(wrap_echochar, "wrap_echochar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getnstr", vc(wrap_getnstr, "wrap_getnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_has_il", vc(wrap_has_il, "wrap_has_il", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvinstr", vc(wrap_mvinstr, "wrap_mvinstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwinchstr", vc(wrap_mvwinchstr, "wrap_mvwinchstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newpad", vc(wrap_newpad, "wrap_newpad", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_scr_dump", vc(wrap_scr_dump, "wrap_scr_dump", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_vline", vc(wrap_vline, "wrap_vline", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wsetscrreg", vc(wrap_wsetscrreg, "wrap_wsetscrreg", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wtouchln", vc(wrap_wtouchln, "wrap_wtouchln", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap__traceattr", vc(wrap__traceattr, "wrap__traceattr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_use_default_colors", vc(wrap_use_default_colors, "wrap_use_default_colors", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_COLOR_PAIR", vc(wrap_COLOR_PAIR, "wrap_COLOR_PAIR", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_init_color", vc(wrap_init_color, "wrap_init_color", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_reset_shell_mode", vc(wrap_reset_shell_mode, "wrap_reset_shell_mode", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_ripoffline", vc(wrap_ripoffline, "wrap_ripoffline", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_slk_touch", vc(wrap_slk_touch, "wrap_slk_touch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_termattrs", vc(wrap_termattrs, "wrap_termattrs", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_timeout", vc(wrap_timeout, "wrap_timeout", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wdeleteln", vc(wrap_wdeleteln, "wrap_wdeleteln", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_winsch", vc(wrap_winsch, "wrap_winsch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wrefresh", vc(wrap_wrefresh, "wrap_wrefresh", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_resize_term", vc(wrap_resize_term, "wrap_resize_term", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_doupdate", vc(wrap_doupdate, "wrap_doupdate", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_longname", vc(wrap_longname, "wrap_longname", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwgetch", vc(wrap_mvwgetch, "wrap_mvwgetch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwinsch", vc(wrap_mvwinsch, "wrap_mvwinsch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_scroll", vc(wrap_scroll, "wrap_scroll", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_slk_color", vc(wrap_slk_color, "wrap_slk_color", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_slk_set", vc(wrap_slk_set, "wrap_slk_set", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wsyncdown", vc(wrap_wsyncdown, "wrap_wsyncdown", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap__traceattr2", vc(wrap__traceattr2, "wrap__traceattr2", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_delch", vc(wrap_delch, "wrap_delch", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap_getwin", vc(wrap_getwin, "wrap_getwin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_move", vc(wrap_move, "wrap_move", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvinch", vc(wrap_mvinch, "wrap_mvinch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwaddch", vc(wrap_mvwaddch, "wrap_mvwaddch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_nocbreak", vc(wrap_nocbreak, "wrap_nocbreak", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_pair_content", vc(wrap_pair_content, "wrap_pair_content", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_pechochar", vc(wrap_pechochar, "wrap_pechochar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_scrollok", vc(wrap_scrollok, "wrap_scrollok", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_waddchnstr", vc(wrap_waddchnstr, "wrap_waddchnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wattr_set", vc(wrap_wattr_set, "wrap_wattr_set", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wmouse_trafo", vc(wrap_wmouse_trafo, "wrap_wmouse_trafo", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap__tracef", vc(wrap__tracef, "wrap__tracef", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_inch", vc(wrap_inch, "wrap_inch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_newwin", vc(wrap_newwin, "wrap_newwin", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_qiflush", vc(wrap_qiflush, "wrap_qiflush", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_savetty", vc(wrap_savetty, "wrap_savetty", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_slk_attr", vc(wrap_slk_attr, "wrap_slk_attr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_getmouse", vc(wrap_getmouse, "wrap_getmouse", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_use_extended_names", vc(wrap_use_extended_names, "wrap_use_extended_names", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_box", vc(wrap_box, "wrap_box", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_insertln", vc(wrap_insertln, "wrap_insertln", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvinchstr", vc(wrap_mvinchstr, "wrap_mvinchstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_mvwinch", vc(wrap_mvwinch, "wrap_mvwinch", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_nodelay", vc(wrap_nodelay, "wrap_nodelay", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_overwrite", vc(wrap_overwrite, "wrap_overwrite", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_subpad", vc(wrap_subpad, "wrap_subpad", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_wgetnstr", vc(wrap_wgetnstr, "wrap_wgetnstr", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_killchar", vc(wrap_killchar, "wrap_killchar", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_refresh", vc(wrap_refresh, "wrap_refresh", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_slk_attr_set", vc(wrap_slk_attr_set, "wrap_slk_attr_set", VC_FUNC_BUILTIN_LEAF));

makefun("wrap_winsdelln", vc(wrap_winsdelln, "wrap_winsdelln", VC_FUNC_BUILTIN_LEAF));

//makefun("wrap__tracecchar_t", vc(wrap__tracecchar_t, "wrap__tracecchar_t", VC_FUNC_BUILTIN_LEAF));

}
