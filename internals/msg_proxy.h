/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <type_traits>
#include <Windows.h>
#include "param_wm.h"
#include "store.h"

namespace _wli {

// Exposes specific methods to add message handlers.
class msg_proxy {
private:
	store<UINT>& _msgs;

protected:
	bool _isDialog;

public:
	msg_proxy(store<UINT>& msgs, bool isDialog) :
		_msgs{msgs}, _isDialog{isDialog} { }

	// Adds a handler to an arbitrary window message.
	// Note: always prefer the specific message handlers, which are safer.
	// func: [](msg::wm p) -> LRESULT {}
	template<typename F>
	auto wm(UINT message, F&& func) -> std::enable_if_t<
		std::is_same_v<
			decltype(std::function{std::forward<F>(func)}),
			std::function<LRESULT(wl::msg::wm)>
		>>
	{
		// https://stackoverflow.com/a/64030013/6923555
		this->_msgs.add(message, std::move(func));
	}
	// Adds a handler to an arbitrary window message.
	// Note: always prefer the specific message handlers, which are safer.
	// func: [](msg::wm p) -> LRESULT {}
	template<typename F>
	auto wm(std::initializer_list<UINT> messages, F&& func) -> std::enable_if_t<
		std::is_same_v<
			decltype(std::function{std::forward<F>(func)}),
			std::function<LRESULT(wl::msg::wm)>
		>>
	{
		this->_msgs.add(messages, std::move(func));
	}

#define WINLAMB_MSG_RET_VAL(handler, dwmsg, RET_NUMBER) \
	template<typename F> \
	auto handler(F&& func) -> std::enable_if_t< \
		std::is_same_v< \
			decltype(std::function{std::forward<F>(func)}), \
			std::function<void(wl::msg::handler)> \
		>> \
	{ \
		this->wm(dwmsg, [func{std::move(func)}, isDialog{this->_isDialog}](wl::msg::wm p) -> LRESULT { \
			func(wl::msg::handler{p}); \
			return RET_NUMBER; \
		}); \
	} \
	template<typename F> \
	auto handler(F&& func) -> std::enable_if_t< \
		std::is_same_v< \
			decltype(std::function{std::forward<F>(func)}), \
			std::function<void()> \
		>> \
	{ \
		this->wm(dwmsg, [func{std::move(func)}, isDialog{this->_isDialog}](wl::msg::wm p) -> LRESULT { \
			func(); \
			return RET_NUMBER; \
		}); \
	}

#define WINLAMB_MSG_RET_ZERO(handler, dwmsg) \
	WINLAMB_MSG_RET_VAL(handler, dwmsg, isDialog ? TRUE : 0)

#define WINLAMB_MSG_RET_TYPE(handler, dwmsg, rettype) \
	template<typename F> \
	auto handler(F&& func) -> std::enable_if_t< \
		std::is_same_v< \
			decltype(std::function{std::forward<F>(func)}), \
			std::function<rettype(wl::msg::handler)> \
		>> \
	{ \
		this->wm(dwmsg, [func{std::move(func)}](wl::msg::wm p) -> LRESULT { \
			return func(wl::msg::handler{p}); \
		}); \
	} \
	template<typename F> \
	auto handler(F&& func) -> std::enable_if_t< \
		std::is_same_v< \
			decltype(std::function{std::forward<F>(func)}), \
			std::function<rettype()> \
		>> \
	{ \
		this->wm(dwmsg, [func{std::move(func)}](wl::msg::wm p) -> LRESULT { \
			return func(); \
		}); \
	}

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-activate
	// func: [](msg::wm_activate p) {}
	// Warning, default handled in:
	// - window_main
	WINLAMB_MSG_RET_ZERO(wm_activate, WM_ACTIVATE)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-activateapp
	// func: [](msg::wm_activate_app p) {}
	WINLAMB_MSG_RET_ZERO(wm_activate_app, WM_ACTIVATEAPP)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-appcommand
	// func: [](msg::wm_app_command p) {}
	WINLAMB_MSG_RET_VAL(wm_app_command, WM_APPCOMMAND, TRUE)

	// https://docs.microsoft.com/en-us/windows/win32/dataxchg/wm-askcbformatname
	// func: [](msg::wm_ask_cb_format_name p) {}
	WINLAMB_MSG_RET_ZERO(wm_ask_cb_format_name, WM_ASKCBFORMATNAME)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-cancelmode
	// func: [](msg::wm_cancel_mode p) {}
	WINLAMB_MSG_RET_ZERO(wm_cancel_mode, WM_CANCELMODE)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-capturechanged
	// func: [](msg::wm_capture_changed p) {}
	WINLAMB_MSG_RET_ZERO(wm_capture_changed, WM_CAPTURECHANGED)

