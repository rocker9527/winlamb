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
#include <CommCtrl.h>
#include <VersionHelpers.h>
#include "internals/base_dialog.h"
#include "internals/base_main_loop.h"
#include "internals/gdi_obj.h"
#include "internals/interfaces.h"
#include "internals/str_aux.h"

namespace wl {

/// Dialog to be used as the application main window.
/// Allows message and notification handling.
///
/// Default handled messages:
/// - WM_CLOSE
/// - WM_NCDESTROY
class dialog_main : public i_window {
public:
	/// Setup options for dialog_main.
	struct setup_opts final {
		/// Resource dialog ID, must be set.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createdialogparamw
		int dialog_id = 0;
		/// Resource icon ID, optional.
		/// @see https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-seticon
		int icon_id = 0;
		/// Resource accelerator table ID, optional.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadacceleratorsw
		int accel_tbl_id = 0;
	};

private:
	setup_opts _setup;
	_wli::base_dialog _base;
	_wli::base_main_loop _mainLoop;

public:
	/// Default constructor.
	dialog_main()
	{
		this->_default_msg_handlers();
	}

	/// Move constructor.
	dialog_main(dialog_main&&) = default;

	/// Move assignment operator.
	dialog_main& operator=(dialog_main&&) = default;

	/// Exposes variables that will be used during dialog creation.
	[[nodiscard]] setup_opts& setup()
	{
		if (this->hwnd()) {
			throw std::logic_error("Cannot call setup() after dialog_main is created.");
		}
		return this->_setup;
	}

	/// Creates the dialog window and runs the main application loop.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createdialogparamw
	int run_as_main(HINSTANCE hInst, int cmdShow = SW_SHOW)
	{
		if (IsWindowsVistaOrGreater()) {
			SetProcessDPIAware();
		}
		InitCommonControls();
		_wli::globalUiFont.create_ui();

		this->_base.create_dialog_param(hInst, nullptr, this->_setup.dialog_id);

		HACCEL hAccel = nullptr;
		if (this->_setup.accel_tbl_id) {
			// An accelerator table loaded from resource is automatically freed by the system.
			hAccel = LoadAcceleratorsW(hInst, MAKEINTRESOURCEW(this->_setup.accel_tbl_id));
			if (!hAccel) {
				throw std::system_error(GetLastError(), std::system_category(),
					"LoadAccelerators failed for main dialog.");
			}
		}

		this->_set_icon_if_any(hInst);
		ShowWindow(this->hwnd(), cmdShow);
		return this->_mainLoop.run_loop(this->hwnd(), hAccel);
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
	const dialog_main& set_title(std::wstring_view t) const noexcept { SetWindowTextW(this->hwnd(), t.data()); return *this; }

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

		this->on_msg().wm_nc_destroy([]() noexcept
		{
			PostQuitMessage(0);
		});
	}

	void _set_icon_if_any(HINSTANCE hInst) const noexcept
	{
		// If an icon ID was specified, load it from the resources.
		// Resource icons are automatically released by the system.
		if (this->_setup.icon_id) {
			SendMessageW(this->hwnd(), WM_SETICON, ICON_SMALL,
				reinterpret_cast<LPARAM>(reinterpret_cast<HICON>(LoadImageW(hInst,
					MAKEINTRESOURCEW(this->_setup.icon_id),
					IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR))));

			SendMessageW(this->hwnd(), WM_SETICON, ICON_BIG,
				reinterpret_cast<LPARAM>(reinterpret_cast<HICON>(LoadImageW(hInst,
					MAKEINTRESOURCEW(this->_setup.icon_id),
					IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR))));
		}
	}
};

}//namespace wl