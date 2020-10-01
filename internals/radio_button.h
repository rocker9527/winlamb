/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <string_view>
#include <Windows.h>
#include "base_native_control.h"
#include "control_visuals.h"
#include "gdi_obj.h"
#include "interfaces.h"

namespace wl {

/// A single radio button control.
/// @see https://docs.microsoft.com/en-us/windows/win32/controls/button-types-and-styles#radio-buttons
class radio_button final : public wl::i_control {
private:
	_wli::base_native_control _base;

public:
	/// Defines if the radion button is the first of the group.
	enum class type {
		/// The radio button will have WS_GROUP.
		FIRST,
		/// The radio button will not have WS_GROUP.
		NONFIRST
	};

	/// Default constructor.
	radio_button() = default;

	/// Move constructor.
	radio_button(radio_button&&) = default;

	/// Move assignment operator.
	radio_button& operator=(radio_button&&) = default;

	/// Calls CreateWindowEx().
	/// Should be called during parent's WM_CREATE processing.
	/// Position will be adjusted to match current system DPI.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	/// @see https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-create
	radio_button& create(const i_window* parent, int id, type t,
		std::wstring_view text, POINT pos)
	{
		DWORD styles = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON
			| (t == type::FIRST ? WS_GROUP : 0);

		pos = _wli::multiply_dpi(pos);
		SIZE size = _wli::calc_check_bound_box(parent->hwnd(), text);

		HWND h = this->_base.create_window(parent, id, L"BUTTON", text,
			pos, size, styles, 0);

		_wli::globalUiFont.set_on_control(*this);
		return *this;
	}

	/// Assigns this object to an existing control in a dialog window.
	radio_button& assign(const i_window* parent, int ctrlId) { this->_base.assign(parent, ctrlId); return *this; }

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
	const radio_button& enable(bool isEnabled) const noexcept { EnableWindow(this->hwnd(), isEnabled); return *this; }

	/// Sets the text in this control.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowtextw
	const radio_button& set_text(std::wstring_view text) const
	{
		SIZE newSize = _wli::calc_check_bound_box(GetParent(this->hwnd()), text);
		SetWindowPos(this->hwnd(), nullptr, 0, 0,
			newSize.cx, newSize.cy, SWP_NOZORDER | SWP_NOMOVE);

		SetWindowTextW(this->hwnd(), text.data());
		return *this;
	}

	/// Retrieves the text in this control.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowtextw
	[[nodiscard]] std::wstring text() const { return _wli::get_window_text(this->hwnd()); }

	/// Sets the state to BST_CHECKED or BST_UNCHECKED.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/bm-setcheck
	const radio_button& set_checked(bool isChecked) const noexcept
	{
		SendMessageW(this->hwnd(), BM_SETCHECK,
			isChecked ? BST_CHECKED : BST_UNCHECKED, 0);
		return *this;
	}

	/// Calls set_checked() and sends a WM_COMMAND message emulating the user click.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/bm-setcheck
	const radio_button& set_checked_and_trigger(bool isChecked) const noexcept
	{
		this->set_checked(isChecked);
		SendMessageW(GetParent(this->hwnd()), WM_COMMAND,
			MAKEWPARAM(this->id(), 0),
			reinterpret_cast<LPARAM>(this->hwnd()) );
		return *this;
	}

	/// Tells if current state is BST_CHECKED.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/bm-getcheck
	[[nodiscard]] bool checked() const noexcept
	{
		return SendMessageW(this->hwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED;
	}
};

}//namespace wl