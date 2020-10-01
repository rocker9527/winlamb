/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <stdexcept>
#include <string_view>
#include <Windows.h>
#include "internals/base_window.h"
#include "internals/control_visuals.h"
#include "internals/interfaces.h"

namespace wl {

/// Custom user control.
/// Allows message and notification handling.
///
/// Default handled messages:
/// - WM_NCPAINT
class window_control : public i_control {
public:
	/// Setup options for window_control.
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
		/// Window background HBRUSH. Defaults to white color.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
		HBRUSH class_bg_brush = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

		/// Window extended styles. Use WS_EX_CLIENTEDGE for a control border.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
		DWORD ex_style = 0;
		/// Window styles.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
		DWORD style = WS_CHILD | WS_TABSTOP | WS_GROUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	};

private:
	setup_opts _setup;
	_wli::base_window _base;

public:
	/// Default constructor.
	window_control()
	{
		this->_default_msg_handlers();
	}

	/// Move constructor.
	window_control(window_control&&) = default;

	/// Move assignment operator.
	window_control& operator=(window_control&&) = default;

	/// Exposes variables that will be used in RegisterClassEx() and
	/// CreateWindowEx() calls, during window creation.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	[[nodiscard]] setup_opts& setup()
	{
		if (this->hwnd()) {
			throw std::logic_error("Cannot call setup() after window_control is created.");
		}
		return this->_setup;
	}

	/// Creates the control.
	/// Should be called during parent's WM_CREATE processing (or if dialog, WM_INITDIALOG).
	/// Position and size will be adjusted to match current system DPI.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	/// @see https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-create
	/// @see https://docs.microsoft.com/en-us/windows/win32/dlgbox/wm-initdialog
	void create(const i_window* parent, int id, POINT pos, SIZE size)
	{
		if (!parent) {
			throw std::invalid_argument("No parent passed to window_control::create().");
		}

		HINSTANCE hInst = reinterpret_cast<HINSTANCE>(
			GetWindowLongPtrW(parent->hwnd(), GWLP_HINSTANCE));
		WNDCLASSEXW wcx = this->_wcx_from_opts(hInst);
		this->_base.register_class(wcx);

		pos = _wli::multiply_dpi(pos);
		size = _wli::multiply_dpi(size);

		this->_base.create_window(wcx.hInstance, parent, wcx.lpszClassName, {},
			reinterpret_cast<HMENU>(static_cast<LONG_PTR>(id)),
			pos, size, this->_setup.ex_style, this->_setup.style);
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

	/// Retrieves the control ID.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdlgctrlid
	[[nodiscard]] int id() const noexcept override { return GetDlgCtrlID(this->hwnd()); }

private:
	void _default_msg_handlers()
	{
		this->on_msg().wm_nc_paint([this](msg::wm_nc_paint p) noexcept
		{
			_wli::paint_control_borders(this->hwnd(), p.wparam, p.lparam);
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