	// https://docs.microsoft.com/en-us/windows/win32/dataxchg/wm-changecbchain
	// func: [](msg::wm_change_cb_chain p) {}
	WINLAMB_MSG_RET_ZERO(wm_change_cb_chain, WM_CHANGECBCHAIN)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-char
	// func: [](msg::wm_char p) {}
	WINLAMB_MSG_RET_ZERO(wm_char, WM_CHAR)

	// https://docs.microsoft.com/en-us/windows/win32/controls/wm-chartoitem
	// func: [](msg::wm_char_to_item p) -> int {}
	WINLAMB_MSG_RET_TYPE(wm_char_to_item, WM_CHARTOITEM, int)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-childactivate
	// func: [](msg::wm_child_activate p) {}
	WINLAMB_MSG_RET_ZERO(wm_child_activate, WM_CHILDACTIVATE)

	// https://docs.microsoft.com/en-us/windows/win32/dataxchg/wm-clipboardupdate
	// func: [](msg::wm_clipboard_update p) {}
	WINLAMB_MSG_RET_ZERO(wm_clipboard_update, WM_CLIPBOARDUPDATE)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-close
	// func: [](msg::wm_close p) {}
	// Warning, default handled in:
	// - window_modal
	// - window_modeless
	// - dialog_main
	// - dialog_modal
	// - dialog_modeless
	WINLAMB_MSG_RET_ZERO(wm_close, WM_CLOSE)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-compacting
	// func: [](msg::wm_compacting p) {}
	WINLAMB_MSG_RET_ZERO(wm_compacting, WM_COMPACTING)

	// https://docs.microsoft.com/en-us/windows/win32/controls/wm-compareitem
	// func: [](msg::wm_compare_item p) -> int {}
	WINLAMB_MSG_RET_TYPE(wm_compare_item, WM_COMPAREITEM, int)

	// https://docs.microsoft.com/en-us/windows/win32/menurc/wm-contextmenu
	// func: [](msg::wm_context_menu p) {}
	WINLAMB_MSG_RET_ZERO(wm_context_menu, WM_CONTEXTMENU)

	// https://docs.microsoft.com/en-us/windows/win32/dataxchg/wm-copydata
	// func: [](msg::wm_copy_data p) -> bool {}
	WINLAMB_MSG_RET_TYPE(wm_copy_data, WM_CREATE, bool)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-create
	// func: [](msg::wm_create p) -> int {}
	WINLAMB_MSG_RET_TYPE(wm_create, WM_CREATE, int)

	// https://docs.microsoft.com/en-us/windows/win32/controls/wm-ctlcolorbtn
	// func: [](msg::wm_ctl_color_btn p) -> HBRUSH {}
	WINLAMB_MSG_RET_TYPE(wm_ctl_color_btn, WM_CTLCOLORBTN, HBRUSH)

	// https://docs.microsoft.com/en-us/windows/win32/dlgbox/wm-ctlcolordlg
	// func: [](msg::wm_ctl_color_dlg p) -> HBRUSH {}
	WINLAMB_MSG_RET_TYPE(wm_ctl_color_dlg, WM_CTLCOLORDLG, HBRUSH)

	// https://docs.microsoft.com/en-us/windows/win32/controls/wm-ctlcoloredit
	// func: [](msg::wm_ctl_color_edit p) -> HBRUSH {}
	WINLAMB_MSG_RET_TYPE(wm_ctl_color_edit, WM_CTLCOLOREDIT, HBRUSH)

	// https://docs.microsoft.com/en-us/windows/win32/controls/wm-ctlcolorlistbox
	// func: [](msg::wm_ctl_color_list_box p) -> HBRUSH {}
	WINLAMB_MSG_RET_TYPE(wm_ctl_color_list_box, WM_CTLCOLORLISTBOX, HBRUSH)

	// https://docs.microsoft.com/en-us/windows/win32/controls/wm-ctlcolorscrollbar
	// func: [](msg::wm_ctl_color_scroll_bar p) -> HBRUSH {}
	WINLAMB_MSG_RET_TYPE(wm_ctl_color_scroll_bar, WM_CTLCOLORSCROLLBAR, HBRUSH)

	// https://docs.microsoft.com/en-us/windows/win32/controls/wm-ctlcolorstatic
	// func: [](msg::wm_ctl_color_static p) -> HBRUSH {}
	WINLAMB_MSG_RET_TYPE(wm_ctl_color_static, WM_CTLCOLORSTATIC, HBRUSH)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-deadchar
	// func: [](msg::wm_dead_char p) {}
	WINLAMB_MSG_RET_ZERO(wm_dead_char, WM_DEADCHAR)

	// https://docs.microsoft.com/en-us/windows/win32/controls/wm-deleteitem
	// func: [](msg::wm_delete_item p) {}
	WINLAMB_MSG_RET_VAL(wm_delete_item, WM_DELETEITEM, TRUE)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-destroy
	// func: [](msg::wm_destroy p) {}
	WINLAMB_MSG_RET_ZERO(wm_destroy, WM_DESTROY)

