/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <Windows.h>
#include "internals/base_window.h"
#include "internals/control_visuals.h"
#include "internals/interfaces.h"
#include "internals/str_aux.h"

namespace wl {

/// Modal popup window.
/// Allows message and notification handling.
///
/// Default handled messages:
/// - WM_SETFOCUS
/// - WM_CLOSE
class window_modal : public i_window {
public:
	// Setup options for window_modal.
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
		SIZE size = {500, 400};
		/// Window extended styles.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
		DWORD ex_style = WS_EX_DLGMODALFRAME;
		/// Window styles.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
		DWORD style = WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_BORDER | WS_VISIBLE;
	};

private:
	setup_opts _setup;
	_wli::base_window _base;
	HWND _hPrevFocusParent = nullptr;

public:
	/// Default constructor.
	window_modal()
	{
		this->_default_msg_handlers();
	}

	/// Move constructor.
	window_modal(window_modal&&) = default;

	/// Move assignment operator.
	window_modal& operator=(window_modal&&) = default;

	/// Exposes variables that will be used in RegisterClassEx() and
	/// CreateWindowEx() calls, during window creation.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	[[nodiscard]] setup_opts& setup()
	{
		if (this->hwnd()) {
			throw std::logic_error("Cannot call setup() after window_modal is created.");
		}
		return this->_setup;
	}

	/// Creates the window and disables the parent.
	/// This method will block until the modal is closed.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	void show(const i_window* parent)
	{
		if (!parent) {
			throw std::invalid_argument("No parent passed to window_modal::show().");
		}

		HINSTANCE hInst = reinterpret_cast<HINSTANCE>(
			GetWindowLongPtrW(parent->hwnd(), GWLP_HINSTANCE));
		WNDCLASSEXW wcx = this->_wcx_from_opts(hInst);
		this->_base.register_class(wcx);

		this->_hPrevFocusParent = GetFocus();
		EnableWindow(parent->hwnd(), FALSE); // https://devblogs.microsoft.com/oldnewthing/20040227-00/?p=40463

		this->_setup.size = _wli::multiply_dpi(this->_setup.size);

		HWND h = this->_base.create_window(wcx.hInstance, parent,
			wcx.lpszClassName, this->_setup.title, nullptr, {0, 0}, this->_setup.size,
			this->_setup.ex_style, this->_setup.style);

		RECT rc{}, rcParent{};
		GetWindowRect(h, &rc);
		GetWindowRect(parent->hwnd(), &rcParent); // both relative to screen

		SetWindowPos(h, nullptr, // center modal over parent (warning: happens after WM_CREATE processing)
			rcParent.left + (rcParent.right - rcParent.left) / 2 - (rc.right - rc.left) / 2,
			rcParent.top + (rcParent.bottom - rcParent.top) / 2 - (rc.bottom - rc.top) / 2,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);

		this->_run_modal_loop();
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
	const window_modal& set_title(std::wstring_view t) const noexcept { SetWindowTextW(this->hwnd(), t.data()); return *this; }

	/// Retrieves the window title.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowtextw
	[[nodiscard]] std::wstring title() const { return _wli::get_window_text(this->hwnd()); }

private:
	void _default_msg_handlers()
	{
		this->on_msg().wm_set_focus([this]() noexcept
		{
			if (this->hwnd() == GetFocus()) {
				// If window receives focus, delegate to first child.
				SetFocus(GetNextDlgTabItem(this->hwnd(), nullptr, FALSE));
			}
		});

		this->on_msg().wm_close([this]() noexcept
		{
			EnableWindow(GetWindow(this->hwnd(), GW_OWNER), TRUE); // re-enable parent
			DestroyWindow(this->hwnd()); // then destroy modal
			SetFocus(this->_hPrevFocusParent); // this focus could be set on WM_DESTROY as well
		});
	}

	void _run_modal_loop()
	{
		for (;;) {
			MSG msg{};
			BOOL gmRet = GetMessageW(&msg, nullptr, 0, 0);
			if (gmRet == -1) {
				throw std::system_error(GetLastError(), std::system_category(),
					"GetMessage failed.");
			} else if (!gmRet) {
				// WM_QUIT was sent, exit modal loop now and signal parent.
				// wParam has the program exit code.
				// https://devblogs.microsoft.com/oldnewthing/20050222-00/?p=36393
				PostQuitMessage(static_cast<int>(msg.wParam));
				break;
			}

			// If a child window, will retrieve its top-level parent.
			// If a top-level, use itself.
			HWND hTopLevel = GetAncestor(msg.hwnd, GA_ROOT);

			if (IsDialogMessageW(hTopLevel, &msg)) {
				// Processed all keyboard actions for child controls.
				if (!this->hwnd()) {
					break; // our modal was destroyed, terminate loop
				} else {
					continue;
				}
			}

			TranslateMessage(&msg);
			DispatchMessageW(&msg);

			if (!this->hwnd()) {
				break; // our modal was destroyed, terminate loop
			}
		}
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