/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <string_view>
#include <Windows.h>
#include "internals/base_dialog.h"
#include "internals/base_main_loop.h"
#include "internals/interfaces.h"
#include "internals/str_aux.h"

namespace wl {

/// Modeless popup dialog.
/// Allows message and notification handling.
///
/// Default handled messages:
/// - WM_CLOSE
/// - WM_NCDESTROY
class dialog_modeless : public i_window {
public:
	/// Setup options for dialog_modeless.
	struct setup_opts final {
		/// Resource dialog ID, must be set.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dialogboxparamw
		int dialog_id = 0;
	};

private:
	setup_opts _setup;
	_wli::base_dialog _base;

public:
	/// Default constructor.
	dialog_modeless()
	{
		this->_default_msg_handlers();
	}

	/// Move constructor.
	dialog_modeless(dialog_modeless&&) = default;

	/// Move assignment operator.
	dialog_modeless& operator=(dialog_modeless&&) = default;

	/// Exposes variables that will be used during dialog creation.
	setup_opts& setup()
	{
		if (this->hwnd()) {
			throw std::logic_error("Cannot call setup() after dialog_modeless is created.");
		}
		return this->_setup;
	}

	/// Creates the modeless dialog and returns immediately.
	/// Should be called during parent's WM_CREATE processing (or if dialog, WM_INITDIALOG).
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createdialogparamw
	/// @see https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-create
	/// @see https://docs.microsoft.com/en-us/windows/win32/dlgbox/wm-initdialog
	void create(const i_window* parent)
	{
		if (!parent) {
			throw std::invalid_argument("No parent passed to dialog_modeless::create().");
		}

		HINSTANCE hInst = reinterpret_cast<HINSTANCE>(
			GetWindowLongPtrW(parent->hwnd(), GWLP_HINSTANCE));
		HWND h = this->_base.create_dialog_param(hInst, nullptr, this->_setup.dialog_id);

		SendMessageW(parent->hwnd(), _wli::WM_MODELESS_CREATED, // tell parent we're here
			0xc0def00d, reinterpret_cast<LPARAM>(h) );
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
	const dialog_modeless& set_title(std::wstring_view t) const noexcept { SetWindowTextW(this->hwnd(), t.data()); return *this; }

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
};

}//namespace wl