	// https://docs.microsoft.com/en-us/windows/win32/dataxchg/wm-destroyclipboard
	// func: [](msg::wm_destroy_clipboard p) {}
	WINLAMB_MSG_RET_ZERO(wm_destroy_clipboard, WM_DESTROYCLIPBOARD)

	// https://docs.microsoft.com/en-us/windows/win32/gdi/wm-devmodechange
	// func: [](msg::wm_dev_mode_change p) {}
	WINLAMB_MSG_RET_ZERO(wm_dev_mode_change, WM_DEVMODECHANGE)

#ifdef _DBT_H // Ras.h
	// https://docs.microsoft.com/en-us/windows/win32/devio/wm-devicechange
	// func: [](msg::wm_device_change p) -> WORD {}
	WINLAMB_MSG_RET_TYPE(wm_device_change, WM_DEVICECHANGE, WORD)
#endif

	// https://docs.microsoft.com/en-us/windows/win32/gdi/wm-displaychange
	// func: [](msg::wm_display_change p) {}
	WINLAMB_MSG_RET_ZERO(wm_display_change, WM_DISPLAYCHANGE)

	// https://docs.microsoft.com/en-us/windows/win32/dataxchg/wm-drawclipboard
	// func: [](msg::wm_draw_clipboard p) {}
	WINLAMB_MSG_RET_ZERO(wm_draw_clipboard, WM_DRAWCLIPBOARD)

	// https://docs.microsoft.com/en-us/windows/win32/controls/wm-drawitem
	// func: [](msg::wm_draw_item p) {}
	WINLAMB_MSG_RET_VAL(wm_draw_item, WM_DRAWITEM, TRUE)

	// https://docs.microsoft.com/en-us/windows/win32/shell/wm-dropfiles
	// func: [](msg::wm_drop_files p) {}
	WINLAMB_MSG_RET_ZERO(wm_drop_files, WM_DROPFILES)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-enable
	// func: [](msg::wm_enable p) {}
	WINLAMB_MSG_RET_ZERO(wm_enable, WM_ENABLE)

	// https://docs.microsoft.com/en-us/windows/win32/shutdown/wm-endsession
	// func: [](msg::wm_end_session p) {}
	WINLAMB_MSG_RET_ZERO(wm_end_session, WM_ENDSESSION)

	// https://docs.microsoft.com/en-us/windows/win32/dlgbox/wm-enteridle
	// func: [](msg::wm_enter_idle p) {}
	WINLAMB_MSG_RET_ZERO(wm_enter_idle, WM_ENTERIDLE)

	// https://docs.microsoft.com/en-us/windows/win32/menurc/wm-entermenuloop
	// func: [](msg::wm_enter_menu_loop p) {}
	WINLAMB_MSG_RET_ZERO(wm_enter_menu_loop, WM_ENTERMENULOOP)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-entersizemove
	// func: [](msg::wm_enter_size_move p) {}
	WINLAMB_MSG_RET_ZERO(wm_enter_size_move, WM_ENTERSIZEMOVE)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-erasebkgnd
	// func: [](msg::wm_erase_bkgnd p) -> int {}
	WINLAMB_MSG_RET_TYPE(wm_erase_bkgnd, WM_ERASEBKGND, int)

	// https://docs.microsoft.com/en-us/windows/win32/menurc/wm-exitmenuloop
	// func: [](msg::wm_exit_menu_loop p) {}
	WINLAMB_MSG_RET_ZERO(wm_exit_menu_loop, WM_EXITMENULOOP)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-exitsizemove
	// func: [](msg::wm_exit_size_move p) {}
	WINLAMB_MSG_RET_ZERO(wm_exit_size_move, WM_EXITSIZEMOVE)

	// https://docs.microsoft.com/en-us/windows/win32/gdi/wm-fontchange
	// func: [](msg::wm_font_change p) {}
	WINLAMB_MSG_RET_ZERO(wm_font_change, WM_FONTCHANGE)

	// https://docs.microsoft.com/en-us/windows/win32/dlgbox/wm-getdlgcode
	// func: [](msg::wm_get_dlg_code p) -> WORD {}
	WINLAMB_MSG_RET_TYPE(wm_get_dlg_code, WM_GETDLGCODE, WORD)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-getfont
	// func: [](msg::wm_get_font p) -> HFONT {}
	WINLAMB_MSG_RET_TYPE(wm_get_font, WM_GETFONT, HFONT)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-getfont
	// func: [](msg::wm_get_hot_key p) -> WORD {}
	WINLAMB_MSG_RET_TYPE(wm_get_hot_key, WM_GETFONT, WORD)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-geticon
	// func: [](msg::wm_get_icon p) -> HICON {}
	WINLAMB_MSG_RET_TYPE(wm_get_icon, WM_GETICON, HICON)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-getminmaxinfo
	// func: [](msg::wm_get_min_max_info p) {}
	WINLAMB_MSG_RET_ZERO(wm_get_min_max_info, WM_GETMINMAXINFO)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-gettext
	// func: [](msg::wm_get_text p) -> UINT {}
	WINLAMB_MSG_RET_TYPE(wm_get_text, WM_GETTEXT, UINT)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-gettextlength
	// func: [](msg::wm_get_text_length p) -> UINT {}
	WINLAMB_MSG_RET_TYPE(wm_get_text_length, WM_GETTEXTLENGTH, UINT)

