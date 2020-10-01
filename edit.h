/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <optional>
#include <string_view>
#include <Windows.h>
#include "internals/base_native_control.h"
#include "internals/control_visuals.h"
#include "internals/gdi_obj.h"
#include "internals/interfaces.h"

namespace wl {

/// Native edit (textbox) control.
/// @see https://docs.microsoft.com/en-us/windows/win32/controls/about-edit-controls
class edit : public i_control {
private:
	_wli::base_native_control _base;

public:
	/// Defines the type of edit to be created.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/edit-control-styles
	enum class type {
		/// Single line, ES_AUTOHSCROLL.
		NORMAL,
		/// Single line, ES_AUTOHSCROLL, ES_PASSWORD.
		PASSWORD,
		/// Multi line, ES_MULTILINE, ES_WANTRETURN.
		MULTILINE
	};

	/// Default constructor.
	edit() = default;

	/// Move constructor.
	edit(edit&&) = default;

	/// Move assignment operator.
	edit& operator=(edit&&) = default;

	/// Calls CreateWindowEx().
	/// Should be called during parent's WM_CREATE processing.
	/// Position and size will be adjusted to match current system DPI.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	/// @see https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-create
	edit& create(const i_window* parent, int id, type editType,
		std::optional<std::wstring_view> text, POINT pos, SIZE size = {100, 21})
	{
		DWORD styles = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP;
		switch (editType) {
		case type::NORMAL:    styles |= ES_AUTOHSCROLL; break;
		case type::PASSWORD:  styles |= ES_AUTOHSCROLL | ES_PASSWORD; break;
		case type::MULTILINE: styles |= ES_MULTILINE | ES_WANTRETURN;
		}

		pos = _wli::multiply_dpi(pos);
		size = _wli::multiply_dpi(size);

		HWND h = this->_base.create_window(parent, id, L"EDIT", text,
			pos, size, styles, WS_EX_CLIENTEDGE);

		_wli::globalUiFont.set_on_control(*this);
		return *this;
	}

	/// Assigns this object to an existing control in a dialog window.
	edit& assign(const i_window* parent, int ctrlId) { this->_base.assign(parent, ctrlId); return *this; }

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
	const edit& enable(bool isEnabled) const noexcept { EnableWindow(this->hwnd(), isEnabled); return *this; }

	/// Sets the text in this control.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowtextw
	const edit& set_text(std::wstring_view t) const noexcept { SetWindowTextW(this->hwnd(), t.data()); return *this; }

	/// Retrieves the text in this control.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowtextw
	[[nodiscard]] std::wstring text() const { return _wli::get_window_text(this->hwnd()); }

	/// Replaces the currently selected text with the given one, with EM_REPLACESEL.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/em-replacesel
	const edit& replace_selection(std::wstring_view replacement) const noexcept
	{
		SendMessageW(this->hwnd(), EM_REPLACESEL, TRUE,
			reinterpret_cast<LPARAM>(replacement.data()));
		return *this;
	}

	/// Selects all the text with EM_SETSEL.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/em-setsel
	const edit& select_all() const noexcept
	{
		SendMessageW(this->hwnd(), EM_SETSEL, 0, -1);
		return *this;
	}

	/// Returns the selected text with EM_GETSEL.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/em-getsel
	[[nodiscard]] std::wstring selection() const
	{
		DWORD start = 0, pastEnd = 0;
		SendMessageW(this->hwnd(), EM_GETSEL,
			reinterpret_cast<WPARAM>(&start), reinterpret_cast<LPARAM>(&pastEnd));
		std::wstring text = this->text();
		return text.substr(start, pastEnd - start);
	}

	/// Sets the selection range with EM_SETSEL.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/em-setsel
	const edit& set_selection(int start, int length) const noexcept
	{
		SendMessageW(this->hwnd(), EM_SETSEL, start, static_cast<size_t>(start) + length);
		return *this;
	}
};

}//namespace wl