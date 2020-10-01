/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <stdexcept>
#include <string_view>
#include <Windows.h>
#include "internals/base_main_loop.h"
#include "internals/base_window.h"
#include "internals/control_visuals.h"
#include "internals/interfaces.h"
#include "internals/str_aux.h"

namespace wl {

/// Modeless popup window.
/// Allows message and notification handling.
///
/// Default handled messages:
/// - WM_CLOSE
/// - WM_NCDESTROY
class window_modeless : public i_window {
public:
	/// Setup options for window_modeless.
	struct setup_opts final {
		/// Window class name to be registered. Defaults to an auto-generated string.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
		std::wstring class_name;
		/// Window class styles.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
		DWORD class_style = CS_DBLCLKS;
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
		SIZE size = {300, 200};
		/// Window extended styles.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
		DWORD ex_style = WS_EX_TOOLWINDOW;
		/// Window styles.
		/// Suggestion: WS_SYSMENU (X button).
		DWORD style = WS_CAPTION | WS_CLIPCHILDREN | WS_BORDER | WS_VISIBLE;
	};

private:
	setup_opts _setup;
	_wli::base_window _base;

public:
	/// Default constructor.
	window_modeless()
	{
		this->_default_msg_handlers();
	}

	/// Move constructor.
	window_modeless(window_modeless&&) = default;

	/// Move assignment operator.
	window_modeless& operator=(window_modeless&&) = default;

	/// Exposes variables that will be used in RegisterClassEx() and
	/// CreateWindowEx() calls, during window creation.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	[[nodiscard]] setup_opts& setup()
	{
		if (this->hwnd()) {
			throw std::logic_error("Cannot call setup() after window_modeless is created.");
		}
		return this->_setup;
	}

	/// Creates the window and returns immediately.
	/// Should be called during parent's WM_CREATE processing (or if dialog, WM_INITDIALOG).
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	/// @see https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-create
	/// @see https://docs.microsoft.com/en-us/windows/win32/dlgbox/wm-initdialog
	void create(const i_window* parent)
	{
		if (!parent) {
			throw std::invalid_argument("No parent passed to window_modeless::create().");
		}

		HINSTANCE hInst = reinterpret_cast<HINSTANCE>(
			GetWindowLongPtrW(parent->hwnd(), GWLP_HINSTANCE));
		WNDCLASSEXW wcx = this->_wcx_from_opts(hInst);
		this->_base.register_class(wcx);

		this->_setup.size = _wli::multiply_dpi(this->_setup.size);

		HWND h = this->_base.create_window(wcx.hInstance, parent,
			wcx.lpszClassName, this->_setup.title, nullptr, {0, 0}, this->_setup.size,
			this->_setup.ex_style, this->_setup.style);

		SendMessageW(parent->hwnd(), _wli::WM_MODELESS_CREATED, // tell parent we're here
			0xc0def00d, reinterpret_cast<LPARAM>(h) );

		RECT rc{}, rcParent{};
		GetWindowRect(h, &rc);
		GetWindowRect(parent->hwnd(), &rcParent); // both relative to screen

		SetWindowPos(h, nullptr, // place over parent (warning: happens after WM_CREATE processing)
			rcParent.right - (rc.right - rc.left),
			rcParent.top + 34,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);
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

	/// Sets the window title.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowtextw
	const window_modeless& set_title(std::wstring_view t) const noexcept { SetWindowTextW(this->hwnd(), t.data()); return *this; }

	/// Retrieves the window title.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowtextw
	[[nodiscard]] std::wstring title() const { return _wli::get_window_text(this->hwnd()); }

private:
	void _default_msg_handlers()
	{
		this->on_msg().wm_close([this]() noexcept
		{
			DestroyWindow(this->hwnd());
		});

		this->on_msg().wm_nc_destroy([this]() noexcept
		{
			SendMessageW(GetWindow(this->hwnd(), GW_OWNER), // tell parent we're gone
				_wli::WM_MODELESS_DESTROYED,
				0xc0def00d, reinterpret_cast<LPARAM>(this->hwnd()) );
		});
	}

	[[nodiscard]] WNDCLASSEXW _wcx_from_opts(HINSTANCE hInst)
	{
		WNDCLASSEXW wcx{};
		wcx.cbSize = sizeof(WNDCLASSEXW);
		wcx.hInstance = hInst;
		wcx.style = this->_setup.class_style;
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