	// https://docs.microsoft.com/en-us/windows/win32/menurc/wm-gettitlebarinfoex
	// func: [](msg::wm_get_title_bar_info_ex p) {}
	WINLAMB_MSG_RET_ZERO(wm_get_title_bar_info_ex, WM_GETTITLEBARINFOEX)

	// https://docs.microsoft.com/en-us/windows/win32/shell/wm-help
	// func: [](msg::wm_help p) {}
	WINLAMB_MSG_RET_VAL(wm_help, WM_HELP, TRUE)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-hotkey
	// func: [](msg::wm_hot_key p) {}
	WINLAMB_MSG_RET_ZERO(wm_hot_key, WM_HOTKEY)

	// https://docs.microsoft.com/en-us/windows/win32/controls/wm-hscroll
	// func: [](msg::wm_h_scroll p) {}
	WINLAMB_MSG_RET_ZERO(wm_h_scroll, WM_HSCROLL)

	// https://docs.microsoft.com/en-us/windows/win32/dataxchg/wm-hscrollclipboard
	// func: [](msg::wm_h_scroll_clipboard p) {}
	WINLAMB_MSG_RET_ZERO(wm_h_scroll_clipboard, WM_HSCROLLCLIPBOARD)

	// https://docs.microsoft.com/en-us/windows/win32/dlgbox/wm-initdialog
	// func: [](msg::wm_init_dialog p) -> bool {}
	WINLAMB_MSG_RET_TYPE(wm_init_dialog, WM_INITDIALOG, bool)

	// https://docs.microsoft.com/en-us/windows/win32/menurc/wm-initmenu
	// func: [](msg::wm_init_menu p) {}
	WINLAMB_MSG_RET_ZERO(wm_init_menu, WM_INITMENU)

	// https://docs.microsoft.com/en-us/windows/win32/menurc/wm-initmenupopup
	// func: [](msg::wm_init_menu_popup p) {}
	WINLAMB_MSG_RET_ZERO(wm_init_menu_popup, WM_INITMENUPOPUP)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-inputlangchange
	// func: [](msg::wm_input_lang_change p) {}
	WINLAMB_MSG_RET_VAL(wm_input_lang_change, WM_HELP, 1)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-inputlangchangerequest
	// func: [](msg::wm_input_lang_change_request p) {}
	WINLAMB_MSG_RET_ZERO(wm_input_lang_change_request, WM_INPUTLANGCHANGEREQUEST)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keydown
	// func: [](msg::wm_key_down p) {}
	WINLAMB_MSG_RET_ZERO(wm_key_down, WM_KEYDOWN)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keyup
	// func: [](msg::wm_key_up p) {}
	WINLAMB_MSG_RET_ZERO(wm_key_up, WM_KEYUP)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-killfocus
	// func: [](msg::wm_kill_focus p) {}
	WINLAMB_MSG_RET_ZERO(wm_kill_focus, WM_KILLFOCUS)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-lbuttondblclk
	// func: [](msg::wm_l_button_dbl_clk p) {}
	WINLAMB_MSG_RET_ZERO(wm_l_button_dbl_clk, WM_LBUTTONDBLCLK)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-lbuttondown
	// func: [](msg::wm_l_button_down p) {}
	WINLAMB_MSG_RET_ZERO(wm_l_button_down, WM_LBUTTONDOWN)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-lbuttonup
	// func: [](msg::wm_l_button_up p) {}
	WINLAMB_MSG_RET_ZERO(wm_l_button_up, WM_LBUTTONUP)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mbuttondblclk
	// func: [](msg::wm_m_button_dbl_clk p) {}
	WINLAMB_MSG_RET_ZERO(wm_m_button_dbl_clk, WM_MBUTTONDBLCLK)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mbuttondown
	// func: [](msg::wm_m_button_down p) {}
	WINLAMB_MSG_RET_ZERO(wm_m_button_down, WM_MBUTTONDOWN)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mbuttonup
	// func: [](msg::wm_m_button_up p) {}
	WINLAMB_MSG_RET_ZERO(wm_m_button_up, WM_MBUTTONUP)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-mdiactivate
	// func: [](msg::wm_mdi_activate p) {}
	WINLAMB_MSG_RET_ZERO(wm_mdi_activate, WM_MDIACTIVATE)

	// https://docs.microsoft.com/en-us/windows/win32/controls/wm-measureitem
	// func: [](msg::wm_measure_item p) {}
	WINLAMB_MSG_RET_VAL(wm_measure_item, WM_MEASUREITEM, TRUE)

