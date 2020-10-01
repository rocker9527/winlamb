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

/// Native static (label) control.
/// @see https://docs.microsoft.com/en-us/windows/win32/controls/about-static-controls
class label final : public i_control {
private:
	_wli::base_native_control _base;

public:
	/// Default constructor.
	label() = default;

	/// Move constructor.
	label(label&&) = default;

	/// Move assignment operator.
	label& operator=(label&&) = default;

	/// Calls CreateWindowEx().
	/// Should be called during parent's WM_CREATE processing.
	/// Position will be adjusted to match current system DPI.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	/// @see https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-create
	label& create(const i_window* parent, int id,
		std::wstring_view text, POINT pos)
	{
		pos = _wli::multiply_dpi(pos);
		SIZE size = _wli::calc_text_bound_box(parent->hwnd(), text, true);

		HWND h = this->_base.create_window(parent, id, L"STATIC", text, pos, size,
			WS_CHILD | WS_VISIBLE | SS_LEFT, 0);

		_wli::globalUiFont.set_on_control(*this);
		return *this;
	}

	/// Assigns this object to an existing control in a dialog window.
	label& assign(const i_window* parent, int ctrlId) { this->_base.assign(parent, ctrlId); return *this; }

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
	const label& enable(bool isEnabled) const noexcept { EnableWindow(this->hwnd(), isEnabled); return *this; }

	/// Sets the text in the control, and resizes it to fit.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowtextw
	const label& set_text(std::wstring_view text) const
	{
		SIZE newSize = _wli::calc_text_bound_box(GetParent(this->hwnd()), text, true);
		SetWindowPos(this->hwnd(), nullptr, 0, 0,
			newSize.cx, newSize.cy, SWP_NOZORDER | SWP_NOMOVE);

		SetWindowTextW(this->hwnd(), text.data());
		return *this;
	}

	/// Retrieves the text in this control.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowtextw
	[[nodiscard]] std::wstring text() const { return _wli::get_window_text(this->hwnd()); }
};

}//namespace wl