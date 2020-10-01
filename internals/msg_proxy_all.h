/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <type_traits>
#include <Windows.h>
#include "msg_proxy.h"
#include "param_notif.h"
#include "store.h"

namespace _wli {

// Exposes specific methods to add message handlers, commands and notifications.
class msg_proxy_all final : public msg_proxy {
private:
	store<WORD>& _cmds;
	store<std::pair<UINT_PTR, int>>& _nfys;

public:
	msg_proxy_all(store<UINT>& msgs, store<WORD>& cmds, store<std::pair<UINT_PTR, int>>& nfys, bool isDialog) :
		msg_proxy{msgs, isDialog}, _cmds{cmds}, _nfys{nfys} { }

	/// Adds a handler to WM_COMMAND, to a specific command ID.
	/// @param func [](msg::wm_command p) {}
	/// @see https://docs.microsoft.com/en-us/windows/win32/menurc/wm-command
	template<typename F>
	auto wm_command(WORD commandId, F&& func) -> std::enable_if_t<
		std::is_same_v<
			decltype(std::function{std::forward<F>(func)}),
			std::function<void(wl::msg::wm_command)>
		>>
	{
		this->_cmds.add(commandId, [func{std::move(func)}, isDialog{this->_isDialog}](wl::msg::wm_command p) -> LRESULT {
			func(p);
			return isDialog ? TRUE : 0;
		});
	}
	template<typename F>
	auto wm_command(WORD commandId, F&& func) -> std::enable_if_t<
		std::is_same_v<
			decltype(std::function{std::forward<F>(func)}),
			std::function<void()>
		>>
	{
		this->_cmds.add(commandId, [func{std::move(func)}, isDialog{this->_isDialog}](wl::msg::wm_command p) -> LRESULT {
			func();
			return isDialog ? TRUE : 0;
		});
	}
	template<typename F>
	auto wm_command(std::initializer_list<WORD> commandIds, F&& func) -> std::enable_if_t<
		std::is_same_v<
			decltype(std::function{std::forward<F>(func)}),
			std::function<void(wl::msg::wm_command)>
		>>
	{
		this->_cmds.add(commandIds, [func{std::move(func)}, isDialog{this->_isDialog}](wl::msg::wm_command p) -> LRESULT {
			func(p);
			return isDialog ? TRUE : 0;
		});
	}
	template<typename F>
	auto wm_command(std::initializer_list<WORD> commandIds, F&& func) -> std::enable_if_t<
		std::is_same_v<
			decltype(std::function{std::forward<F>(func)}),
			std::function<void()>
		>>
	{
		this->_cmds.add(commandIds, [func{std::move(func)}, isDialog{this->_isDialog}](wl::msg::wm_command p) -> LRESULT {
			func();
			return isDialog ? TRUE : 0;
		});
	}