	// https://docs.microsoft.com/en-us/windows/win32/menurc/wm-menuchar
	// func: [](msg::wm_menu_char p) -> DWORD {}
	WINLAMB_MSG_RET_TYPE(wm_menu_char, WM_MENUCHAR, DWORD)

	// https://docs.microsoft.com/en-us/windows/win32/menurc/wm-menudrag
	// func: [](msg::wm_menu_drag p) -> BYTE {}
	WINLAMB_MSG_RET_TYPE(wm_menu_drag, WM_MENUDRAG, BYTE)

	// https://docs.microsoft.com/en-us/windows/win32/menurc/wm-menugetobject
	// func: [](msg::wm_menu_get_object p) -> DWORD {}
	WINLAMB_MSG_RET_TYPE(wm_menu_get_object, WM_MENUGETOBJECT, DWORD)

	// https://docs.microsoft.com/en-us/windows/win32/menurc/wm-menurbuttonup
	// func: [](msg::wm_menu_r_button_up p) {}
	WINLAMB_MSG_RET_ZERO(wm_menu_r_button_up, WM_MENURBUTTONUP)

	// https://docs.microsoft.com/en-us/windows/win32/menurc/wm-menuselect
	// func: [](msg::wm_menu_select p) {}
	WINLAMB_MSG_RET_ZERO(wm_menu_select, WM_MENUSELECT)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mouseactivate
	// func: [](msg::wm_mouse_activate p) -> BYTE {}
	WINLAMB_MSG_RET_TYPE(wm_mouse_activate, WM_MOUSEACTIVATE, BYTE)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mousehover
	// func: [](msg::wm_mouse_hover p) {}
	WINLAMB_MSG_RET_ZERO(wm_mouse_hover, WM_MOUSEHOVER)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mousehwheel
	// func: [](msg::wm_mouse_h_wheel p) {}
	WINLAMB_MSG_RET_ZERO(wm_mouse_h_wheel, WM_MOUSEHWHEEL)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mouseleave
	// func: [](msg::wm_mouse_leave p) {}
	WINLAMB_MSG_RET_ZERO(wm_mouse_leave, WM_MOUSELEAVE)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mousemove
	// func: [](msg::wm_mouse_move p) {}
	WINLAMB_MSG_RET_ZERO(wm_mouse_move, WM_MOUSEMOVE)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mousewheel
	// func: [](msg::wm_mouse_wheel p) {}
	WINLAMB_MSG_RET_ZERO(wm_mouse_wheel, WM_MOUSEWHEEL)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-move
	// func: [](msg::wm_move p) {}
	WINLAMB_MSG_RET_ZERO(wm_move, WM_MOVE)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-moving
	// func: [](msg::wm_moving p) {}
	WINLAMB_MSG_RET_VAL(wm_moving, WM_MOVING, TRUE)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-ncactivate
	// func: [](msg::wm_nc_activate p) -> bool {}
	WINLAMB_MSG_RET_TYPE(wm_nc_activate, WM_NCACTIVATE, bool)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-nccalcsize
	// func: [](msg::wm_nc_calc_size p) -> WORD {}
	WINLAMB_MSG_RET_TYPE(wm_nc_calc_size, WM_NCCALCSIZE, WORD)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-nccreate
	// func: [](msg::wm_nc_create p) -> bool {}
	WINLAMB_MSG_RET_TYPE(wm_nc_create, WM_NCCREATE, bool)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-ncdestroy
	// func: [](msg::wm_nc_destroy p) {}
	// Warning, default handled in:
	// - window_main
	// - window_modeless
	// - dialog_main
	// - dialog_modeless
	WINLAMB_MSG_RET_ZERO(wm_nc_destroy, WM_NCDESTROY)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-nchittest
	// func: [](msg::wm_nc_hit_test p) -> int {}
	WINLAMB_MSG_RET_TYPE(wm_nc_hit_test, WM_NCHITTEST, int)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-nclbuttondblclk
	// func: [](msg::wm_nc_l_button_dbl_clk p) {}
	WINLAMB_MSG_RET_ZERO(wm_nc_l_button_dbl_clk, WM_NCLBUTTONDBLCLK)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-nclbuttondown
	// func: [](msg::wm_nc_l_button_down p) {}
	WINLAMB_MSG_RET_ZERO(wm_nc_l_button_down, WM_NCLBUTTONDOWN)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-nclbuttonup
	// func: [](msg::wm_nc_l_button_up p) {}
	WINLAMB_MSG_RET_ZERO(wm_nc_l_button_up, WM_NCLBUTTONUP)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-ncmbuttondblclk
	// func: [](msg::wm_nc_m_button_dbl_clk p) {}
	WINLAMB_MSG_RET_ZERO(wm_nc_m_button_dbl_clk, WM_NCMBUTTONDBLCLK)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-ncmbuttondown
	// func: [](msg::wm_nc_m_button_down p) {}
	WINLAMB_MSG_RET_ZERO(wm_nc_m_button_down, WM_NCMBUTTONDOWN)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-ncmbuttonup
	// func: [](msg::wm_nc_m_button_up p) {}
	WINLAMB_MSG_RET_ZERO(wm_nc_m_button_up, WM_NCMBUTTONUP)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-ncmousehover
	// func: [](msg::wm_nc_mouse_hover p) {}
	WINLAMB_MSG_RET_ZERO(wm_nc_mouse_hover, WM_NCMOUSEHOVER)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-ncmouseleave
	// func: [](msg::wm_nc_mouse_leave p) {}
	WINLAMB_MSG_RET_ZERO(wm_nc_mouse_leave, WM_NCMOUSELEAVE)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-ncmousemove
	// func: [](msg::wm_nc_mouse_move p) {}
	WINLAMB_MSG_RET_ZERO(wm_nc_mouse_move, WM_NCMOUSEMOVE)

