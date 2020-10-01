/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <stdexcept>
#include <string_view>
#include <vector>
#include <Windows.h>
#include <CommCtrl.h>
#include <VersionHelpers.h>
#include "internals/base_main_loop.h"
#include "internals/base_window.h"
#include "internals/control_visuals.h"
#include "internals/gdi_obj.h"
#include "internals/interfaces.h"
#include "internals/str_aux.h"
#include "accel_table.h"
#include "menu.h"

namespace wl {

/// Main application window.
/// Allows message and notification handling.
///
/// Default handled messages:
/// - WM_ACTIVATE
/// - WM_SETFOCUS
/// - WM_NCDESTROY
class window_main : public i_window {
public:
	/// Setup options for window_main.
	struct setup_opts final {
		/// Window class name to be registered. Defaults to an auto-generated string.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
		std::wstring class_name;
		/// Window class styles.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
		DWORD class_style = CS_DBLCLKS;
		/// Window main HICON.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
		HICON class_icon = nullptr;
		/// Window small HICON.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
		HICON class_icon_sm = nullptr;
		/// Window HCURSOR. If none is specified, will load IDC_ARROW.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
		HCURSOR class_cursor = nullptr;
		/// Window background HBRUSH. Defaults to brown/gray.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
		HBRUSH class_bg_brush = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);

		/// Window title. Defaults to an empty string.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
		std::wstring title;
		/// Window size, including title bar and borders.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
		SIZE size = {600, 500};
		/// Window extended styles.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
		DWORD ex_style = 0;
		/// Window styles. Suggestions: WS_SIZEBOX (resizable), WS_MINIMIZEBOX, WS_MAXIMIZEBOX.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
		DWORD style = WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_BORDER;
		/// Main window menu. This menu is not shared, window_main owns it.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
		menu_main main_menu;

		/// Accelerator table for keyboard shortcuts. Defaults to empty.
		accel_table accel_tbl;
	};

private:
	setup_opts _setup;
	_wli::base_window _base;
	_wli::base_main_loop _mainLoop;
	HWND _hChildPrevFocus = nullptr; // WM_ACTIVATE woes

public:
	/// Default constructor.
	window_main()
	{
		this->_default_msg_handlers();
	}

	/// Move constructor.
	window_main(window_main&&) = default;

	/// Move assignment operator.
	window_main& operator=(window_main&&) = default;

	/// Exposes variables that will be used in RegisterClassEx() and
	/// CreateWindowEx() calls, during window creation.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	[[nodiscard]] setup_opts& setup()
	{
		if (this->hwnd()) {
			throw std::logic_error("Cannot call setup() after window_main is created.");
		}
		return this->_setup;
	}

	/// Creates the window and runs the main application loop.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	int run_as_main(HINSTANCE hInst, int cmdShow = SW_SHOW)
	{
		if (IsWindowsVistaOrGreater()) {
			SetProcessDPIAware();
		}
		InitCommonControls();
		_wli::globalUiFont.create_ui();

		WNDCLASSEXW wcx = this->_wcx_from_opts(hInst);
		this->_base.register_class(wcx);

		this->_setup.size = _wli::multiply_dpi(this->_setup.size);

		SIZE szScreen = {GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
		POINT pos = {
			szScreen.cx / 2 - this->_setup.size.cx / 2, // center on screen
			szScreen.cy / 2 - this->_setup.size.cy / 2
		};

		HWND h = this->_base.create_window(wcx.hInstance, nullptr,
			wcx.lpszClassName, this->_setup.title, this->_setup.main_menu.hmenu(),
			pos, this->_setup.size, this->_setup.ex_style, this->_setup.style);
		ShowWindow(h, cmdShow);

		if (!UpdateWindow(h)) {
			throw std::runtime_error("UpdateWindow failed when creating window_main.");
		}
		return this->_mainLoop.run_loop(
			this->hwnd(), this->_setup.accel_tbl.haccel());
	}

	/// Exposes methods to add message handlers.
	[[nodiscard]] _wli::msg_proxy_all& on_msg() { return this->_base.on_msg(); }

	/// Executes a function asynchronously, in a new detached background thread.
	/// @tparam F std::function<void()>
	/// @param func [](ui_work ui) {}
	template<typename F>
	void background_work(F&& func) { this->_base.background_work(std::move(func)); }

	/// Returns the underlying HWND handle.
	[[nodiscard]] HWND hwnd() const noexcept override { return this->_base.hwnd(); }

	/// Returns the horizontal main window menu.
	[[nodiscard]] const menu& main_menu() const noexcept { return this->_setup.main_menu; }

	/// Sets the window title.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowtextw
	const window_main& set_title(std::wstring_view t) const noexcept { SetWindowTextW(this->hwnd(), t.data()); return *this; }

	/// Retrieves the window title.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowtextw
	[[nodiscard]] std::wstring title() const { return _wli::get_window_text(this->hwnd()); }

private:
	void _default_msg_handlers()
	{
		this->on_msg().wm_activate([this](msg::wm_activate p) noexcept
		{
			if (!p.is_minimized()) {
				if (!p.is_being_activated()) {
					HWND hCurFocus = GetFocus();
					if (hCurFocus && IsChild(this->hwnd(), hCurFocus)) {
						this->_hChildPrevFocus = hCurFocus; // save previously focused control
					}
				} else if (this->_hChildPrevFocus) {
					SetFocus(this->_hChildPrevFocus); // put focus back
				}
			}
		});

		this->on_msg().wm_set_focus([this]() noexcept
		{
			if (this->hwnd() == GetFocus()) {
				// If window receives focus, delegate to first child.
				SetFocus(GetNextDlgTabItem(this->hwnd(), nullptr, FALSE));
			}
		});

		this->on_msg().wm_nc_destroy([]() noexcept
		{
			PostQuitMessage(0);
		});

		this->on_msg().wm(_wli::WM_MODELESS_CREATED, [this](msg::wm p) -> LRESULT
		{
			if (p.wparam == 0xc0def00d) {
				this->_mainLoop.add_modeless_child(
					reinterpret_cast<HWND>(p.lparam));
			}
			return 0;
		});

		this->on_msg().wm(_wli::WM_MODELESS_DESTROYED, [this](msg::wm p) -> LRESULT
		{
			if (p.wparam == 0xc0def00d) {
				this->_mainLoop.delete_modeless_child(
					reinterpret_cast<HWND>(p.lparam));
			}
			return 0;
		});
	}

	[[nodiscard]] WNDCLASSEXW _wcx_from_opts(HINSTANCE hInst)
	{
		WNDCLASSEXW wcx{};
		wcx.cbSize = sizeof(WNDCLASSEXW);
		wcx.hInstance = hInst;
		wcx.style = this->_setup.class_style;
		wcx.hIcon = this->_setup.class_icon;
		wcx.hIconSm = this->_setup.class_icon_sm;
		wcx.hbrBackground = this->_setup.class_bg_brush;

		_wli::base_window::wcx_set_cursor(this->_setup.class_cursor, wcx);

		if (this->_setup.class_name.empty()) { // if user didn't choose a class name
			this->_setup.class_name = _wli::base_window::wcx_generate_hash(wcx); // generate hash after all fields are set
			wcx.lpszClassName = this->_setup.class_name.c_str();
		}
		return wcx;
	}
};

}//namespace wl