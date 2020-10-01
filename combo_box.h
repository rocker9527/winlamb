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

/// Native combo box (dropdown) control.
/// @see https://docs.microsoft.com/en-us/windows/win32/controls/about-combo-boxes
class combo_box final : i_control {
private:
	_wli::base_native_control _base;

public:
	/// Defines the sort order when creating a combo box.
	enum class sort { SORTED, UNSORTED };

	/// Default constructor.
	combo_box() = default;

	/// Move constructor.
	combo_box(combo_box&&) = default;

	/// Move assignment operator.
	combo_box& operator=(combo_box&&) = default;

	/// Calls CreateWindowEx().
	/// Should be called during parent's WM_CREATE processing.
	/// Position and width will be adjusted to match current system DPI.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	/// @see https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-create
	combo_box& create(const i_window* parent, int id, sort sortOrder, POINT pos, UINT width)
	{
		DWORD styles = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP | CBS_DROPDOWNLIST
			| (sortOrder == sort::SORTED ? CBS_SORT : 0);

		pos = _wli::multiply_dpi(pos);

		SIZE size = {static_cast<LONG>(width), 0};
		size = _wli::multiply_dpi(size);

		HWND h = this->_base.create_window(parent, id, L"COMBOBOX", {},
			pos, size, styles, 0);

		_wli::globalUiFont.set_on_control(*this);
		return *this;
	}

	/// Assigns this object to an existing control in a dialog window.
	combo_box& assign(const i_window* parent, int ctrlId) { this->_base.assign(parent, ctrlId); return *this; }

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
	const combo_box& enable(bool isEnabled) const noexcept { EnableWindow(this->hwnd(), isEnabled); return *this; }

	/// Adds a new item with CB_ADDSTRING.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/cb-addstring
	const combo_box& add_item(std::wstring_view text) const noexcept
	{
		SendMessageW(this->hwnd(), CB_ADDSTRING,
			0, reinterpret_cast<LPARAM>(text.data()) );
		return *this;
	}

	/// Adds many items at once with CB_ADDSTRING.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/cb-addstring
	const combo_box& add_item(std::initializer_list<std::wstring_view> texts) const noexcept
	{
		for (std::wstring_view text : texts) {
			this->add_item(text);
		}
		return *this;
	}

	/// Deletes all items with CB_RESETCONTENT.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/cb-resetcontent
	const combo_box& delete_all_items() const noexcept
	{
		SendMessageW(this->hwnd(), CB_RESETCONTENT, 0, 0);
		return *this;
	}

	/// Deletes the item with CB_DELETESTRING.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/cb-deletestring
	const combo_box& delete_item(size_t itemIndex) const
	{
		if (SendMessageW(this->hwnd(), CB_DELETESTRING, itemIndex, 0) == CB_ERR) {
			throw std::runtime_error(
				wl::str::unicode_to_ansi(
					wl::str::format(L"CB_DELETESTRING failed on \"%d\".", itemIndex) ));
		}
		return *this;
	}

	/// Retrieves the number if items with CB_GETCOUNT.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/cb-getcount
	[[nodiscard]] size_t item_count() const noexcept
	{
		return SendMessageW(this->hwnd(), CB_GETCOUNT, 0, 0);
	}

	/// Retrieves the text with CB_GETLBTEXT.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/cb-getlbtext
	[[nodiscard]] std::wstring item_text(size_t itemIndex) const
	{
		std::wstring buf;
		size_t len = SendMessageW(this->hwnd(), CB_GETLBTEXTLEN, itemIndex, 0);

		if (len) {
			buf.resize(len, L'\0');
			SendMessageW(this->hwnd(), CB_GETLBTEXT,
				itemIndex, reinterpret_cast<LPARAM>(&buf[0]));
			buf.resize(len);
		}
		return buf;
	}

	/// Selects the item with CB_SETCURSEL.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/cb-setcursel
	const combo_box& select_item(std::optional<size_t> itemIndex) const noexcept
	{
		SendMessageW(this->hwnd(), CB_SETCURSEL, itemIndex.value_or(-1), 0);
		return *this;
	}

	/// Retrieves the selected item index with CB_GETCURSEL, if any.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/cb-getcursel
	[[nodiscard]] std::optional<size_t> selected_item_index() const noexcept
	{
		size_t idx = SendMessageW(this->hwnd(), CB_GETCURSEL, 0, 0);
		if (idx == CB_ERR) {
			return {};
		}
		return idx;
	}

	/// Retrieves the selected item text, if any.
	[[nodiscard]] std::optional<std::wstring> selected_item_text() const
	{
		std::optional<size_t> selIdx = this->selected_item_index();
		if (selIdx.has_value()) {
			return this->item_text(selIdx.value());
		}
		return {};
	}

	/// Shows or hide the list box with CB_SHOWDROPDOWN.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/cb-showdropdown
	const combo_box& show_dropdown(bool isVisible) const noexcept
	{
		SendMessageW(this->hwnd(), CB_SHOWDROPDOWN, isVisible, 0);
		return *this;
	}
};

}//namespace wl