	// https://docs.microsoft.com/en-us/windows/win32/gdi/wm-ncpaint
	// func: [](msg::wm_nc_paint p) {}
	// Warning, default handled in:
	// - window_control
	// - dialog_control
	WINLAMB_MSG_RET_ZERO(wm_nc_paint, WM_NCPAINT)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-ncrbuttondblclk
	// func: [](msg::wm_nc_r_button_dbl_clk p) {}
	WINLAMB_MSG_RET_ZERO(wm_nc_r_button_dbl_clk, WM_NCRBUTTONDBLCLK)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-ncrbuttondown
	// func: [](msg::wm_nc_r_button_down p) {}
	WINLAMB_MSG_RET_ZERO(wm_nc_r_button_down, WM_NCRBUTTONDOWN)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-ncrbuttonup
	// func: [](msg::wm_nc_r_button_up p) {}
	WINLAMB_MSG_RET_ZERO(wm_nc_r_button_up, WM_NCRBUTTONUP)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-ncxbuttondblclk
	// func: [](msg::wm_nc_x_button_dbl_clk p) {}
	WINLAMB_MSG_RET_VAL(wm_nc_x_button_dbl_clk, WM_NCXBUTTONDBLCLK, TRUE)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-ncxbuttondown
	// func: [](msg::wm_nc_x_button_down p) {}
	WINLAMB_MSG_RET_VAL(wm_nc_x_button_down, WM_NCXBUTTONDOWN, TRUE)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-ncxbuttonup
	// func: [](msg::wm_nc_x_button_up p) {}
	WINLAMB_MSG_RET_VAL(wm_nc_x_button_up, WM_NCXBUTTONUP, TRUE)

	// https://docs.microsoft.com/en-us/windows/win32/dlgbox/wm-nextdlgctl
	// func: [](msg::wm_next_dlg_ctl p) {}
	WINLAMB_MSG_RET_ZERO(wm_next_dlg_ctl, WM_NEXTDLGCTL)

	// https://docs.microsoft.com/en-us/windows/win32/menurc/wm-nextmenu
	// func: [](msg::wm_next_menu p) {}
	WINLAMB_MSG_RET_ZERO(wm_next_menu, WM_NEXTMENU)

	// https://docs.microsoft.com/en-us/windows/win32/controls/wm-notifyformat
	// func: [](msg::wm_notify_format p) -> BYTE {}
	WINLAMB_MSG_RET_TYPE(wm_notify_format, WM_NOTIFYFORMAT, BYTE)

	// https://docs.microsoft.com/en-us/windows/win32/gdi/wm-paint
	// func: [](msg::wm_paint p) {}
	WINLAMB_MSG_RET_ZERO(wm_paint, WM_PAINT)

	// https://docs.microsoft.com/en-us/windows/win32/dataxchg/wm-paintclipboard
	// func: [](msg::wm_paint_clipboard p) {}
	WINLAMB_MSG_RET_ZERO(wm_paint_clipboard, WM_PAINTCLIPBOARD)

	// https://docs.microsoft.com/en-us/windows/win32/gdi/wm-palettechanged
	// func: [](msg::wm_palette_changed p) {}
	WINLAMB_MSG_RET_ZERO(wm_palette_changed, WM_PALETTECHANGED)

	// https://docs.microsoft.com/en-us/windows/win32/gdi/wm-paletteischanging
	// func: [](msg::wm_palette_is_changing p) {}
	WINLAMB_MSG_RET_ZERO(wm_palette_is_changing, WM_PALETTEISCHANGING)

	// https://docs.microsoft.com/en-us/windows/win32/inputmsg/wm-parentnotify
	// func: [](msg::wm_parent_notify p) {}
	WINLAMB_MSG_RET_ZERO(wm_parent_notify, WM_PARENTNOTIFY)

