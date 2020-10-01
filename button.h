/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <string_view>
#include <Windows.h>
#include "internals/base_native_control.h"
#include "internals/control_visuals.h"
#include "internals/gdi_obj.h"
#include "internals/interfaces.h"

namespace wl {

/// Native button control.
/// @see https://docs.microsoft.com/en-us/windows/win32/controls/button-types-and-styles#push-buttons
class button final : public i_control {
private:
	_wli::base_native_control _base;

public:
	/// Defines the type of button to be created.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/button-styles
	enum class type {
		/// Button will not have BS_DEFPUSHBUTTON style.
		NORMAL,
		/// Button will have BS_DEFPUSHBUTTON style.
		DEFPUSH
	};

	/// Default constructor.
	button() = default;

	/// Move constructor.
	button(button&&) = default;

	/// Move assignment operator.
	button& operator=(button&&) = default;

	/// Calls CreateWindowEx().
	/// Should be called during parent's WM_CREATE processing.
	/// Position and size will be adjusted to match current system DPI.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	/// @see https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-create
	button& create(const i_window* parent, int id, type btnType,
		std::wstring_view text, POINT pos, SIZE size = {80, 23})
	{
		DWORD styles = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP
			| (btnType == type::DEFPUSH ? BS_DEFPUSHBUTTON : 0);

		pos = _wli::multiply_dpi(pos);
		size = _wli::multiply_dpi(size);

		HWND h = this->_base.create_window(parent, id, L"BUTTON", text,
			pos, size, styles, 0);

		_wli::globalUiFont.set_on_control(*this);
		return *this;
	}

	/// Assigns this object to an existing control in a dialog window.
	button& assign(const i_window* parent, int ctrlId) { this->_base.assign(parent, ctrlId); return *this; }

	/// Installs a window subclass and exposes methods to add message handlers.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/subclassing-overview
	[[nodiscard]] _wli::msg_proxy& on_subclass_msg() { return this->_base.on_subclass_msg(); }

	/// Returns the underlying HWND handle.
	[[nodiscard]] HWND hwnd() const noexcept override { return this->_base.hwnd(); }

	/// Retrieves the control ID.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdlgctrlid
	[[nodiscard]] int id() const noexcept override { return this->_base.id(); }

	/// Calls EnableWindow().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enablewindow
	const button& enable(bool isEnabled) const noexcept { EnableWindow(this->hwnd(), isEnabled); return *this; }

	/// Sets the text in this control.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowtextw
	const button& set_text(std::wstring_view t) const noexcept { SetWindowTextW(this->hwnd(), t.data()); return *this; }

	/// Retrieves the text in this control.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowtextw
	[[nodiscard]] std::wstring text() const { return _wli::get_window_text(this->hwnd()); }
};

}//namespace wl