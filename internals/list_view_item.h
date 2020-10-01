/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <Windows.h>

namespace wl {

// A single item of the list view.
class list_view_item final {
private:
	HWND _hList = nullptr;
	size_t _index = -1;

public:
	/// Constructor.
	explicit list_view_item(HWND owner, size_t index) noexcept :
		_hList{owner}, _index{index} { }

	/// Equality operator.
	[[nodiscard]] bool operator==(const list_view_item& other) const noexcept
	{
		return this->_hList == other._hList
			&& this->_index == other._index;
	}

	/// Inequality operator.
	[[nodiscard]] bool operator!=(const list_view_item& other) const noexcept { return !this->operator==(other); }

	/// Returns the HWND of the list view to which this item belongs.
	[[nodiscard]] HWND hlist() const noexcept { return this->_hList; }

	/// Returns the zero-based index of this item.
	[[nodiscard]] size_t index() const noexcept { return this->_index; }

	/// Sets the text of the item at the first column.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/lvm-setitemtext
	const list_view_item& set_text(std::wstring_view text) const { return this->set_subitem_text(0, text); }

	/// Retrieves the text at the first column.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/lvm-getitemtext
	[[nodiscard]] std::wstring text() const { return this->subitem_text(0); }

	/// Sets the text of the item at the given column.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/lvm-setitemtext
	const list_view_item& set_subitem_text(
		size_t columnIndex, std::wstring_view text) const
	{
		LVITEMW lvi{};
		lvi.iSubItem = static_cast<int>(columnIndex);
		lvi.pszText = const_cast<wchar_t*>(text.data());

		if (!SendMessageW(this->_hList, LVM_SETITEMTEXT,
			this->_index, reinterpret_cast<LPARAM>(&lvi)) )
		{
			throw std::runtime_error(
				wl::str::unicode_to_ansi(
					wl::str::format(L"LVM_SETITEMTEXT failed to set text \"%s\".", text) ));
		}
		return *this;
	}

	/// Retrieves the text of at the given column.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/lvm-getitemtext
	[[nodiscard]] std::wstring subitem_text(size_t columnIndex) const
	{
		// http://forums.codeguru.com/showthread.php?351972-Getting-listView-item-text-length
		LVITEMW lvi{};
		lvi.iSubItem = static_cast<int>(columnIndex);

		// Notice that, since strings' size always increase, if the buffer
		// was previously allocated with a value bigger than our 1st step,
		// this will speed up the size checks.

		std::wstring buf(64, L'\0'); // speed-up 1st allocation
		int baseBufLen = 0;
		int charsWrittenWithoutNull = 0;

		do {
			baseBufLen += 64; // buffer increasing step, arbitrary
			buf.resize(baseBufLen);
			lvi.cchTextMax = baseBufLen;
			lvi.pszText = &buf[0];
			charsWrittenWithoutNull = static_cast<int>(
				SendMessageW(this->_hList, LVM_GETITEMTEXT,
					this->_index, reinterpret_cast<LPARAM>(&lvi)) );
		} while (charsWrittenWithoutNull == baseBufLen - 1); // to break, must have at least 1 char gap

		buf.resize( lstrlenW(buf.c_str()) ); // trim nulls
		return buf;
	}

	/// Sets the index of the image list icon for an item.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_setitem
	const list_view_item& set_icon_index(int iconIndex) const
	{
		LVITEMW lvi{};
		lvi.iItem = static_cast<int>(this->_index);
		lvi.mask = LVIF_IMAGE;
		lvi.iImage = iconIndex;

		if (!ListView_SetItem(this->_hList, &lvi)) {
			throw std::runtime_error("ListView_SetItem failed for icon index.");
		}
		return *this;
	}

	/// Returns the index of the image list icon.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_getitem
	[[nodiscard]] int icon_index() const
	{
		LVITEMW lvi{};
		lvi.iItem = static_cast<int>(this->_index);
		lvi.mask = LVIF_IMAGE;

		if (!ListView_GetItem(this->_hList, &lvi)) {
			throw std::runtime_error("ListView_GetItem failed for icon index.");
		}
		return lvi.iImage;
	}

	/// Sets the LPARAM associated to the item.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_setitem
	const list_view_item& set_lparam(LPARAM lp) const
	{
		LVITEMW lvi{};
		lvi.iItem = static_cast<int>(this->_index);
		lvi.mask = LVIF_PARAM;
		lvi.lParam = lp;

		if (!ListView_SetItem(this->_hList, &lvi)) {
			throw std::runtime_error("ListView_SetItem failed for param.");
		}
		return *this;
	}

	/// Returns the LPARAM associated to the item.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_getitem
	[[nodiscard]] LPARAM lparam() const
	{
		LVITEMW lvi{};
		lvi.iItem = static_cast<int>(this->_index);
		lvi.mask = LVIF_PARAM;

		if (!ListView_GetItem(this->_hList, &lvi)) {
			throw std::runtime_error("ListView_GetItem failed for param.");
		}
		return lvi.lParam;
	}

	/// Scrolls the list view, if needed, so the item becomes visible.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_ensurevisible
	const list_view_item& ensure_visible() const
	{
		if (!ListView_EnsureVisible(this->_hList, this->_index, 1)) {
			throw std::runtime_error("LVM_ENSUREVISIBLE failed.");
		}
		return *this;
	}

	/// Tells if the item is currently visible.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_isitemvisible
	[[nodiscard]] bool visible() const noexcept { return ListView_IsItemVisible(this->_hList, this->_index) != 0; }

	/// Focus the item.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_setitemstate
	const list_view_item& focus() const noexcept
	{
		ListView_SetItemState(this->_hList, this->_index, LVIS_FOCUSED, LVIS_FOCUSED);
		return *this;
	}

	/// Gets the bounding rectangle for the item.
	/// @return Coordinates relative to list view.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_getitemrect
	[[nodiscard]] RECT rect(int lvirPortion = LVIR_BOUNDS) const
	{
		RECT rcItem{};
		rcItem.left = lvirPortion;

		if (!ListView_GetItemRect(this->_hList, this->_index, &rcItem, lvirPortion)) {
			throw std::runtime_error("ListView_GetItemRect failed.");
		}
		return rcItem;
	}

	/// Deletes the item.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_deleteitem
	const list_view_item& remove() const
	{
		if (!ListView_DeleteItem(this->_hList, this->_index)) {
			throw std::runtime_error("ListView_DeleteItem failed.");
		}
		return *this;
	}

	/// Selects or deselects the item.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_setitemstate
	const list_view_item& select(bool isSelected) const noexcept
	{
		ListView_SetItemState(this->_hList, this->_index,
			isSelected ? LVIS_SELECTED : 0, LVIS_SELECTED);
		return *this;
	}

	/// Updates the item, rearranging the list view.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_update
	const list_view_item& update() const
	{
		if (!ListView_Update(this->_hList, this->_index)) {
			throw std::runtime_error("ListView_Update failed.");
		}
		return *this;
	}

	
};

}//namespace wl