	// https://docs.microsoft.com/en-us/windows/win32/power/wm-powerbroadcast
	// func: [](msg::wm_power_broadcast p) {}
	WINLAMB_MSG_RET_VAL(wm_power_broadcast, WM_POWERBROADCAST, TRUE)

	// https://docs.microsoft.com/en-us/windows/win32/gdi/wm-print
	// func: [](msg::wm_print p) {}
	WINLAMB_MSG_RET_ZERO(wm_print, WM_PRINT)

	// https://docs.microsoft.com/en-us/windows/win32/gdi/wm-printclient
	// func: [](msg::wm_print_client p) {}
	WINLAMB_MSG_RET_ZERO(wm_print_client, WM_PRINTCLIENT)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-querydragicon
	// func: [](msg::wm_query_drag_icon p) -> HICON {}
	WINLAMB_MSG_RET_TYPE(wm_query_drag_icon, WM_QUERYNEWPALETTE, HICON)

	// https://docs.microsoft.com/en-us/windows/win32/shutdown/wm-queryendsession
	// func: [](msg::wm_query_end_session p) -> bool {}
	WINLAMB_MSG_RET_TYPE(wm_query_end_session, WM_QUERYENDSESSION, bool)

	// https://docs.microsoft.com/en-us/windows/win32/gdi/wm-querynewpalette
	// func: [](msg::wm_query_new_palette p) -> bool {}
	WINLAMB_MSG_RET_TYPE(wm_query_new_palette, WM_QUERYNEWPALETTE, bool)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-queryopen
	// func: [](msg::wm_query_open p) -> bool {}
	WINLAMB_MSG_RET_TYPE(wm_query_open, WM_QUERYOPEN, bool)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-rbuttondblclk
	// func: [](msg::wm_r_button_dbl_clk p) {}
	WINLAMB_MSG_RET_ZERO(wm_r_button_dbl_clk, WM_RBUTTONDBLCLK)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-rbuttondown
	// func: [](msg::wm_r_button_down p) {}
	WINLAMB_MSG_RET_ZERO(wm_r_button_down, WM_RBUTTONDOWN)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-rbuttonup
	// func: [](msg::wm_r_button_up p) {}
	WINLAMB_MSG_RET_ZERO(wm_r_button_up, WM_RBUTTONUP)

#ifdef _RAS_H_ // Ras.h
	// https://docs.microsoft.com/en-us/windows/win32/rras/wm-rasdialevent
	// func: [](msg::wm_ras_dial_event p) {}
	WINLAMB_MSG_RET_VAL(wm_ras_dial_event, WM_RASDIALEVENT, TRUE)
#endif

	// https://docs.microsoft.com/en-us/windows/win32/dataxchg/wm-renderallformats
	// func: [](msg::wm_render_all_formats p) {}
	WINLAMB_MSG_RET_ZERO(wm_render_all_formats, WM_RENDERALLFORMATS)

	// https://docs.microsoft.com/en-us/windows/win32/dataxchg/wm-renderformat
	// func: [](msg::wm_render_format p) {}
	WINLAMB_MSG_RET_ZERO(wm_render_format, WM_RENDERFORMAT)

	// https://docs.microsoft.com/en-us/windows/win32/menurc/wm-setcursor
	// func: [](msg::wm_set_cursor p) -> bool {}
	WINLAMB_MSG_RET_TYPE(wm_set_cursor, WM_SETCURSOR, bool)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-setfocus
	// func: [](msg::wm_set_focus p) {}
	// Warning, default handled in:
	// - window_main
	// - window_modal
	WINLAMB_MSG_RET_ZERO(wm_set_focus, WM_SETFOCUS)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-setfont
	// func: [](msg::wm_set_font p) {}
	WINLAMB_MSG_RET_ZERO(wm_set_font, WM_SETFONT)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-sethotkey
	// func: [](msg::wm_set_hot_key p) -> int {}
	WINLAMB_MSG_RET_TYPE(wm_set_hot_key, WM_SETHOTKEY, int)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-seticon
	// func: [](msg::wm_set_icon p) -> HICON {}
	WINLAMB_MSG_RET_TYPE(wm_set_icon, WM_SETICON, HICON)

	// https://docs.microsoft.com/en-us/windows/win32/gdi/wm-setredraw
	// func: [](msg::wm_set_redraw p) {}
	WINLAMB_MSG_RET_ZERO(wm_set_redraw, WM_SETREDRAW)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-settext
	// func: [](msg::wm_set_text p) -> int {}
	WINLAMB_MSG_RET_TYPE(wm_set_text, WM_SETTEXT, int)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-settingchange
	// func: [](msg::wm_setting_change p) {}
	WINLAMB_MSG_RET_ZERO(wm_setting_change, WM_SETTINGCHANGE)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-showwindow
	// func: [](msg::wm_show_window p) {}
	WINLAMB_MSG_RET_ZERO(wm_show_window, WM_SHOWWINDOW)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-size
	// func: [](msg::wm_size p) {}
	WINLAMB_MSG_RET_ZERO(wm_size, WM_SIZE)

