/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <Windows.h>
#include <CommCtrl.h>
#include "param_wm.h"

// Structs that extract values from window messages.
namespace wl::msg {

#define WINLAMB_PARM_NOTIFY(msgname, hdrtype, hdrname) \
	struct msgname : public wm { \
		msgname(wm p) noexcept : wm{p} { } \
		[[nodiscard]] hdrtype& hdrname() const noexcept { return *reinterpret_cast<hdrtype*>(this->lparam); } \
	};

WINLAMB_PARM_NOTIFY(wm_notify, NMHDR, nmhdr)

WINLAMB_PARM_NOTIFY(cben_begin_edit, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(cben_delete_item, NMCOMBOBOXEXW, nmcomboboxex)
WINLAMB_PARM_NOTIFY(cben_drag_begin, NMCBEDRAGBEGINW, nmcbedragbegin)
WINLAMB_PARM_NOTIFY(cben_end_edit, NMCBEENDEDITW, nmcbeendedit)
WINLAMB_PARM_NOTIFY(cben_get_disp_info, NMCOMBOBOXEXW, nmcomboboxex)
WINLAMB_PARM_NOTIFY(cben_insert_item, NMCOMBOBOXEXW, nmcomboboxex)
WINLAMB_PARM_NOTIFY(cben_set_cursor, NMMOUSE, nmmouse)

WINLAMB_PARM_NOTIFY(dtn_close_up, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(dtn_date_time_change, NMDATETIMECHANGE, nmdatetimechange)
WINLAMB_PARM_NOTIFY(dtn_drop_down, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(dtn_format, NMDATETIMEFORMATW, nmdatetimeformat)
WINLAMB_PARM_NOTIFY(dtn_format_query, NMDATETIMEFORMATQUERYW, nmdatetimeformatquery)
WINLAMB_PARM_NOTIFY(dtn_user_string, NMDATETIMESTRINGW, nmdatetimestring)
WINLAMB_PARM_NOTIFY(dtn_wm_key_down, NMDATETIMEWMKEYDOWNW, nmdatetimewmkeydown)
WINLAMB_PARM_NOTIFY(dtn_kill_focus, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(dtn_set_focus, NMHDR, nmhdr)

WINLAMB_PARM_NOTIFY(lvn_begin_drag, NMLISTVIEW, nmlistview)
WINLAMB_PARM_NOTIFY(lvn_begin_label_edit, NMLVDISPINFOW, nmlvdispinfo)
WINLAMB_PARM_NOTIFY(lvn_begin_r_drag, NMLISTVIEW, nmlistview)
WINLAMB_PARM_NOTIFY(lvn_begin_scroll, NMLVSCROLL, nmlvscroll)
WINLAMB_PARM_NOTIFY(lvn_column_click, NMLISTVIEW, nmlistview)
WINLAMB_PARM_NOTIFY(lvn_column_drop_down, NMLISTVIEW, nmlistview)
WINLAMB_PARM_NOTIFY(lvn_column_overflow_click, NMLISTVIEW, nmlistview)
WINLAMB_PARM_NOTIFY(lvn_delete_all_items, NMLISTVIEW, nmlistview)
WINLAMB_PARM_NOTIFY(lvn_delete_item, NMLISTVIEW, nmlistview)
WINLAMB_PARM_NOTIFY(lvn_end_label_edit, NMLVDISPINFOW, nmlvdispinfo)
WINLAMB_PARM_NOTIFY(lvn_end_scroll, NMLVSCROLL, nmlvscroll)
WINLAMB_PARM_NOTIFY(lvn_get_disp_info, NMLVDISPINFOW, nmlvdispinfo)
WINLAMB_PARM_NOTIFY(lvn_get_empty_markup, NMLVEMPTYMARKUP, nmlvemptymarkup)
WINLAMB_PARM_NOTIFY(lvn_get_info_tip, LPNMLVGETINFOTIPW, nmlvgetinfotip)
WINLAMB_PARM_NOTIFY(lvn_hot_track, NMLISTVIEW, nmlistview)
WINLAMB_PARM_NOTIFY(lvn_incremental_search, NMLVFINDITEMW, nmlvfinditem)
WINLAMB_PARM_NOTIFY(lvn_insert_item, NMLISTVIEW, nmlistview)
WINLAMB_PARM_NOTIFY(lvn_item_activate, NMITEMACTIVATE, nmitemactivate)
WINLAMB_PARM_NOTIFY(lvn_item_changed, NMLISTVIEW, nmlistview)
WINLAMB_PARM_NOTIFY(lvn_item_changing, NMLISTVIEW, nmlistview)
WINLAMB_PARM_NOTIFY(lvn_key_down, NMLVKEYDOWN, nmlvkeydown)
WINLAMB_PARM_NOTIFY(lvn_link_click, NMLVLINK, nmlvlink)
WINLAMB_PARM_NOTIFY(lvn_marquee_begin, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(lvn_od_cache_hint, NMLVCACHEHINT, nmlvcachehint)
WINLAMB_PARM_NOTIFY(lvn_od_find_item, NMLVFINDITEMW, nmlvfinditem)
WINLAMB_PARM_NOTIFY(lvn_od_state_changed, NMLVODSTATECHANGE, nmlvodstatechange)
WINLAMB_PARM_NOTIFY(lvn_set_disp_info, NMLVDISPINFOW, nmlvdispinfo)
WINLAMB_PARM_NOTIFY(lvn_click, NMITEMACTIVATE, nmitemactivate)
WINLAMB_PARM_NOTIFY(lvn_custom_draw, NMLVCUSTOMDRAW, nmlvcustomdraw)
WINLAMB_PARM_NOTIFY(lvn_dbl_clk, NMITEMACTIVATE, nmitemactivate)
WINLAMB_PARM_NOTIFY(lvn_hover, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(lvn_kill_focus, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(lvn_r_click, NMITEMACTIVATE, nmitemactivate)
WINLAMB_PARM_NOTIFY(lvn_r_dbl_clk, NMITEMACTIVATE, nmitemactivate)
WINLAMB_PARM_NOTIFY(lvn_released_capture, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(lvn_return, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(lvn_set_focus, NMHDR, nmhdr)

WINLAMB_PARM_NOTIFY(mcn_get_day_state, NMDAYSTATE, nmdaystate)
WINLAMB_PARM_NOTIFY(mcn_sel_change, NMSELCHANGE, nmselchange)
WINLAMB_PARM_NOTIFY(mcn_select, NMSELCHANGE, nmselchange)
WINLAMB_PARM_NOTIFY(mcn_view_change, NMVIEWCHANGE, nmviewchange)
WINLAMB_PARM_NOTIFY(mcn_released_capture, NMHDR, nmhdr)

WINLAMB_PARM_NOTIFY(sbn_simple_mode_change, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(sbn_click, NMMOUSE, nmmouse)
WINLAMB_PARM_NOTIFY(sbn_dbl_clk, NMMOUSE, nmmouse)
WINLAMB_PARM_NOTIFY(sbn_r_click, NMMOUSE, nmmouse)
WINLAMB_PARM_NOTIFY(sbn_r_dbl_clk, NMMOUSE, nmmouse)

WINLAMB_PARM_NOTIFY(sln_click, NMLINK, nmlink)

WINLAMB_PARM_NOTIFY(tcn_focus_change, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(tcn_get_object, NMOBJECTNOTIFY, nmobjectnotify)
WINLAMB_PARM_NOTIFY(tcn_key_down, NMTCKEYDOWN, nmtckeydown)
WINLAMB_PARM_NOTIFY(tcn_sel_change, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(tcn_sel_changing, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(tcn_click, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(tcn_dbl_clk, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(tcn_r_click, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(tcn_r_dbl_clk, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(tcn_released_capture, NMHDR, nmhdr)

WINLAMB_PARM_NOTIFY(trbn_thumb_pos_changing, NMTRBTHUMBPOSCHANGING, nmtrbthumbposchanging)
WINLAMB_PARM_NOTIFY(trbn_custom_draw, NMCUSTOMDRAW, nmcustomdraw)
WINLAMB_PARM_NOTIFY(trbn_released_capture, NMHDR, nmhdr)

WINLAMB_PARM_NOTIFY(ttn_get_disp_info, NMTTDISPINFOW, nmttdispinfo)
WINLAMB_PARM_NOTIFY(ttn_link_click, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(ttn_need_text, NMTTDISPINFOW, nmttdispinfo)
WINLAMB_PARM_NOTIFY(ttn_pop, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(ttn_show, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(ttn_custom_draw, NMTTCUSTOMDRAW, nmttcustomdraw)

WINLAMB_PARM_NOTIFY(tvn_async_draw, NMTVASYNCDRAW, nmtvasyncdraw)
WINLAMB_PARM_NOTIFY(tvn_begin_drag, NMTREEVIEWW, nmtreeview)
WINLAMB_PARM_NOTIFY(tvn_begin_label_edit, NMTVDISPINFOW, nmtvdispinfo)
WINLAMB_PARM_NOTIFY(tvn_begin_r_drag, NMTREEVIEWW, nmtreeview)
WINLAMB_PARM_NOTIFY(tvn_delete_item, NMTREEVIEWW, nmtreeview)
WINLAMB_PARM_NOTIFY(tvn_end_label_edit, NMTVDISPINFOW, nmtvdispinfo)
WINLAMB_PARM_NOTIFY(tvn_get_disp_info, NMTVDISPINFOW, nmtvdispinfo)
WINLAMB_PARM_NOTIFY(tvn_get_info_tip, NMTVGETINFOTIPW, nmtvgetinfotip)
WINLAMB_PARM_NOTIFY(tvn_item_changed, NMTVITEMCHANGE, nmtvitemchange)
WINLAMB_PARM_NOTIFY(tvn_item_changing, NMTVITEMCHANGE, nmtvitemchange)
WINLAMB_PARM_NOTIFY(tvn_item_expanded, NMTREEVIEWW, nmtreeview)
WINLAMB_PARM_NOTIFY(tvn_item_expanding, NMTREEVIEWW, nmtreeview)
WINLAMB_PARM_NOTIFY(tvn_key_down, NMTVKEYDOWN, nmtvkeydown)
WINLAMB_PARM_NOTIFY(tvn_sel_changed, NMTREEVIEWW, nmtreeview)
WINLAMB_PARM_NOTIFY(tvn_sel_changing, NMTREEVIEWW, nmtreeview)
WINLAMB_PARM_NOTIFY(tvn_set_disp_info, NMTVDISPINFOW, nmtvdispinfo)
WINLAMB_PARM_NOTIFY(tvn_single_expand, NMTREEVIEWW, nmtreeview)
WINLAMB_PARM_NOTIFY(tvn_click, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(tvn_custom_draw, NMTVCUSTOMDRAW, nmtvcustomdraw)
WINLAMB_PARM_NOTIFY(tvn_dbl_clk, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(tvn_kill_focus, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(tvn_r_click, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(tvn_r_dbl_clk, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(tvn_return, NMHDR, nmhdr)
WINLAMB_PARM_NOTIFY(tvn_set_cursor, NMMOUSE, nmmouse)
WINLAMB_PARM_NOTIFY(tvn_set_focus, NMHDR, nmhdr)

WINLAMB_PARM_NOTIFY(udn_delta_pos, NMUPDOWN, nmupdown)
WINLAMB_PARM_NOTIFY(udn_released_capture, NMHDR, nmhdr)

}//namespace wl::msg