	// https://docs.microsoft.com/en-us/windows/win32/controls/wm-notify
	// Adds a handler to WM_NOTIFY, to a specific control ID, and any notification code.
	// Note: always prefer the specific notify handlers, which are safer.
	// func: [](msg::wm_notify p) -> LRESULT {}
	template<typename F>
	auto wm_notify(WORD ctrlId, int notifCode, F&& func) -> std::enable_if_t<
		std::is_same_v<
			decltype(std::function{std::forward<F>(func)}),
			std::function<LRESULT(wl::msg::wm_notify)>
		>>
	{
		this->_nfys.add({ctrlId, notifCode}, [func{std::move(func)}](wl::msg::wm p) -> LRESULT {
			return func(wl::msg::wm_notify{p});
		});
	}
	// https://docs.microsoft.com/en-us/windows/win32/controls/wm-notify
	// Adds a handler to WM_NOTIFY, to a specific control ID, and any notification code.
	// Note: always prefer the specific notify handlers, which are safer.
	// func: [](msg::wm_notify p) -> LRESULT {}
	template<typename F>
	auto wm_notify(WORD ctrlId, int notifCode, F&& func) -> std::enable_if_t<
		std::is_same_v<
			decltype(std::function{std::forward<F>(func)}),
			std::function<LRESULT()>
		>>
	{
		this->_nfys.add({ctrlId, notifCode}, [func{std::move(func)}](wl::msg::wm p) -> LRESULT {
			return func();
		});
	}

#define WINLAMB_NOTIFY_RET_ZERO(handler, nmcode) \
	template<typename F> \
	auto handler(WORD ctrlId, F&& func) -> std::enable_if_t< \
		std::is_same_v< \
			decltype(std::function{std::forward<F>(func)}), \
			std::function<void(wl::msg::handler)> \
		>> \
	{ \
		this->wm_notify(ctrlId, nmcode, [func{std::move(func)}, isDialog{this->_isDialog}](wl::msg::wm_notify p) -> LRESULT { \
			func(wl::msg::handler{p}); \
			return isDialog ? TRUE : 0; \
		}); \
	} \
	template<typename F> \
		auto handler(WORD ctrlId, F&& func) -> std::enable_if_t< \
		std::is_same_v< \
			decltype(std::function{std::forward<F>(func)}), \
			std::function<void()> \
		>> \
	{ \
		this->wm_notify(ctrlId, nmcode, [func{std::move(func)}, isDialog{this->_isDialog}](wl::msg::wm_notify p) -> LRESULT { \
			func(); \
			return isDialog ? TRUE : 0; \
		}); \
	}

#define WINLAMB_NOTIFY_RET_TYPE(handler, nmcode, rettype) \
	template<typename F> \
	auto handler(WORD ctrlId, F&& func) -> std::enable_if_t< \
		std::is_same_v< \
			decltype(std::function{std::forward<F>(func)}), \
			std::function<rettype(wl::msg::handler)> \
		>> \
	{ \
		this->wm_notify(ctrlId, nmcode, [func{std::move(func)}](wl::msg::wm_notify p) -> LRESULT { \
			return func(wl::msg::handler{p}); \
		}); \
	} \
	template<typename F> \
		auto handler(WORD ctrlId, F&& func) -> std::enable_if_t< \
		std::is_same_v< \
			decltype(std::function{std::forward<F>(func)}), \
			std::function<rettype()> \
		>> \
	{ \
		this->wm_notify(ctrlId, nmcode, [func{std::move(func)}](wl::msg::wm_notify p) -> LRESULT { \
			return func(); \
		}); \
	}

	// --- CBEN ---

	// https://docs.microsoft.com/en-us/windows/win32/controls/cben-beginedit
	// func: [](msg::cben_begin_edit p) {}
	WINLAMB_NOTIFY_RET_ZERO(cben_begin_edit, CBEN_BEGINEDIT)

	// https://docs.microsoft.com/en-us/windows/win32/controls/cben-deleteitem
	// func: [](msg::cben_delete_item p) {}
	WINLAMB_NOTIFY_RET_ZERO(cben_delete_item, CBEN_DELETEITEM)