	// https://docs.microsoft.com/en-us/windows/win32/dataxchg/wm-sizeclipboard
	// func: [](msg::wm_size_clipboard p) {}
	WINLAMB_MSG_RET_ZERO(wm_size_clipboard, WM_SIZECLIPBOARD)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-sizing
	// func: [](msg::wm_sizing p) {}
	WINLAMB_MSG_RET_VAL(wm_sizing, WM_SIZING, TRUE)

	// https://docs.microsoft.com/en-us/windows/win32/printdocs/wm-spoolerstatus
	// func: [](msg::wm_spooler_status p) {}
	WINLAMB_MSG_RET_ZERO(wm_spooler_status, WM_SPOOLERSTATUS)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-stylechanged
	// func: [](msg::wm_style_changed p) {}
	WINLAMB_MSG_RET_ZERO(wm_style_changed, WM_STYLECHANGED)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-stylechanging
	// func: [](msg::wm_style_changing p) {}
	WINLAMB_MSG_RET_ZERO(wm_style_changing, WM_STYLECHANGING)

	// https://docs.microsoft.com/en-us/windows/win32/gdi/wm-syncpaint
	// func: [](msg::wm_sync_paint p) {}
	WINLAMB_MSG_RET_ZERO(wm_sync_paint, WM_SYNCPAINT)

	// https://docs.microsoft.com/en-us/windows/win32/menurc/wm-syschar
	// func: [](msg::wm_sys_char p) {}
	WINLAMB_MSG_RET_ZERO(wm_sys_char, WM_SYSCHAR)

	// https://docs.microsoft.com/en-us/windows/win32/gdi/wm-syscolorchange
	// func: [](msg::wm_sys_color_change p) {}
	WINLAMB_MSG_RET_ZERO(wm_sys_color_change, WM_SYSCOLORCHANGE)

	// https://docs.microsoft.com/en-us/windows/win32/menurc/wm-syscommand
	// func: [](msg::wm_sys_command p) {}
	WINLAMB_MSG_RET_ZERO(wm_sys_command, WM_SYSCOMMAND)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-sysdeadchar
	// func: [](msg::wm_sys_dead_char p) {}
	WINLAMB_MSG_RET_ZERO(wm_sys_dead_char, WM_SYSDEADCHAR)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-syskeydown
	// func: [](msg::wm_sys_key_down p) {}
	WINLAMB_MSG_RET_ZERO(wm_sys_key_down, WM_SYSKEYDOWN)

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-syskeyup
	// func: [](msg::wm_sys_key_up p) {}
	WINLAMB_MSG_RET_ZERO(wm_sys_key_up, WM_SYSKEYUP)

	// https://docs.microsoft.com/en-us/windows/win32/shell/wm-tcard
	// func: [](msg::wm_t_card p) {}
	WINLAMB_MSG_RET_ZERO(wm_t_card, WM_TCARD)

	// https://docs.microsoft.com/en-us/windows/win32/sysinfo/wm-timechange
	// func: [](msg::wm_time_change p) {}
	WINLAMB_MSG_RET_ZERO(wm_time_change, WM_TIMECHANGE)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-timer
	// func: [](msg::wm_timer p) {}
	WINLAMB_MSG_RET_ZERO(wm_timer, WM_TIMER)

	// https://docs.microsoft.com/en-us/windows/win32/menurc/wm-uninitmenupopup
	// func: [](msg::wm_un_init_menu_popup p) {}
	WINLAMB_MSG_RET_ZERO(wm_un_init_menu_popup, WM_UNINITMENUPOPUP)

	// https://docs.microsoft.com/en-us/windows/win32/controls/wm-vkeytoitem
	// func: [](msg::wm_vkey_to_item p) -> int {}
	WINLAMB_MSG_RET_TYPE(wm_vkey_to_item, WM_VKEYTOITEM, int)

	// https://docs.microsoft.com/en-us/windows/win32/controls/wm-vscroll
	// func: [](msg::wm_v_scroll p) {}
	WINLAMB_MSG_RET_ZERO(wm_v_scroll, WM_VSCROLL)

	// https://docs.microsoft.com/en-us/windows/win32/dataxchg/wm-vscrollclipboard
	// func: [](msg::wm_v_scroll_clipboard p) {}
	WINLAMB_MSG_RET_ZERO(wm_v_scroll_clipboard, WM_VSCROLLCLIPBOARD)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-windowposchanged
	// func: [](msg::wm_window_pos_changed p) {}
	WINLAMB_MSG_RET_ZERO(wm_window_pos_changed, WM_WINDOWPOSCHANGED)

	// https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-windowposchanging
	// func: [](msg::wm_window_pos_changing p) {}
	WINLAMB_MSG_RET_ZERO(wm_window_pos_changing, WM_WINDOWPOSCHANGING)
};

}//namespace _wli