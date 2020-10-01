/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <algorithm>
#include <string>
#include <vector>
#include <Windows.h>
#include "../menu.h"

// Structs that extract values from window messages.
namespace wl::msg {

// Message parameters to any window message, raw WPARAM and LPARAM values.
struct wm {
	// Raw, unprocessed WPARAM data.
	WPARAM wparam;
	// Raw, unprocessed LPARAM data.
	LPARAM lparam;
};

#define WINLAMB_PARM_EMPTY(msgname) \
	struct msgname : public wm { \
		msgname(wm p) noexcept : wm{p} { } \
	};

#define WINLAMB_PARM_INHERIT(msgname, msgbase) \
	struct msgname : public msgbase { \
		msgname(wm p) noexcept : msgbase{p} { } \
	};

struct wm_activate : public wm {
	wm_activate(wm p) noexcept : wm{p} { }
	[[nodiscard]] bool is_being_activated() const noexcept              { return LOWORD(this->wparam) != WA_INACTIVE; }
	[[nodiscard]] bool is_activated_not_by_mouse_click() const noexcept { return LOWORD(this->wparam) == WA_ACTIVE; }
	[[nodiscard]] bool is_activated_by_mouse_click() const noexcept     { return LOWORD(this->wparam) == WA_CLICKACTIVE; }
	[[nodiscard]] bool is_minimized() const noexcept                    { return HIWORD(this->wparam) != 0; }
	[[nodiscard]] HWND swapped_window() const noexcept                  { return reinterpret_cast<HWND>(this->lparam); }
};

struct wm_activate_app : public wm {
	wm_activate_app(wm p) noexcept : wm{p} { }
	[[nodiscard]] bool  is_being_activated() const noexcept { return this->wparam != FALSE; }
	[[nodiscard]] DWORD thread_id() const noexcept          { return static_cast<DWORD>(this->lparam); }
};

struct wm_app_command : public wm {
	wm_app_command(wm p) noexcept : wm{p} { }
	[[nodiscard]] HWND owner_hwnd() const noexcept  { return reinterpret_cast<HWND>(this->wparam); }
	[[nodiscard]] WORD app_command() const noexcept { return GET_APPCOMMAND_LPARAM(this->lparam); }
	[[nodiscard]] WORD u_device() const noexcept    { return GET_DEVICE_LPARAM(this->lparam); }
	[[nodiscard]] WORD key_state() const noexcept   { return GET_KEYSTATE_LPARAM(this->lparam); }
};

struct wm_ask_cb_format_name : public wm {
	wm_ask_cb_format_name(wm p) noexcept : wm{p} { }
	[[nodiscard]] UINT     buffer_sz() const noexcept { return static_cast<UINT>(this->wparam); }
	[[nodiscard]] wchar_t* buffer() const noexcept    { return reinterpret_cast<wchar_t*>(this->lparam); }
};

WINLAMB_PARM_EMPTY(wm_cancel_mode)

struct wm_capture_changed : public wm {
	wm_capture_changed(wm p) noexcept : wm{p} { }
	[[nodiscard]] HWND hwnd_gaining_mouse() const noexcept { return reinterpret_cast<HWND>(this->lparam); }
};

struct wm_change_cb_chain : public wm {
	wm_change_cb_chain(wm p) noexcept : wm{p} { }
	[[nodiscard]] HWND hwnd_being_removed() const noexcept { return reinterpret_cast<HWND>(this->wparam); }
	[[nodiscard]] HWND next_hwnd() const noexcept          { return reinterpret_cast<HWND>(this->lparam); }
	[[nodiscard]] bool is_last_hwnd() const noexcept       { return this->next_hwnd() == nullptr; }
};

struct wm_char : public wm {
	wm_char(wm p) noexcept : wm{p} { }
	[[nodiscard]] WORD char_code() const noexcept               { return static_cast<WORD>(this->wparam); }
	[[nodiscard]] WORD repeat_count() const noexcept            { return LOWORD(this->lparam); }
	[[nodiscard]] BYTE scan_code() const noexcept               { return LOBYTE(HIWORD(this->lparam)); }
	[[nodiscard]] bool is_extended_key() const noexcept         { return (this->lparam >> 24) & 1; }
	[[nodiscard]] bool has_alt_key() const noexcept             { return (this->lparam >> 29) & 1; }
	[[nodiscard]] bool was_key_previously_down() const noexcept { return (this->lparam >> 30) & 1; }
	[[nodiscard]] bool is_key_being_released() const noexcept   { return (this->lparam >> 31) & 1; }
};

struct wm_char_to_item : public wm {
	wm_char_to_item(wm p) noexcept : wm{p} { }
	[[nodiscard]] WORD char_code() const noexcept         { return LOWORD(this->wparam); }
	[[nodiscard]] WORD current_caret_pos() const noexcept { return HIWORD(this->wparam); }
	[[nodiscard]] HWND hlistbox() const noexcept          { return reinterpret_cast<HWND>(this->lparam); }
};

WINLAMB_PARM_EMPTY(wm_child_activate)

WINLAMB_PARM_EMPTY(wm_clipboard_update)

WINLAMB_PARM_EMPTY(wm_close)

struct wm_command : public wm {
	wm_command(wm p) noexcept : wm{p} { }
	[[nodiscard]] bool is_from_menu() const noexcept        { return HIWORD(this->wparam) == 0; }
	[[nodiscard]] bool is_from_accelerator() const noexcept { return HIWORD(this->wparam) == 1; }
	[[nodiscard]] bool is_from_control() const noexcept     { return !this->is_from_menu() && !this->is_from_accelerator(); }
	[[nodiscard]] int  menu_id() const noexcept             { return this->control_id(); }
	[[nodiscard]] int  accelerator_id() const noexcept      { return this->control_id(); }
	[[nodiscard]] int  control_id() const noexcept          { return LOWORD(this->wparam); }
	[[nodiscard]] int  control_notif_code() const noexcept  { return HIWORD(this->wparam); }
	[[nodiscard]] HWND control_hwnd() const noexcept        { return reinterpret_cast<HWND>(this->lparam); }
};

struct wm_compacting : public wm {
	wm_compacting(wm p) noexcept : wm{p} { }
	[[nodiscard]] UINT cpu_time_ratio() const noexcept { return static_cast<UINT>(this->wparam); }
};

struct wm_compare_item : public wm {
	wm_compare_item(wm p) noexcept : wm{p} { }
	[[nodiscard]] int                      control_id() const noexcept        { return static_cast<int>(this->wparam); }
	[[nodiscard]] const COMPAREITEMSTRUCT& compareitemstruct() const noexcept { return *reinterpret_cast<const COMPAREITEMSTRUCT*>(this->lparam); }
};

struct wm_context_menu : public wm {
	wm_context_menu(wm p) noexcept : wm{p} { }
	[[nodiscard]] HWND  target() const noexcept { return reinterpret_cast<HWND>(this->wparam); }
	[[nodiscard]] POINT pos() const noexcept    { return {LOWORD(this->lparam), HIWORD(this->lparam)}; }
};

struct wm_copy_data : public wm {
	wm_copy_data(wm p) noexcept : wm{p} { }
	[[nodiscard]] const COPYDATASTRUCT& copydatastruct() const noexcept { return *reinterpret_cast<const COPYDATASTRUCT*>(this->lparam); }
};

struct wm_create : public wm {
	wm_create(wm p) noexcept : wm{p} { }
	[[nodiscard]] const CREATESTRUCTW& createstruct() const noexcept { return *reinterpret_cast<const CREATESTRUCTW*>(this->lparam); }
};

struct wm_ctl_color_btn : public wm {
	wm_ctl_color_btn(wm p) noexcept : wm{p} { }
	[[nodiscard]] HDC  hdc() const noexcept  { return reinterpret_cast<HDC>(this->wparam); }
	[[nodiscard]] HWND hctl() const noexcept { return reinterpret_cast<HWND>(this->lparam); }
};

WINLAMB_PARM_INHERIT(wm_ctl_color_dlg, wm_ctl_color_btn)

WINLAMB_PARM_INHERIT(wm_ctl_color_edit, wm_ctl_color_btn)

WINLAMB_PARM_INHERIT(wm_ctl_color_list_box, wm_ctl_color_btn)

WINLAMB_PARM_INHERIT(wm_ctl_color_scroll_bar, wm_ctl_color_btn)

WINLAMB_PARM_INHERIT(wm_ctl_color_static, wm_ctl_color_btn)

WINLAMB_PARM_INHERIT(wm_dead_char, wm_char)

struct wm_delete_item : public wm {
	wm_delete_item(wm p) noexcept : wm{p} { }
	[[nodiscard]] int                     control_id() const noexcept       { return static_cast<int>(this->wparam); }
	[[nodiscard]] const DELETEITEMSTRUCT& deleteitemstruct() const noexcept { return *reinterpret_cast<const DELETEITEMSTRUCT*>(this->lparam); }
};

WINLAMB_PARM_EMPTY(wm_destroy)

WINLAMB_PARM_EMPTY(wm_destroy_clipboard)

struct wm_dev_mode_change : public wm {
	wm_dev_mode_change(wm p) noexcept : wm{p} { }
	[[nodiscard]] const wchar_t* device_name() const noexcept { return reinterpret_cast<const wchar_t*>(this->lparam); }
};

#ifdef _DBT_H // Ras.h
struct wm_device_change : public wm {
	wm_device_change(wm p) : wm(p) { }
	[[nodiscard]] DWORD                                device_event() const noexcept                  { return static_cast<UINT>(this->wparam); }
	[[nodiscard]] const DEV_BROADCAST_HDR&             dev_broadcast_hdr() const noexcept             { return *reinterpret_cast<const DEV_BROADCAST_HDR*>(this->lparam); }
	[[nodiscard]] const DEV_BROADCAST_DEVICEINTERFACE& dev_broadcast_deviceinterface() const noexcept { return *reinterpret_cast<const DEV_BROADCAST_DEVICEINTERFACE*>(this->lparam); }
	[[nodiscard]] const DEV_BROADCAST_HANDLE&          dev_broadcast_handle() const noexcept          { return *reinterpret_cast<const DEV_BROADCAST_HANDLE*>(this->lparam); }
	[[nodiscard]] const DEV_BROADCAST_OEM&             dev_broadcast_oem() const noexcept             { return *reinterpret_cast<const DEV_BROADCAST_OEM*>(this->lparam); }
	[[nodiscard]] const DEV_BROADCAST_PORT&            dev_broadcast_port() const noexcept            { return *reinterpret_cast<const DEV_BROADCAST_PORT*>(this->lparam); }
	[[nodiscard]] const DEV_BROADCAST_VOLUME&          dev_broadcast_volume() const noexcept          { return *reinterpret_cast<const DEV_BROADCAST_VOLUME*>(this->lparam); }
};
#endif

struct wm_display_change : public wm {
	wm_display_change(const wm p) noexcept : wm{p} { }
	[[nodiscard]] UINT bits_per_pixel() const noexcept { return static_cast<UINT>(this->wparam); }
	[[nodiscard]] SIZE sz() const noexcept             { return {LOWORD(this->lparam), HIWORD(this->lparam)}; }
};

WINLAMB_PARM_EMPTY(wm_draw_clipboard)

struct wm_draw_item : public wm {
	wm_draw_item(wm p) noexcept : wm{p} { }
	[[nodiscard]] int                   control_id() const noexcept     { return static_cast<int>(this->wparam); }
	[[nodiscard]] bool                  is_from_menu() const noexcept   { return this->control_id() == 0; }
	[[nodiscard]] const DRAWITEMSTRUCT& drawitemstruct() const noexcept { return *reinterpret_cast<const DRAWITEMSTRUCT*>(this->lparam); }
};

struct wm_drop_files : public wm {
	wm_drop_files(wm p) noexcept : wm{p} { }
	[[nodiscard]] HDROP hdrop() const noexcept { return reinterpret_cast<HDROP>(this->wparam); }
	[[nodiscard]] UINT  count() const noexcept { return DragQueryFileW(this->hdrop(), 0xffff'ffff, nullptr, 0); }

	// Retrieves all files with DragQueryFile() and calls DragFinish().
	[[nodiscard]] std::vector<std::wstring> files() const
	{
		std::vector<std::wstring> files(this->count()); // return vector, sized
		for (size_t i = 0; i < files.size(); ++i) {
			files[i].resize( // alloc path string
				static_cast<size_t>(DragQueryFileW(this->hdrop(), static_cast<UINT>(i), nullptr, 0)) + 1,
				L'\0');
			DragQueryFileW(this->hdrop(), static_cast<UINT>(i), &files[i][0],
				static_cast<UINT>(files[i].length()));
			files[i].resize(files[i].length() - 1); // trim null
		}
		DragFinish(this->hdrop());
		std::sort(files.begin(), files.end()); // make sure files are sorted
		return files;
	}

	[[nodiscard]] POINT pos() const noexcept
	{
		POINT pt{};
		DragQueryPoint(this->hdrop(), &pt);
		return pt;
	}
};

struct wm_enable : public wm {
	wm_enable(wm p) noexcept : wm{p} { }
	[[nodiscard]] bool has_been_enabled() const noexcept { return this->wparam != FALSE; }
};

struct wm_end_session : public wm {
	wm_end_session(wm p) noexcept : wm{p} { }
	[[nodiscard]] bool is_session_being_ended() const noexcept { return this->wparam != FALSE; }
	[[nodiscard]] bool is_system_issue() const noexcept        { return (this->lparam & ENDSESSION_CLOSEAPP) != 0; }
	[[nodiscard]] bool is_forced_critical() const noexcept     { return (this->lparam & ENDSESSION_CRITICAL) != 0; }
	[[nodiscard]] bool is_logoff() const noexcept              { return (this->lparam & ENDSESSION_LOGOFF) != 0; }
	[[nodiscard]] bool is_shutdown() const noexcept            { return this->lparam == 0; }
};

struct wm_enter_idle : public wm {
	wm_enter_idle(wm p) noexcept : wm{p} { }
	[[nodiscard]] bool is_menu_displayed() const noexcept { return this->wparam == MSGF_MENU; }
	[[nodiscard]] HWND hwnd() const noexcept              { return reinterpret_cast<HWND>(this->lparam); }
};

struct wm_enter_menu_loop : public wm {
	wm_enter_menu_loop(wm p) noexcept : wm{p} { }
	[[nodiscard]] bool uses_trackpopupmenu() const noexcept { return this->wparam != FALSE; }
};

WINLAMB_PARM_EMPTY(wm_enter_size_move)

struct wm_erase_bkgnd : public wm {
	wm_erase_bkgnd(wm p) noexcept : wm{p} { }
	[[nodiscard]] HDC hdc() const noexcept { return reinterpret_cast<HDC>(this->wparam); }
};

struct wm_exit_menu_loop : public wm {
	wm_exit_menu_loop(wm p) noexcept : wm{p} { }
	[[nodiscard]] bool is_shortcut_menu() const noexcept { return this->wparam != FALSE; }
};

WINLAMB_PARM_EMPTY(wm_exit_size_move)

WINLAMB_PARM_EMPTY(wm_font_change)

struct wm_get_dlg_code : public wm {
	wm_get_dlg_code(wm p) noexcept : wm{p} { }
	[[nodiscard]] BYTE vkey_code() const noexcept { return static_cast<BYTE>(this->wparam); }
	[[nodiscard]] bool is_query() const noexcept  { return this->lparam == 0; }
	[[nodiscard]] const MSG* msg() const noexcept { return this->is_query() ? nullptr : reinterpret_cast<const MSG*>(this->lparam); }
	[[nodiscard]] bool has_alt() const noexcept   { return (GetAsyncKeyState(VK_MENU) & 0x8000) != 0; }
	[[nodiscard]] bool has_ctrl() const noexcept  { return (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0; }
	[[nodiscard]] bool has_shift() const noexcept { return (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0; }
};

WINLAMB_PARM_EMPTY(wm_get_font)

WINLAMB_PARM_EMPTY(wm_get_hot_key)

struct wm_get_icon : public wm {
	wm_get_icon(wm p) noexcept : wm{p} { }
	[[nodiscard]] BYTE icon_type() const noexcept    { return static_cast<BYTE>(this->wparam); }
	[[nodiscard]] bool is_big() const noexcept       { return this->icon_type() == ICON_BIG; }
	[[nodiscard]] bool is_small() const noexcept     { return this->icon_type() == ICON_SMALL; }
	[[nodiscard]] bool is_small_app() const noexcept { return this->icon_type() == ICON_SMALL2; }
	[[nodiscard]] UINT dpi() const noexcept          { return static_cast<UINT>(this->lparam); }
};

struct wm_get_min_max_info : public wm {
	wm_get_min_max_info(wm p) noexcept : wm{p} { }
	[[nodiscard]] MINMAXINFO& minmaxinfo() const noexcept { return *reinterpret_cast<MINMAXINFO*>(this->lparam); }
};

struct wm_get_title_bar_info_ex : public wm {
	wm_get_title_bar_info_ex(wm p) noexcept : wm{p} { }
	[[nodiscard]] const TITLEBARINFOEX& titlebarinfoex() const noexcept { return *reinterpret_cast<const TITLEBARINFOEX*>(this->lparam); }
};

struct wm_get_text : public wm {
	wm_get_text(wm p) noexcept : wm{p} { }
	[[nodiscard]] UINT     buffer_size() const noexcept { return static_cast<UINT>(this->wparam); }
	[[nodiscard]] wchar_t* buffer() const noexcept      { return reinterpret_cast<wchar_t*>(this->lparam); }
};

WINLAMB_PARM_EMPTY(wm_get_text_length)

struct wm_help : public wm {
	wm_help(wm p) noexcept : wm{p} { }
	[[nodiscard]] const HELPINFO& helpinfo() const noexcept { return *reinterpret_cast<const HELPINFO*>(this->lparam); }
};

struct wm_hot_key : public wm {
	wm_hot_key(wm p) noexcept : wm{p} { }
	[[nodiscard]] bool is_snap_desktop() const noexcept { return this->wparam == IDHOT_SNAPDESKTOP; }
	[[nodiscard]] bool is_snap_window() const noexcept  { return this->wparam == IDHOT_SNAPWINDOW; }
	[[nodiscard]] bool has_alt() const noexcept         { return (LOWORD(this->lparam) & MOD_ALT) != 0; }
	[[nodiscard]] bool has_ctrl() const noexcept        { return (LOWORD(this->lparam) & MOD_CONTROL) != 0; }
	[[nodiscard]] bool has_shift() const noexcept       { return (LOWORD(this->lparam) & MOD_SHIFT) != 0; }
	[[nodiscard]] bool has_win() const noexcept         { return (LOWORD(this->lparam) & MOD_WIN) != 0; }
	[[nodiscard]] BYTE vkey_code() const noexcept       { return static_cast<BYTE>(HIWORD(this->lparam)); }
};

struct wm_h_scroll : public wm {
	wm_h_scroll(wm p) noexcept : wm{p} { }
	[[nodiscard]] WORD scroll_request() const noexcept { return LOWORD(this->wparam); }
	[[nodiscard]] WORD scroll_pos() const noexcept     { return HIWORD(this->wparam); }
	[[nodiscard]] HWND hscrollbar() const noexcept     { return reinterpret_cast<HWND>(this->lparam); }
};

WINLAMB_PARM_INHERIT(wm_h_scroll_clipboard, wm_h_scroll)

struct wm_init_dialog : public wm {
	wm_init_dialog(wm p) noexcept : wm{p} { }
	[[nodiscard]] HWND focused_ctrl() const noexcept { return reinterpret_cast<HWND>(this->wparam); }
};

struct wm_init_menu : public wm {
	wm_init_menu(wm p) noexcept : wm{p} { }
	[[nodiscard]] menu menu() const noexcept { return wl::menu{reinterpret_cast<HMENU>(this->wparam)}; }
};

struct wm_init_menu_popup : public wm {
	wm_init_menu_popup(wm p) noexcept : wm{p} { }
	[[nodiscard]] menu menu() const noexcept               { return wl::menu{reinterpret_cast<HMENU>(this->wparam)}; }
	[[nodiscard]] WORD menu_relative_pos() const noexcept  { return LOWORD(this->lparam); }
	[[nodiscard]] bool is_window_menu() const noexcept     { return HIWORD(this->lparam) != FALSE; }
};

struct wm_input_lang_change : public wm {
	wm_input_lang_change(wm p) noexcept : wm{p} { }
	[[nodiscard]] DWORD new_charset() const noexcept     { return static_cast<DWORD>(this->wparam); }
	[[nodiscard]] HKL   keyboard_layout() const noexcept { return reinterpret_cast<HKL>(this->lparam); }
};

struct wm_input_lang_change_request : public wm {
	wm_input_lang_change_request(wm p) noexcept : wm{p} { }
	[[nodiscard]] WORD new_input_locale() const noexcept { return static_cast<WORD>(this->wparam); }
	[[nodiscard]] HKL  keyboard_layout() const noexcept  { return reinterpret_cast<HKL>(this->lparam); }
};

struct wm_key_down : public wm {
	wm_key_down(wm p) noexcept : wm{p} { }
	[[nodiscard]] BYTE vkey_code() const noexcept           { return static_cast<BYTE>(this->wparam); }
	[[nodiscard]] WORD repeat_count() const noexcept        { return LOWORD(this->lparam); }
	[[nodiscard]] BYTE scan_code() const noexcept           { return LOBYTE(HIWORD(this->lparam)); }
	[[nodiscard]] bool is_extended_key() const noexcept     { return (this->lparam >> 24) & 1; }
	[[nodiscard]] bool context_code() const noexcept        { return (this->lparam >> 29) & 1; }
	[[nodiscard]] bool key_previously_down() const noexcept { return (this->lparam >> 30) & 1; }
	[[nodiscard]] bool transition_state() const noexcept    { return (this->lparam >> 31) & 1; }
};

WINLAMB_PARM_INHERIT(wm_key_up, wm_key_down)

struct wm_kill_focus : public wm {
	wm_kill_focus(wm p) noexcept : wm{p} { }
	[[nodiscard]] HWND focused_hwnd() const noexcept { return reinterpret_cast<HWND>(this->wparam); }
};

struct wm_l_button_dbl_clk : public wm {
	wm_l_button_dbl_clk(wm p) noexcept : wm{p} { }
	[[nodiscard]] WORD  vkeys() const noexcept         { return LOWORD(this->wparam); }
	[[nodiscard]] bool  has_ctrl() const noexcept      { return (this->vkeys() & MK_CONTROL) != 0; }
	[[nodiscard]] bool  has_shift() const noexcept     { return (this->vkeys() & MK_SHIFT) != 0; }
	[[nodiscard]] bool  is_left_btn() const noexcept   { return (this->vkeys() & MK_LBUTTON) != 0; }
	[[nodiscard]] bool  is_middle_btn() const noexcept { return (this->vkeys() & MK_MBUTTON) != 0; }
	[[nodiscard]] bool  is_right_btn() const noexcept  { return (this->vkeys() & MK_RBUTTON) != 0; }
	[[nodiscard]] bool  is_xbtn1() const noexcept      { return (this->vkeys() & MK_XBUTTON1) != 0; }
	[[nodiscard]] bool  is_xbtn2() const noexcept      { return (this->vkeys() & MK_XBUTTON2) != 0; }
	[[nodiscard]] POINT pos() const noexcept           { return {LOWORD(this->lparam), HIWORD(this->lparam)}; }
};

WINLAMB_PARM_INHERIT(wm_l_button_down, wm_l_button_dbl_clk)

WINLAMB_PARM_INHERIT(wm_l_button_up, wm_l_button_dbl_clk)

WINLAMB_PARM_INHERIT(wm_m_button_dbl_clk, wm_l_button_dbl_clk)

WINLAMB_PARM_INHERIT(wm_m_button_down, wm_l_button_dbl_clk)

WINLAMB_PARM_INHERIT(wm_m_button_up, wm_l_button_dbl_clk)

struct wm_mdi_activate : public wm {
	wm_mdi_activate(wm p) noexcept : wm{p} { }
	[[nodiscard]] HWND activated_child() const noexcept   { return reinterpret_cast<HWND>(this->wparam); }
	[[nodiscard]] HWND deactivated_child() const noexcept { return reinterpret_cast<HWND>(this->lparam); }
};

struct wm_measure_item : public wm {
	wm_measure_item(wm p) noexcept : wm{p} { }
	[[nodiscard]] const MEASUREITEMSTRUCT& measureitemstruct() const noexcept { return *reinterpret_cast<const MEASUREITEMSTRUCT*>(this->lparam); }
};

struct wm_menu_char : public wm {
	wm_menu_char(wm p) noexcept : wm{p} { }
	[[nodiscard]] WORD char_code() const noexcept      { return LOWORD(this->wparam); }
	[[nodiscard]] bool is_window_menu() const noexcept { return HIWORD(this->wparam) == MF_SYSMENU; }
	[[nodiscard]] menu menu() const noexcept           { return wl::menu{reinterpret_cast<HMENU>(this->lparam)}; }
};

struct wm_menu_drag : public wm {
	wm_menu_drag(wm p) noexcept : wm{p} { }
	[[nodiscard]] UINT initial_pos() const noexcept { return static_cast<UINT>(this->wparam); }
	[[nodiscard]] menu menu() const noexcept        { return wl::menu{reinterpret_cast<HMENU>(this->lparam)}; }
};

struct wm_menu_get_object : public wm {
	wm_menu_get_object(wm p) noexcept : wm{p} { }
	[[nodiscard]] MENUGETOBJECTINFO& menugetobjectinfo() const noexcept { return *reinterpret_cast<MENUGETOBJECTINFO*>(this->lparam); }
};

struct wm_menu_r_button_up : public wm {
	wm_menu_r_button_up(wm p) noexcept : wm{p} { }
	[[nodiscard]] UINT index() const noexcept { return static_cast<UINT>(this->wparam); }
	[[nodiscard]] menu menu() const noexcept  { return wl::menu{reinterpret_cast<HMENU>(this->lparam)}; }
};

struct wm_menu_select : public wm {
	wm_menu_select(wm p) noexcept : wm{p} { }
	[[nodiscard]] WORD item() const noexcept               { return LOWORD(this->wparam); }
	[[nodiscard]] WORD flags() const noexcept              { return HIWORD(this->wparam); }
	[[nodiscard]] bool system_closed_menu() const noexcept { return HIWORD(this->wparam) == 0xFFFF && !this->lparam; }
};

struct wm_mouse_activate : public wm {
	wm_mouse_activate(wm p) noexcept : wm{p} { }
	[[nodiscard]] short hit_test_code() const noexcept { return static_cast<short>(LOWORD(this->lparam)); }
	[[nodiscard]] WORD  mouse_msg_id() const noexcept  { return HIWORD(this->lparam); }
};

WINLAMB_PARM_INHERIT(wm_mouse_hover, wm_l_button_dbl_clk)

struct wm_mouse_h_wheel : public wm {
	wm_mouse_h_wheel(wm p) noexcept : wm{p} { }
	[[nodiscard]] short wheel_delta() const noexcept   { return GET_WHEEL_DELTA_WPARAM(this->wparam); }
	[[nodiscard]] WORD  vkeys() const noexcept         { return LOWORD(this->wparam); }
	[[nodiscard]] bool  has_ctrl() const noexcept      { return (this->vkeys() & MK_CONTROL) != 0; }
	[[nodiscard]] bool  has_shift() const noexcept     { return (this->vkeys() & MK_SHIFT) != 0; }
	[[nodiscard]] bool  is_left_btn() const noexcept   { return (this->vkeys() & MK_LBUTTON) != 0; }
	[[nodiscard]] bool  is_middle_btn() const noexcept { return (this->vkeys() & MK_MBUTTON) != 0; }
	[[nodiscard]] bool  is_right_btn() const noexcept  { return (this->vkeys() & MK_RBUTTON) != 0; }
	[[nodiscard]] bool  is_xbtn1() const noexcept      { return (this->vkeys() & MK_XBUTTON1) != 0; }
	[[nodiscard]] bool  is_xbtn2() const noexcept      { return (this->vkeys() & MK_XBUTTON2) != 0; }
	[[nodiscard]] POINT pos() const noexcept           { return {LOWORD(this->lparam), HIWORD(this->lparam)}; }
};

WINLAMB_PARM_EMPTY(wm_mouse_leave)

WINLAMB_PARM_INHERIT(wm_mouse_move, wm_l_button_dbl_clk)

WINLAMB_PARM_INHERIT(wm_mouse_wheel, wm_mouse_h_wheel)

struct wm_move : public wm {
	wm_move(wm p) noexcept : wm{p} { }
	[[nodiscard]] POINT client_area_pos() const noexcept { return {LOWORD(this->lparam), HIWORD(this->lparam)}; }
};

struct wm_moving : public wm {
	wm_moving(wm p) noexcept : wm{p} { }
	[[nodiscard]] RECT& window_pos() const noexcept { return *reinterpret_cast<RECT*>(this->lparam); }
};

struct wm_nc_activate : public wm {
	wm_nc_activate(wm p) noexcept : wm{p} { }
	[[nodiscard]] bool is_active() const noexcept { return this->wparam == TRUE; }
};

struct wm_nc_calc_size : public wm {
	wm_nc_calc_size(wm p) noexcept : wm{p} { }
	[[nodiscard]] bool                     is_nccalcsize() const noexcept     { return this->wparam == TRUE; }
	[[nodiscard]] bool                     is_rect() const noexcept           { return this->wparam == FALSE; }
	[[nodiscard]] const NCCALCSIZE_PARAMS& nccalcsize_params() const noexcept { return *reinterpret_cast<const NCCALCSIZE_PARAMS*>(this->lparam); }
	[[nodiscard]] const RECT&              rect() const noexcept              { return *reinterpret_cast<const RECT*>(this->lparam); }
};

WINLAMB_PARM_INHERIT(wm_nc_create, wm_create)

WINLAMB_PARM_EMPTY(wm_nc_destroy)

struct wm_nc_hit_test : public wm {
	wm_nc_hit_test(wm p) noexcept : wm{p} { }
	[[nodiscard]] POINT cursor_pos() const noexcept { return {LOWORD(this->lparam), HIWORD(this->lparam)}; }
};

struct wm_nc_l_button_dbl_clk : public wm {
	wm_nc_l_button_dbl_clk(wm p) noexcept : wm{p} { }
	[[nodiscard]] short hit_test_code() const noexcept { return static_cast<short>(this->wparam); }
	[[nodiscard]] POINT cursor_pos() const noexcept    { return {LOWORD(this->lparam), HIWORD(this->lparam)}; }
};

WINLAMB_PARM_INHERIT(wm_nc_l_button_down, wm_nc_l_button_dbl_clk)

WINLAMB_PARM_INHERIT(wm_nc_l_button_up, wm_nc_l_button_dbl_clk)

WINLAMB_PARM_INHERIT(wm_nc_m_button_dbl_clk, wm_nc_l_button_dbl_clk)

WINLAMB_PARM_INHERIT(wm_nc_m_button_down, wm_nc_l_button_dbl_clk)

WINLAMB_PARM_INHERIT(wm_nc_m_button_up, wm_nc_l_button_dbl_clk)

WINLAMB_PARM_INHERIT(wm_nc_mouse_hover, wm_nc_l_button_dbl_clk)

WINLAMB_PARM_EMPTY(wm_nc_mouse_leave)

WINLAMB_PARM_INHERIT(wm_nc_mouse_move, wm_nc_l_button_dbl_clk)

struct wm_nc_paint : public wm {
	wm_nc_paint(wm p) noexcept : wm{p} { }
	[[nodiscard]] HRGN updated_region() const noexcept { return reinterpret_cast<HRGN>(this->wparam); }
};

WINLAMB_PARM_INHERIT(wm_nc_r_button_dbl_clk, wm_nc_l_button_dbl_clk)

WINLAMB_PARM_INHERIT(wm_nc_r_button_down, wm_nc_l_button_dbl_clk)

WINLAMB_PARM_INHERIT(wm_nc_r_button_up, wm_nc_l_button_dbl_clk)

struct wm_nc_x_button_dbl_clk : public wm {
	wm_nc_x_button_dbl_clk(wm p) noexcept : wm{p} { }
	[[nodiscard]] short hit_test_code() const noexcept { return LOWORD(this->wparam); }
	[[nodiscard]] bool  is_xbtn1() const noexcept      { return HIWORD(this->wparam) == XBUTTON1; }
	[[nodiscard]] bool  is_xbtn2() const noexcept      { return HIWORD(this->wparam) == XBUTTON2; }
	[[nodiscard]] POINT cursor_pos() const noexcept    { return {LOWORD(this->lparam), HIWORD(this->lparam)}; }
};

WINLAMB_PARM_INHERIT(wm_nc_x_button_down, wm_nc_x_button_dbl_clk)

WINLAMB_PARM_INHERIT(wm_nc_x_button_up, wm_nc_x_button_dbl_clk)

struct wm_next_dlg_ctl : public wm {
	wm_next_dlg_ctl(wm p) noexcept : wm{p} { }
	[[nodiscard]] bool has_ctrl_receiving_focus() const noexcept { return LOWORD(this->lparam) != FALSE; }
	[[nodiscard]] HWND ctrl_receiving_focus() const noexcept     { return LOWORD(this->lparam) ? reinterpret_cast<HWND>(this->wparam) : nullptr; }
	[[nodiscard]] bool focus_next() const noexcept               { return this->wparam == 0; }
};

struct wm_next_menu : public wm {
	wm_next_menu(wm p) noexcept : wm{p} { }
	[[nodiscard]] BYTE               vkey_code() const noexcept   { return static_cast<BYTE>(this->wparam); }
	[[nodiscard]] const MDINEXTMENU& mdinextmenu() const noexcept { return *reinterpret_cast<const MDINEXTMENU*>(this->lparam); }
};

struct wm_notify_format : public wm {
	wm_notify_format(wm p) noexcept : wm{p} { }
	[[nodiscard]] HWND hwnd_from() const noexcept             { return reinterpret_cast<HWND>(this->wparam); }
	[[nodiscard]] bool is_query_from_control() const noexcept { return this->lparam == NF_QUERY; }
	[[nodiscard]] bool is_requery_to_control() const noexcept { return this->lparam == NF_REQUERY; }
};

WINLAMB_PARM_EMPTY(wm_paint)

struct wm_paint_clipboard : public wm {
	wm_paint_clipboard(wm p) noexcept : wm{p} { }
	[[nodiscard]] HWND               clipboard_viewer() const noexcept { return reinterpret_cast<HWND>(this->wparam); }
	[[nodiscard]] const PAINTSTRUCT& paintstruct() const noexcept      { return *reinterpret_cast<const PAINTSTRUCT*>(this->lparam); }
};

struct wm_palette_changed : public wm {
	wm_palette_changed(wm p) noexcept : wm{p} { }
	[[nodiscard]] HWND hwnd_origin() const noexcept { return reinterpret_cast<HWND>(this->wparam); }
};

WINLAMB_PARM_INHERIT(wm_palette_is_changing, wm_palette_changed)

struct wm_parent_notify : public wm {
	wm_parent_notify(wm p) noexcept : wm{p} { }
	[[nodiscard]] UINT  event_message() const noexcept { return static_cast<UINT>(LOWORD(this->wparam)); }
	[[nodiscard]] WORD  child_id() const noexcept      { return HIWORD(this->wparam); }
	[[nodiscard]] HWND  child_hwnd() const noexcept    { return reinterpret_cast<HWND>(this->lparam); }
	[[nodiscard]] POINT pos() const noexcept           { return {LOWORD(this->lparam), HIWORD(this->lparam)}; }
	[[nodiscard]] bool  is_xbtn1() const noexcept      { return HIWORD(this->wparam) == XBUTTON1; }
	[[nodiscard]] bool  is_xbtn2() const noexcept      { return HIWORD(this->wparam) == XBUTTON2; }
	[[nodiscard]] WORD  pointer_flag() const noexcept  { return HIWORD(this->wparam); }
};

struct wm_power_broadcast : public wm {
	wm_power_broadcast(wm p) noexcept : wm{p} { }
	[[nodiscard]] bool                          is_power_status_change() const noexcept  { return this->wparam == PBT_APMPOWERSTATUSCHANGE; }
	[[nodiscard]] bool                          is_resuming() const noexcept             { return this->wparam == PBT_APMRESUMEAUTOMATIC; }
	[[nodiscard]] bool                          is_suspending() const noexcept           { return this->wparam == PBT_APMSUSPEND; }
	[[nodiscard]] bool                          is_power_setting_change() const noexcept { return this->wparam == PBT_POWERSETTINGCHANGE; }
	[[nodiscard]] const POWERBROADCAST_SETTING& power_setting() const noexcept           { return *reinterpret_cast<const POWERBROADCAST_SETTING*>(this->lparam); }
};

struct wm_print : public wm {
	wm_print(wm p) noexcept : wm{p} { }
	[[nodiscard]] HDC  hdc() const noexcept   { return reinterpret_cast<HDC>(this->wparam); }
	[[nodiscard]] UINT flags() const noexcept { return static_cast<UINT>(this->lparam); }
};

WINLAMB_PARM_INHERIT(wm_print_client, wm_print)

WINLAMB_PARM_EMPTY(wm_query_drag_icon)

struct wm_query_end_session : public wm {
	wm_query_end_session(wm p) noexcept : wm{p} { }
	[[nodiscard]] bool is_system_issue() const noexcept    { return (this->lparam & ENDSESSION_CLOSEAPP) != 0; }
	[[nodiscard]] bool is_forced_critical() const noexcept { return (this->lparam & ENDSESSION_CRITICAL) != 0; }
	[[nodiscard]] bool is_logoff() const noexcept          { return (this->lparam & ENDSESSION_LOGOFF) != 0; }
	[[nodiscard]] bool is_shutdown() const noexcept        { return this->lparam == 0; }
};

WINLAMB_PARM_EMPTY(wm_query_new_palette)

WINLAMB_PARM_EMPTY(wm_query_open)

WINLAMB_PARM_INHERIT(wm_r_button_dbl_clk, wm_l_button_dbl_clk)

WINLAMB_PARM_INHERIT(wm_r_button_down, wm_l_button_dbl_clk)

WINLAMB_PARM_INHERIT(wm_r_button_up, wm_l_button_dbl_clk)

#ifdef _RAS_H_ // Ras.h
struct wm_ras_dial_event : public wm {
	wm_ras_dial_event(wm p) : wm{p} { }
	[[nodiscard]] RASCONNSTATE rasconnstate() const noexcept { return static_cast<RASCONNSTATE>(this->wparam); }
	[[nodiscard]] DWORD        error() const noexcept        { return static_cast<DWORD>(this->lparam); }
};
#endif

WINLAMB_PARM_EMPTY(wm_render_all_formats)

struct wm_render_format : public wm {
	wm_render_format(wm p) noexcept : wm{p} { }
	[[nodiscard]] WORD clipboard_format() const noexcept { return static_cast<WORD>(this->wparam); }
};

struct wm_set_cursor : public wm {
	wm_set_cursor(wm p) noexcept : wm{p} { }
	[[nodiscard]] HWND  cursor_owner() const noexcept  { return reinterpret_cast<HWND>(this->wparam); }
	[[nodiscard]] short hit_test_code() const noexcept { return static_cast<short>(LOWORD(this->lparam)); }
	[[nodiscard]] WORD  mouse_msg_id() const noexcept  { return HIWORD(this->lparam); }
};

struct wm_set_focus : public wm {
	wm_set_focus(wm p) noexcept : wm{p} { }
	[[nodiscard]] HWND hwnd_losing_focus() const noexcept { return reinterpret_cast<HWND>(this->wparam); }
};

struct wm_set_font : public wm {
	wm_set_font(wm p) noexcept : wm{p} { }
	[[nodiscard]] HFONT hfont() const noexcept   { return reinterpret_cast<HFONT>(this->wparam); }
	[[nodiscard]] bool  should_redraw() noexcept { return LOWORD(this->lparam) != FALSE; }
};

struct wm_set_hot_key : public wm {
	wm_set_hot_key(wm p) noexcept : wm{p} { }
	[[nodiscard]] BYTE vkey_code() const noexcept    { return static_cast<BYTE>(LOWORD(this->wparam)); }
	[[nodiscard]] bool has_alt() const noexcept      { return (HIWORD(this->wparam) & HOTKEYF_ALT) != 0; }
	[[nodiscard]] bool has_ctrl() const noexcept     { return (HIWORD(this->wparam) & HOTKEYF_CONTROL) != 0; }
	[[nodiscard]] bool has_extended() const noexcept { return (HIWORD(this->wparam) & HOTKEYF_EXT) != 0; }
	[[nodiscard]] bool has_shift() const noexcept    { return (HIWORD(this->wparam) & HOTKEYF_SHIFT) != 0; }
};

struct wm_set_icon : public wm {
	wm_set_icon(wm p) noexcept : wm{p} { }
	[[nodiscard]] bool  is_small() const noexcept   { return this->wparam == ICON_SMALL; }
	[[nodiscard]] HICON hicon() const noexcept      { return reinterpret_cast<HICON>(this->lparam); }
	[[nodiscard]] bool  is_removed() const noexcept { return this->hicon() == nullptr; }
};

struct wm_set_redraw : public wm {
	wm_set_redraw(wm p) noexcept : wm{p} { }
	[[nodiscard]] bool can_redraw() const noexcept { return this->wparam != FALSE; }
};

struct wm_set_text : public wm {
	wm_set_text(wm p) noexcept : wm{p} { }
	[[nodiscard]] const wchar_t* text() const noexcept { return reinterpret_cast<const wchar_t*>(this->lparam); }
};

struct wm_setting_change : public wm {
	wm_setting_change(wm p) noexcept : wm{p} { }
	[[nodiscard]] const wchar_t* string_id() const noexcept           { return reinterpret_cast<const wchar_t*>(this->lparam); }
	[[nodiscard]] bool           is_policy() const noexcept           { return !lstrcmpW(this->string_id(), L"Policy"); }
	[[nodiscard]] bool           is_locale() const noexcept           { return !lstrcmpW(this->string_id(), L"intl"); }
	[[nodiscard]] bool           is_environment_vars() const noexcept { return !lstrcmpW(this->string_id(), L"Environment"); }
};

struct wm_show_window : public wm {
	wm_show_window(wm p) noexcept : wm{p} { }
	[[nodiscard]] BYTE status() const noexcept                   { return static_cast<BYTE>(this->lparam); }
	[[nodiscard]] bool is_being_shown() const noexcept           { return this->wparam != FALSE; }
	[[nodiscard]] bool is_other_away() const noexcept            { return this->status() == SW_OTHERUNZOOM; }
	[[nodiscard]] bool is_other_over() const noexcept            { return this->status() == SW_OTHERZOOM; }
	[[nodiscard]] bool is_owner_being_minimized() const noexcept { return this->status() == SW_PARENTCLOSING; }
	[[nodiscard]] bool is_owner_being_restored() const noexcept  { return this->status() == SW_PARENTOPENING; }
};

struct wm_size : public wm {
	wm_size(wm p) noexcept : wm{p} { }
	[[nodiscard]] DWORD request() const noexcept            { return static_cast<DWORD>(this->wparam); }
	[[nodiscard]] bool  is_other_maximized() const noexcept { return this->request() == SIZE_MAXHIDE; }
	[[nodiscard]] bool  is_maximized() const noexcept       { return this->request() == SIZE_MAXIMIZED; }
	[[nodiscard]] bool  is_other_restored() const noexcept  { return this->request() == SIZE_MAXSHOW; }
	[[nodiscard]] bool  is_minimized() const noexcept       { return this->request() == SIZE_MINIMIZED; }
	[[nodiscard]] bool  is_restored() const noexcept        { return this->request() == SIZE_RESTORED; }
	[[nodiscard]] SIZE  client_area_size() const noexcept   { return {LOWORD(this->lparam), HIWORD(this->lparam)}; }
};

struct wm_size_clipboard : public wm {
	wm_size_clipboard(wm p) noexcept : wm{p} { }
	[[nodiscard]] HWND        clipboard_viewer() const noexcept { return reinterpret_cast<HWND>(this->wparam); }
	[[nodiscard]] const RECT& clipboard_rect() const noexcept   { return *reinterpret_cast<const RECT*>(this->lparam); }
};

struct wm_sizing : public wm {
	wm_sizing(wm p) noexcept : wm{p} { }
	[[nodiscard]] WORD  edge() const noexcept          { return static_cast<WORD>(this->wparam); }
	[[nodiscard]] RECT& screen_coords() const noexcept { return *reinterpret_cast<RECT*>(this->lparam); }
};

struct wm_spooler_status : public wm {
	wm_spooler_status(wm p) noexcept : wm{p} { }
	[[nodiscard]] UINT status_flag() const noexcept    { return static_cast<UINT>(this->wparam); }
	[[nodiscard]] WORD remaining_jobs() const noexcept { return LOWORD(this->lparam); }
};

struct wm_style_changed : public wm {
	wm_style_changed(wm p) noexcept : wm{p} { }
	[[nodiscard]] int                change() const noexcept      { return static_cast<int>(this->wparam); }
	[[nodiscard]] bool               is_style() const noexcept    { return (this->change() & GWL_STYLE) != 0; }
	[[nodiscard]] bool               is_ex_style() const noexcept { return (this->change() & GWL_EXSTYLE) != 0; }
	[[nodiscard]] const STYLESTRUCT& stylestruct() const noexcept { return *reinterpret_cast<const STYLESTRUCT*>(this->lparam); }
};

WINLAMB_PARM_INHERIT(wm_style_changing, wm_style_changed)

WINLAMB_PARM_EMPTY(wm_sync_paint)

WINLAMB_PARM_INHERIT(wm_sys_char, wm_char)

WINLAMB_PARM_EMPTY(wm_sys_color_change)

struct wm_sys_command : public wm {
	wm_sys_command(wm p) noexcept : wm{p} { }
	[[nodiscard]] WORD  command_type() const noexcept { return static_cast<WORD>(this->wparam); }
	[[nodiscard]] POINT cursor_pos() const noexcept          { return {LOWORD(this->lparam), HIWORD(this->lparam)}; }
};

WINLAMB_PARM_INHERIT(wm_sys_dead_char, wm_char)

WINLAMB_PARM_INHERIT(wm_sys_key_down, wm_key_down)

WINLAMB_PARM_INHERIT(wm_sys_key_up, wm_key_down)

struct wm_t_card : public wm {
	wm_t_card(wm p) noexcept : wm{p} { }
	[[nodiscard]] UINT action_id() const noexcept   { return static_cast<UINT>(this->wparam); }
	[[nodiscard]] long action_data() const noexcept { return static_cast<long>(this->lparam); }
};

WINLAMB_PARM_EMPTY(wm_time_change)

struct wm_timer : public wm {
	wm_timer(wm p) noexcept : wm{p} { }
	[[nodiscard]] UINT_PTR  timer_id() const noexcept { return static_cast<UINT_PTR>(this->wparam); }
	[[nodiscard]] TIMERPROC callback() const noexcept { return reinterpret_cast<TIMERPROC>(this->lparam); }
};

struct wm_un_init_menu_popup : public wm {
	wm_un_init_menu_popup(wm p) noexcept : wm{p} { }
	[[nodiscard]] menu menu() const noexcept    { return wl::menu{reinterpret_cast<HMENU>(this->wparam)}; }
	[[nodiscard]] WORD menu_id() const noexcept { return HIWORD(this->lparam); }
};

WINLAMB_PARM_INHERIT(wm_v_scroll, wm_h_scroll)

WINLAMB_PARM_INHERIT(wm_v_scroll_clipboard, wm_h_scroll)

struct wm_vkey_to_item : public wm {
	wm_vkey_to_item(wm p) noexcept : wm{p} { }
	[[nodiscard]] BYTE vkey_code() const noexcept         { return static_cast<BYTE>(LOWORD(this->wparam)); }
	[[nodiscard]] WORD current_caret_pos() const noexcept { return HIWORD(this->wparam); }
	[[nodiscard]] HWND hlistbox() const noexcept          { return reinterpret_cast<HWND>(this->lparam); }
};

struct wm_window_pos_changed : public wm {
	wm_window_pos_changed(wm p) noexcept : wm{p} { }
	[[nodiscard]] const WINDOWPOS& windowpos() const noexcept { return *reinterpret_cast<const WINDOWPOS*>(this->lparam); }
};

WINLAMB_PARM_INHERIT(wm_window_pos_changing, wm_window_pos_changed)

}//namespace wl::msg