	// https://docs.microsoft.com/en-us/windows/win32/controls/cben-dragbegin
	// func: [](msg::cben_drag_begin p) {}
	WINLAMB_NOTIFY_RET_ZERO(cben_drag_begin, CBEN_DRAGBEGINW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/cben-endedit
	// func: [](msg::cben_end_edit p) -> bool {}
	WINLAMB_NOTIFY_RET_TYPE(cben_end_edit, CBEN_ENDEDITW, bool)

	// https://docs.microsoft.com/en-us/windows/win32/controls/cben-getdispinfo
	// func: [](msg::cben_get_disp_info p) {}
	WINLAMB_NOTIFY_RET_ZERO(cben_get_disp_info, CBEN_GETDISPINFOW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/cben-insertitem
	// func: [](msg::cben_insert_item p) {}
	WINLAMB_NOTIFY_RET_ZERO(cben_insert_item, CBEN_INSERTITEM)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-setcursor-comboboxex-
	// func: [](msg::cben_set_cursor p) -> int {}
	WINLAMB_NOTIFY_RET_TYPE(cben_set_cursor, NM_SETCURSOR, int)

	// --- DTN ---

	// https://docs.microsoft.com/en-us/windows/win32/controls/dtn-closeup
	// func: [](msg::dtn_close_up p) {}
	WINLAMB_NOTIFY_RET_ZERO(dtn_close_up, DTN_CLOSEUP)

	// https://docs.microsoft.com/en-us/windows/win32/controls/dtn-datetimechange
	// func: [](msg::dtn_date_time_change p) {}
	WINLAMB_NOTIFY_RET_ZERO(dtn_date_time_change, DTN_DATETIMECHANGE)

	// https://docs.microsoft.com/en-us/windows/win32/controls/dtn-dropdown
	// func: [](msg::dtn_drop_down p) {}
	WINLAMB_NOTIFY_RET_ZERO(dtn_drop_down, DTN_DROPDOWN)

	// https://docs.microsoft.com/en-us/windows/win32/controls/dtn-format
	// func: [](msg::dtn_format p) {}
	WINLAMB_NOTIFY_RET_ZERO(dtn_format, DTN_FORMATW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/dtn-formatquery
	// func: [](msg::dtn_format_query p) {}
	WINLAMB_NOTIFY_RET_ZERO(dtn_format_query, DTN_FORMATQUERYW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/dtn-userstring
	// func: [](msg::dtn_user_string p) {}
	WINLAMB_NOTIFY_RET_ZERO(dtn_user_string, DTN_USERSTRINGW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/dtn-wmkeydown
	// func: [](msg::dtn_wm_key_down p) {}
	WINLAMB_NOTIFY_RET_ZERO(dtn_wm_key_down, DTN_WMKEYDOWNW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-killfocus-date-time
	// func: [](msg::dtn_kill_focus p) {}
	WINLAMB_NOTIFY_RET_ZERO(dtn_kill_focus, NM_KILLFOCUS)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-setfocus-date-time-
	// func: [](msg::dtn_set_focus p) {}
	WINLAMB_NOTIFY_RET_ZERO(dtn_set_focus, NM_SETFOCUS)

	// --- LVN ---

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-begindrag
	// func: [](msg::lvn_begin_drag p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_begin_drag, LVN_BEGINDRAG)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-beginlabeledit
	// func: [](msg::lvn_begin_label_edit p) -> bool {}
	WINLAMB_NOTIFY_RET_TYPE(lvn_begin_label_edit, LVN_BEGINLABELEDITW, bool)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-beginrdrag
	// func: [](msg::lvn_begin_r_drag p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_begin_r_drag, LVN_BEGINRDRAG)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-beginscroll
	// func: [](msg::lvn_begin_scroll p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_begin_scroll, LVN_BEGINSCROLL)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-columnclick
	// func: [](msg::lvn_column_click p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_column_click, LVN_COLUMNCLICK)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-columndropdown
	// func: [](msg::lvn_column_drop_down p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_column_drop_down, LVN_COLUMNDROPDOWN)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-columnoverflowclick
	// func: [](msg::lvn_column_overflow_click p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_column_overflow_click, LVN_COLUMNOVERFLOWCLICK)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-deleteallitems
	// func: [](msg::lvn_delete_all_items p) -> bool {}
	WINLAMB_NOTIFY_RET_TYPE(lvn_delete_all_items, LVN_DELETEALLITEMS, bool)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-deleteitem
	// func: [](msg::lvn_delete_item p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_delete_item, LVN_DELETEITEM)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-endlabeledit
	// func: [](msg::lvn_end_label_edit p) -> bool {}
	WINLAMB_NOTIFY_RET_TYPE(lvn_end_label_edit, LVN_ENDLABELEDITW, bool)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-endscroll
	// func: [](msg::lvn_end_scroll p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_end_scroll, LVN_ENDSCROLL)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-getdispinfo
	// func: [](msg::lvn_get_disp_info p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_get_disp_info, LVN_GETDISPINFOW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-getemptymarkup
	// func: [](msg::lvn_get_empty_markup p) -> bool {}
	WINLAMB_NOTIFY_RET_TYPE(lvn_get_empty_markup, LVN_GETEMPTYMARKUP, bool)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-getinfotip
	// func: [](msg::lvn_get_info_tip p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_get_info_tip, LVN_GETINFOTIPW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-hottrack
	// func: [](msg::lvn_hot_track p) -> int {}
	WINLAMB_NOTIFY_RET_TYPE(lvn_hot_track, LVN_HOTTRACK, int)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-incrementalsearch
	// func: [](msg::lvn_incremental_search p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_incremental_search, LVN_INCREMENTALSEARCHW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-insertitem
	// func: [](msg::lvn_insert_item p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_insert_item, LVN_INSERTITEM)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-itemactivate
	// func: [](msg::lvn_item_activate p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_item_activate, LVN_ITEMACTIVATE)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-itemchanged
	// func: [](msg::lvn_item_changed p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_item_changed, LVN_ITEMCHANGED)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-itemchangin
	// func: [](msg::lvn_item_changing p) -> bool {}
	WINLAMB_NOTIFY_RET_TYPE(lvn_item_changing, LVN_ITEMCHANGING, bool)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-keydown
	// func: [](msg::lvn_key_down p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_key_down, LVN_KEYDOWN)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-linkclick
	// func: [](msg::lvn_link_click p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_link_click, LVN_LINKCLICK)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-marqueebegin
	// func: [](msg::lvn_marquee_begin p) -> int {}
	WINLAMB_NOTIFY_RET_TYPE(lvn_marquee_begin, LVN_MARQUEEBEGIN, int)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-odcachehint
	// func: [](msg::lvn_od_cache_hint p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_od_cache_hint, LVN_ODCACHEHINT)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-odfinditem
	// func: [](msg::lvn_od_find_item p) -> int {}
	WINLAMB_NOTIFY_RET_TYPE(lvn_od_find_item, LVN_ODFINDITEMW, int)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-odstatechanged
	// func: [](msg::lvn_od_state_change p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_od_state_changed, LVN_ODSTATECHANGED)

	// https://docs.microsoft.com/en-us/windows/win32/controls/lvn-setdispinfo
	// func: [](msg::lvn_set_disp_info p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_set_disp_info, LVN_SETDISPINFOW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-click-list-view
	// func: [](msg::lvn_click p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_click, NM_CLICK)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-customdraw-list-view
	// func: [](msg::lvn_custom_draw p) -> DWORD {}
	WINLAMB_NOTIFY_RET_TYPE(lvn_custom_draw, NM_CUSTOMDRAW, DWORD)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-dblclk-list-view
	// func: [](msg::lvn_dbl_clk p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_dbl_clk, NM_DBLCLK)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-hover-list-view
	// func: [](msg::lvn_hover p) -> int {}
	WINLAMB_NOTIFY_RET_TYPE(lvn_hover, NM_HOVER, int)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-killfocus-list-view
	// func: [](msg::lvn_kill_focus p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_kill_focus, NM_KILLFOCUS)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-rclick-list-view
	// func: [](msg::lvn_r_click p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_r_click, NM_RCLICK)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-rdblclk-list-view
	// func: [](msg::lvn_r_dbl_clk p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_r_dbl_clk, NM_RDBLCLK)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-releasedcapture-list-view-
	// func: [](msg::lvn_released_capture p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_released_capture, NM_RELEASEDCAPTURE)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-return-list-view-
	// func: [](msg::lvn_return p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_return, NM_RETURN)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-setfocus-list-view-
	// func: [](msg::lvn_set_focus p) {}
	WINLAMB_NOTIFY_RET_ZERO(lvn_set_focus, NM_SETFOCUS)

	// --- MCN ---

	// https://docs.microsoft.com/en-us/windows/win32/controls/mcn-getdaystate
	// func: [](msg::mcn_get_day_state p) {}
	WINLAMB_NOTIFY_RET_ZERO(mcn_get_day_state, MCN_GETDAYSTATE)

	// https://docs.microsoft.com/en-us/windows/win32/controls/mcn-selchange
	// func: [](msg::mcn_sel_change p) {}
	WINLAMB_NOTIFY_RET_ZERO(mcn_sel_change, MCN_SELCHANGE)

	// https://docs.microsoft.com/en-us/windows/win32/controls/mcn-select
	// func: [](msg::mcn_select p) {}
	WINLAMB_NOTIFY_RET_ZERO(mcn_select, MCN_SELECT)

	// https://docs.microsoft.com/en-us/windows/win32/controls/mcn-viewchange
	// func: [](msg::mcn_view_change p) {}
	WINLAMB_NOTIFY_RET_ZERO(mcn_view_change, MCN_VIEWCHANGE)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-releasedcapture-monthcal-
	// func: [](msg::mcn_released_capture p) {}
	WINLAMB_NOTIFY_RET_ZERO(mcn_released_capture, NM_RELEASEDCAPTURE)

	// --- SBN ---

	// https://docs.microsoft.com/en-us/windows/win32/controls/sbn-simplemodechange
	// func: [](msg::sbn_simple_mode_change p) {}
	WINLAMB_NOTIFY_RET_ZERO(sbn_simple_mode_change, SBN_SIMPLEMODECHANGE)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-click-status-bar
	// func: [](msg::sbn_click p) {}
	WINLAMB_NOTIFY_RET_ZERO(sbn_click, NM_CLICK)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-dblclk-status-bar
	// func: [](msg::sbn_dbl_clk p) {}
	WINLAMB_NOTIFY_RET_ZERO(sbn_dbl_clk, NM_DBLCLK)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-rclick-status-bar
	// func: [](msg::sbn_r_click p) {}
	WINLAMB_NOTIFY_RET_ZERO(sbn_r_click, NM_RCLICK)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-rdblclk-status-bar
	// func: [](msg::sbn_r_dbl_clk p) {}
	WINLAMB_NOTIFY_RET_ZERO(sbn_r_dbl_clk, NM_RDBLCLK)

	// --- SLN ---

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-click-syslink
	// func: [](msg::sln_click p) {}
	WINLAMB_NOTIFY_RET_ZERO(sln_click, NM_CLICK)

	// --- TCN ---

	// https://docs.microsoft.com/en-us/windows/win32/controls/tcn-focuschange
	// func: [](msg::tcn_focus_change p) {}
	WINLAMB_NOTIFY_RET_ZERO(tcn_focus_change, TCN_FOCUSCHANGE)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tcn-getobject
	// func: [](msg::tcn_get_object p) {}
	WINLAMB_NOTIFY_RET_ZERO(tcn_get_object, TCN_GETOBJECT)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tcn-keydown
	// func: [](msg::tcn_key_down p) {}
	WINLAMB_NOTIFY_RET_ZERO(tcn_key_down, TCN_KEYDOWN)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tcn-selchange
	// func: [](msg::tcn_sel_change p) {}
	WINLAMB_NOTIFY_RET_ZERO(tcn_sel_change, TCN_SELCHANGE)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tcn-selchanging
	// func: [](msg::tcn_sel_changing p) -> bool {}
	WINLAMB_NOTIFY_RET_TYPE(tcn_sel_changing, TCN_SELCHANGING, bool)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-click-tab
	// func: [](msg::tcn_click p) {}
	WINLAMB_NOTIFY_RET_ZERO(tcn_click, NM_CLICK)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-dblclk-tab
	// func: [](msg::tcn_dbl_clk p) -> int {}
	WINLAMB_NOTIFY_RET_TYPE(tcn_dbl_clk, NM_DBLCLK, int)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-rclick-tab
	// func: [](msg::tcn_r_click p) -> int {}
	WINLAMB_NOTIFY_RET_TYPE(tcn_r_click, NM_RCLICK, int)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-rdblclk-tab
	// func: [](msg::tcn_r_dbl_clk p) -> int {}
	WINLAMB_NOTIFY_RET_TYPE(tcn_r_dbl_clk, NM_RDBLCLK, int)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-releasedcapture-tab-
	// func: [](msg::tcn_released_capture p) {}
	WINLAMB_NOTIFY_RET_ZERO(tcn_released_capture, NM_RELEASEDCAPTURE)

	// --- TRBN ---

	// https://docs.microsoft.com/en-us/windows/win32/controls/trbn-thumbposchanging
	// func: [](msg::trbn_thumb_pos_changing p) {}
	WINLAMB_NOTIFY_RET_ZERO(trbn_thumb_pos_changing, TRBN_THUMBPOSCHANGING)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-customdraw-trackbar
	// func: [](msg::trbn_custom_draw p) -> DWORD {}
	WINLAMB_NOTIFY_RET_TYPE(trbn_custom_draw, NM_CUSTOMDRAW, DWORD)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-releasedcapture-trackbar-
	// func: [](msg::trbn_released_capture p) {}
	WINLAMB_NOTIFY_RET_ZERO(trbn_released_capture, NM_RELEASEDCAPTURE)

	// --- TTN ---

	// https://docs.microsoft.com/en-us/windows/win32/controls/ttn-getdispinfo
	// func: [](msg::ttn_get_disp_info p) {}
	WINLAMB_NOTIFY_RET_ZERO(ttn_get_disp_info, TTN_GETDISPINFOW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/ttn-linkclick
	// func: [](msg::ttn_link_click p) {}
	WINLAMB_NOTIFY_RET_ZERO(ttn_link_click, TTN_LINKCLICK)

	// https://docs.microsoft.com/en-us/windows/win32/controls/ttn-needtext
	// func: [](msg::ttn_need_text p) {}
	WINLAMB_NOTIFY_RET_ZERO(ttn_need_text, TTN_NEEDTEXTW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/ttn-pop
	// func: [](msg::ttn_pop p) {}
	WINLAMB_NOTIFY_RET_ZERO(ttn_pop, TTN_POP)

	// https://docs.microsoft.com/en-us/windows/win32/controls/ttn-show
	// func: [](msg::ttn_show p) {}
	WINLAMB_NOTIFY_RET_ZERO(ttn_show, TTN_SHOW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-customdraw-tooltip
	// func: [](msg::ttn_custom_draw p) -> DWORD {}
	WINLAMB_NOTIFY_RET_TYPE(ttn_custom_draw, NM_CUSTOMDRAW, DWORD)

	// --- TVN ---

	// https://docs.microsoft.com/en-us/windows/win32/controls/tvn-asyncdraw
	// func: [](msg::tvn_async_draw p) {}
	WINLAMB_NOTIFY_RET_ZERO(tvn_async_draw, TVN_ASYNCDRAW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tvn-begindrag
	// func: [](msg::tvn_begin_drag p) {}
	WINLAMB_NOTIFY_RET_ZERO(tvn_begin_drag, TVN_BEGINDRAGW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tvn-beginlabeledit
	// func: [](msg::tvn_begin_label_edit p) -> bool {}
	WINLAMB_NOTIFY_RET_TYPE(tvn_begin_label_edit, TVN_BEGINLABELEDITW, bool)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tvn-beginrdrag
	// func: [](msg::tvn_begin_r_drag p) {}
	WINLAMB_NOTIFY_RET_ZERO(tvn_begin_r_drag, TVN_BEGINRDRAGW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tvn-deleteitem
	// func: [](msg::tvn_delete_item p) {}
	WINLAMB_NOTIFY_RET_ZERO(tvn_delete_item, TVN_DELETEITEMW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tvn-endlabeledit
	// func: [](msg::tvn_end_label_edit p) -> bool {}
	WINLAMB_NOTIFY_RET_TYPE(tvn_end_label_edit, TVN_ENDLABELEDITW, bool)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tvn-getdispinfo
	// func: [](msg::tvn_get_disp_info p) {}
	WINLAMB_NOTIFY_RET_ZERO(tvn_get_disp_info, TVN_GETDISPINFOW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tvn-getinfotip
	// func: [](msg::tvn_get_info_tip p) {}
	WINLAMB_NOTIFY_RET_ZERO(tvn_get_info_tip, TVN_GETINFOTIPW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tvn-itemchanged
	// func: [](msg::tvn_item_changed p) {}
	WINLAMB_NOTIFY_RET_ZERO(tvn_item_changed, TVN_ITEMCHANGEDW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tvn-itemchanging
	// func: [](msg::tvn_item_changing p) -> bool {}
	WINLAMB_NOTIFY_RET_TYPE(tvn_item_changing, TVN_ITEMCHANGINGW, bool)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tvn-itemexpanded
	// func: [](msg::tvn_item_expanded p) {}
	WINLAMB_NOTIFY_RET_ZERO(tvn_item_expanded, TVN_ITEMEXPANDEDW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tvn-itemexpanding
	// func: [](msg::tvn_item_expanding p) -> bool {}
	WINLAMB_NOTIFY_RET_TYPE(tvn_item_expanding, TVN_ITEMEXPANDINGW, bool)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tvn-keydown
	// func: [](msg::tvn_key_down p) -> bool {}
	WINLAMB_NOTIFY_RET_TYPE(tvn_key_down, TVN_KEYDOWN, bool)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tvn-selchanged
	// func: [](msg::tvn_sel_changed p) {}
	WINLAMB_NOTIFY_RET_ZERO(tvn_sel_changed, TVN_SELCHANGEDW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tvn-selchanging
	// func: [](msg::tvn_sel_changing p) -> bool {}
	WINLAMB_NOTIFY_RET_TYPE(tvn_sel_changing, TVN_SELCHANGINGW, bool)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tvn-setdispinfo
	// func: [](msg::tvn_set_disp_info p) {}
	WINLAMB_NOTIFY_RET_ZERO(tvn_set_disp_info, TVN_SETDISPINFOW)

	// https://docs.microsoft.com/en-us/windows/win32/controls/tvn-singleexpand
	// func: [](msg::tvn_single_expand p) -> BYTE {}
	WINLAMB_NOTIFY_RET_TYPE(tvn_single_expand, TVN_SINGLEEXPAND, BYTE)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-click-tree-view
	// func: [](msg::tvn_click p) {}
	WINLAMB_NOTIFY_RET_ZERO(tvn_click, NM_CLICK)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-customdraw-tree-view
	// func: [](msg::tvn_sel_changing p) -> DWORD {}
	WINLAMB_NOTIFY_RET_TYPE(tvn_custom_draw, NM_CUSTOMDRAW, DWORD)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-dblclk-tree-view
	// func: [](msg::tvn_dbl_clk p) {}
	WINLAMB_NOTIFY_RET_ZERO(tvn_dbl_clk, NM_DBLCLK)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-killfocus-tree-view
	// func: [](msg::tvn_kill_focus p) {}
	WINLAMB_NOTIFY_RET_ZERO(tvn_kill_focus, NM_KILLFOCUS)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-rclick-tree-view
	// func: [](msg::tvn_click p) {}
	WINLAMB_NOTIFY_RET_ZERO(tvn_r_click, NM_RCLICK)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-rdblclk-tree-view
	// func: [](msg::tvn_r_dbl_clk p) {}
	WINLAMB_NOTIFY_RET_ZERO(tvn_r_dbl_clk, NM_RDBLCLK)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-return-tree-view-
	// func: [](msg::tvn_return p) {}
	WINLAMB_NOTIFY_RET_ZERO(tvn_return, NM_RETURN)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-setcursor-tree-view-
	// func: [](msg::tvn_set_cursor p) -> int {}
	WINLAMB_NOTIFY_RET_TYPE(tvn_set_cursor, NM_SETCURSOR, int)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-setfocus-tree-view-
	// func: [](msg::tvn_set_focus p) {}
	WINLAMB_NOTIFY_RET_ZERO(tvn_set_focus, NM_SETFOCUS)

	// --- UDN ---

	// https://docs.microsoft.com/en-us/windows/win32/controls/udn-deltapos
	// func: [](msg::udn_delta_pos p) -> int {}
	WINLAMB_NOTIFY_RET_TYPE(udn_delta_pos, UDN_DELTAPOS, int)

	// https://docs.microsoft.com/en-us/windows/win32/controls/nm-releasedcapture-up-down-
	// func: [](msg::udn_released_capture p) {}
	WINLAMB_NOTIFY_RET_ZERO(udn_released_capture, NM_RELEASEDCAPTURE)
};

}//namespace _wli