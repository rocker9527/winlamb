/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <stdexcept>
#include <string_view>
#include <Windows.h>
#include "internals/base_dialog.h"
#include "internals/interfaces.h"
#include "internals/str_aux.h"

namespace wl {

/// Modal popup dialog.
/// Allows message and notification handling.
///
/// Default handled messages:
/// - WM_CLOSE
class dialog_modal : public i_window {
public:
	/// Setup options for dialog_modal.
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
	dialog_modal()
	{
		this->_default_msg_handlers();
	}

	/// Move constructor.
	dialog_modal(dialog_modal&&) = default;

	/// Move assignment operator.
	dialog_modal& operator=(dialog_modal&&) = default;

	/// Exposes variables that will be used during dialog creation.
	setup_opts& setup()
	{
		if (this->hwnd()) {
			throw std::logic_error("Cannot call setup() after dialog_modal is created.");
		}
		return this->_setup;
	}

	/// Creates the modal dialog and disables the parent.
	/// This method will block until the modal is closed.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dialogboxparamw
	int show(const i_window* parent)
	{
		if (!parent) {
			throw std::invalid_argument("No parent passed to dialog_modal::show().");
		}

		HINSTANCE hInst = reinterpret_cast<HINSTANCE>(
			GetWindowLongPtrW(parent->hwnd(), GWLP_HINSTANCE));
		INT_PTR ret = this->_base.dialog_box_param(hInst, parent, this->_setup.dialog_id);
		return static_cast<int>(ret); // value passed to EndDialog()
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
	const dialog_modal& set_title(std::wstring_view t) const noexcept { SetWindowTextW(this->hwnd(), t.data()); return *this; }

	/// Retrieves the window title.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowtextw
	[[nodiscard]] std::wstring title() const { return _wli::get_window_text(this->hwnd()); }

private:
	void _default_msg_handlers()
	{
		this->on_msg().wm_close([this]() noexcept
		{
			EndDialog(this->hwnd(), IDCANCEL);
		});
	}
};

}//namespace wl