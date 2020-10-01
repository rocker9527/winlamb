/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <algorithm>
#include <optional>
#include <stdexcept>

#include <vector>
#include <Windows.h>
#include <CommCtrl.h>
#include "internals/base_native_control.h"
#include "internals/control_visuals.h"
#include "internals/interfaces.h"
#include "internals/list_view_column.h"
#include "internals/list_view_item.h"
#include "internals/param_notif.h"
#include "image_list.h"
#include "menu.h"
#include "str.h"

namespace wl {

/// Native list view control.
///
/// The list view is subclassed by default, default handling these messages:
/// - WM_GETDLGCODE
/// - WM_RBUTTONDOWN
///
/// @see https://docs.microsoft.com/en-us/windows/win32/controls/list-view-controls-overview
class list_view final : public i_control {
private:
	_wli::base_native_control _base;
	menu _contextMenu;

public:
	/// Default constructor.
	list_view()
	{
		this->_add_default_subclass_handlers();
	}

	/// Move constructor
	list_view(list_view&&) = default;

	/// Move assignment operator.
	list_view& operator=(list_view&&) = default;

	/// Calls CreateWindowEx().
	/// This method will always add LVS_SHAREIMAGELISTS style, for safety.
	/// Should be called during parent's WM_CREATE processing.
	/// Position and size will be adjusted to match current system DPI.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	/// @see https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-create
	list_view& create(const i_window* parent, int id, POINT pos, SIZE size,
		DWORD lvStyles = 0, DWORD lvExStyles = 0)
	{
		HWND h = this->_base.create_window(parent, id, WC_LISTVIEWW, {}, pos, size,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP | LVS_SHAREIMAGELISTS | lvStyles, // never own image lists
			WS_EX_CLIENTEDGE);

		pos = _wli::multiply_dpi(pos);
		size = _wli::multiply_dpi(size);

		if (lvExStyles) {
			this->set_extended_lv_style(true, lvExStyles);
		}
		return *this;
	}

	/// Assigns this object to an existing control in a dialog window.
	list_view& assign(const i_window* parent, int ctrlId) { this->_base.assign(parent, ctrlId); return *this; }

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
	const list_view& enable(bool isEnabled) const noexcept { EnableWindow(this->hwnd(), isEnabled); return *this; }

	/// Returns the column at the given index.
	/// Does not perform bound checking.
	[[nodiscard]] list_view_column column(size_t columnIndex) const noexcept { return list_view_column{this->hwnd(), columnIndex}; }

	/// Adds a new column.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_insertcolumn
	const list_view& add_column(std::wstring_view text, UINT width) const
	{
		LVCOLUMNW lvc{};
		lvc.mask = LVCF_TEXT | LVCF_WIDTH;
		lvc.pszText = const_cast<wchar_t*>(text.data());
		lvc.cx = width;

		if (ListView_InsertColumn(this->hwnd(), 0xffff, &lvc) == -1) {
			throw std::runtime_error(
				wl::str::unicode_to_ansi(
					wl::str::format(L"ListView_InsertColumn failed \"%s\".", text) ));
		}
		return *this;
	}

	/// Retrieves the number of columns.
	[[nodiscard]] size_t column_count() const { return _wli::list_view_column_count(this->hwnd()); }

	/// Returns the item at the given index.
	/// Does not perform bound checking.
	[[nodiscard]] list_view_item item(size_t itemIndex) const noexcept { return list_view_item{this->hwnd(), itemIndex}; }

	/// Adds a new item.
	/// @return The newly added item.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_insertitem
	list_view_item add_item(std::wstring_view text) const
	{
		LVITEMW lvi{};
		lvi.mask = LVIF_TEXT;
		lvi.pszText = const_cast<wchar_t*>(text.data());
		lvi.iItem = 0x0fff'ffff; // insert as the last one

		size_t newIdx = ListView_InsertItem(this->hwnd(), &lvi);
		if (newIdx == -1) {
			throw std::runtime_error(
				wl::str::unicode_to_ansi(
					wl::str::format(L"ListView_InsertItem failed \"%s\".", text) ));
		}
		return list_view_item{this->hwnd(), newIdx};
	}

	/// Adds a new item.
	/// You must attach an image list to see the icons.
	/// @return The newly added item.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_insertitem
	list_view_item add_item_with_icon(std::wstring_view text, int iIcon) const
	{
		LVITEMW lvi{};
		lvi.mask = LVIF_TEXT | LVIF_IMAGE;
		lvi.pszText = const_cast<wchar_t*>(text.data());
		lvi.iItem = 0x0fff'ffff; // insert as the last one
		lvi.iImage = iIcon;

		size_t newIdx = ListView_InsertItem(this->hwnd(), &lvi);
		if (newIdx == -1) {
			throw std::runtime_error(
				wl::str::unicode_to_ansi(
					wl::str::format(L"ListView_InsertItem failed \"%s\".", text) ));
		}
		return list_view_item{this->hwnd(), newIdx};
	}

	/// Retrieves all items in the list view at once.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_getnextitem
	std::vector<list_view_item> all_items() const
	{
		std::vector<list_view_item> items;
		items.reserve(this->item_count());

		int idx = -1;
		for (;;) {
			idx = ListView_GetNextItem(this->hwnd(), idx, LVNI_ALL);
			if (idx == -1) break;

			items.emplace_back(this->item(idx));
		}

		return items;
	}

	/// Retrieves the texts of all items, at the given column.
	std::vector<std::wstring> all_items_text(size_t columnIndex) const
	{
		std::vector<std::wstring> texts;
		texts.reserve(this->item_count());

		int idx = -1;
		for (;;) {
			idx = ListView_GetNextItem(this->hwnd(), idx, LVNI_ALL);
			if (idx == -1) break;

			list_view_item theItem = this->item(idx);
			texts.emplace_back(theItem.subitem_text(columnIndex));
		}

		return texts;
	}

	/// Retrieves the item with the given text, case-insensitive, if any.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_finditem
	[[nodiscard]] std::optional<list_view_item> find(std::wstring_view text) const noexcept
	{
		LVFINDINFOW lfi{};
		lfi.flags = LVFI_STRING;
		lfi.psz = text.data();

		int idx = ListView_FindItem(this->hwnd(), -1, &lfi);
		if (idx == -1) {
			return {};
		}
		return {this->item(idx)};
	}

	/// Retrieves the currently focused item, if any.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_getnextitem
	[[nodiscard]] std::optional<list_view_item> focused_item() const noexcept
	{
		int idx = ListView_GetNextItem(this->hwnd(), -1, LVNI_FOCUSED);
		if (idx == -1) {
			return {};
		}
		return {this->item(idx)};
	}

	/// Sends LVM_HITTEST to determine the item at specified position, if any.
	/// Position coordinates must be relative to list view.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_hittestex
	[[nodiscard]] LVHITTESTINFO hit_test(POINT pos) const noexcept
	{
		LVHITTESTINFO lvht{};
		lvht.pt = pos;

		ListView_HitTestEx(this->hwnd(), &lvht);
		return lvht;
	}

	/// Retrieves the number of items.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_getitemcount
	[[nodiscard]] size_t item_count() const noexcept { return ListView_GetItemCount(this->hwnd()); }

	/// Deletes all items at once.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_deleteallitems
	const list_view& remove_all_items() const
	{
		if (!ListView_DeleteAllItems(this->hwnd())) {
			throw std::runtime_error("ListView_DeleteAllItems failed.");
		}
		return *this;
	}

	/// Deletes the given items at once.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_deleteitem
	const list_view& remove_items(const std::vector<list_view_item>& items) const
	{
		std::vector<size_t> sortedIdxs;
		sortedIdxs.reserve(items.size());

		for (const list_view_item& it : items) {
			if (it.hlist() != this->hwnd()) {
				throw std::logic_error("Cannot delete item from another list view.");
			}
			sortedIdxs.emplace_back(it.index());
		}
		std::sort(sortedIdxs.begin(), sortedIdxs.end(), std::greater()); // sort in reverse order

		for (size_t idx : sortedIdxs) {
			if (!ListView_DeleteItem(this->hwnd(), idx)) {
				throw std::runtime_error("ListView_DeleteItem failed.");
			}
		}
		return *this;
	}

	/// Deletes the given items at once.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_deleteitem
	const list_view& remove_items(std::initializer_list<size_t> indexes) const
	{
		std::vector<size_t> sortedIdxs = indexes;
		std::sort(sortedIdxs.begin(), sortedIdxs.end(), std::greater()); // sort in reverse order
		
		for (size_t idx : sortedIdxs) {
			if (!ListView_DeleteItem(this->hwnd(), idx)) {
				throw std::runtime_error("ListView_DeleteItem failed.");
			}
		}
		return *this;
	}

	/// Deletes all items currently selected.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_getnextitem
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_deleteitem
	const list_view& remove_selected_items() const noexcept
	{
		int i = -1;
		while ((i = ListView_GetNextItem(this->hwnd(), -1, LVNI_SELECTED)) != -1) {
			ListView_DeleteItem(this->hwnd(), i);
		}
		return *this;
	}

	/// Selects or deselects all items.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_setitemstate
	const list_view& select_all_items(bool isSelected) const noexcept
	{
		ListView_SetItemState(this->hwnd(), -1,
			isSelected ? LVIS_SELECTED : 0, LVIS_SELECTED);
		return *this;
	}

	/// Selects or deselects the given items at once.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_setitemstate
	const list_view& select_items(
		const std::vector<list_view_item>& items, bool isSelected) const noexcept
	{
		for (const list_view_item& i : items) {
			i.select(isSelected);
		}
		return *this;
	}

	/// Selects or deselects the given items at once.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_setitemstate
	const list_view& select_items(
		std::initializer_list<size_t> indexes, bool isSelected) const noexcept
	{
		for (size_t idx : indexes) {
			ListView_SetItemState(this->hwnd(), idx,
				isSelected ? LVIS_SELECTED : 0, LVIS_SELECTED);
		}
		return *this;
	}

	/// Retrieves the number of selected items.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_getselectedcount
	[[nodiscard]] size_t selected_item_count() const noexcept { return ListView_GetSelectedCount(this->hwnd()); }

	/// Retrieves the selected items, if any.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_getnextitem
	[[nodiscard]] std::vector<list_view_item> selected_items() const
	{
		std::vector<list_view_item> items;
		items.reserve(this->selected_item_count());

		int idx = -1;
		for (;;) {
			idx = ListView_GetNextItem(this->hwnd(), idx, LVNI_SELECTED);
			if (idx == -1) break;
			items.emplace_back(this->hwnd(), idx);
		}
		return items;
	}

	/// Retrieves the texts of the selected items, at the given column.
	[[nodiscard]] std::vector<std::wstring> selected_items_text(size_t columnIndex) const
	{
		std::vector<std::wstring> texts;
		texts.reserve(this->selected_item_count());

		int idx = -1;
		for (;;) {
			idx = ListView_GetNextItem(this->hwnd(), idx, LVNI_SELECTED);
			if (idx == -1) break;

			list_view_item theItem = this->item(idx);
			texts.emplace_back(theItem.subitem_text(columnIndex));
		}

		return texts;
	}

	/// Sets a popup menu to be used as the context menu.
	/// The list view doesn't own the menu, and it must remain valid to be used.
	list_view& set_context_menu(const menu& popupMenu) noexcept { this->_contextMenu = popupMenu; return *this; }

	/// Returns the context menu attached to this list view.
	[[nodiscard]] const menu& context_menu() const noexcept { return this->_contextMenu; }

	/// Sets or unsets the given LVS_EX styles.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/lvm-setextendedlistviewstyle
	const list_view& set_extended_lv_style(bool isSet, DWORD lvExStyles) const noexcept
	{
		SendMessageW(this->hwnd(), LVM_SETEXTENDEDLISTVIEWSTYLE,
			lvExStyles, isSet ? lvExStyles : 0);
		return *this;
	}

	/// Retrieves the LVS_EX styles.
	/// https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_getextendedlistviewstyle
	[[nodiscard]] DWORD extended_lv_style() const noexcept { return ListView_GetExtendedListViewStyle(this->hwnd()); }

	/// Sets the associated image list.
	/// 
	/// When creating the list view, use the LVS_SHAREIMAGELISTS style,
	/// because the image list can be shared. Without this style, the
	/// list view will automatically destroy the image list.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-listview_setimagelist
	list_view& set_image_list(
		const image_list& imageList, DWORD lvsilType = LVSIL_SMALL) noexcept
	{
		// This method is non-const because it's usually called during object creation,
		// which chains many non-const methods.
		ListView_SetImageList(this->hwnd(), imageList.himagelist(), lvsilType);
		return *this;
	}

	/// Sends a WM_SETREDRAW.
	/// Value false prevents the control from being redrawn.
	/// @see https://docs.microsoft.com/en-us/windows/win32/gdi/wm-setredraw
	const list_view& set_redraw(bool doRedraw) const noexcept
	{
		SendMessageW(this->hwnd(), WM_SETREDRAW,
			static_cast<WPARAM>(static_cast<BOOL>(doRedraw)), 0);
		return *this;
	}

private:
	void _add_default_subclass_handlers()
	{
		this->on_subclass_msg().wm_get_dlg_code([this](msg::wm_get_dlg_code p) noexcept -> WORD
		{
			if (!p.is_query() && p.vkey_code() == 'A' && p.has_ctrl()) { // Ctrl+A to select all items
				this->select_all_items(true);
				return DLGC_WANTCHARS;
			} else if (!p.is_query() && p.vkey_code() == VK_RETURN) { // send Enter key to parent
#pragma warning (disable: 26454) // https://stackoverflow.com/a/51142504/6923555
				NMLVKEYDOWN nmlvkd = {
					{
						this->hwnd(),
						static_cast<WORD>(this->id()),
						LVN_KEYDOWN // this triggers warning 26454: arithmetic overflow
					},
					VK_RETURN, 0
				};
#pragma warning (default: 26454)
				SendMessageW(GetAncestor(this->hwnd(), GA_PARENT),
					WM_NOTIFY, reinterpret_cast<WPARAM>(this->hwnd()),
					reinterpret_cast<LPARAM>(&nmlvkd));
				return DLGC_WANTALLKEYS;
			} else if (!p.is_query() && p.vkey_code() == VK_APPS) { // context menu keyboard key
				this->_show_context_menu(false, p.has_ctrl(), p.has_shift());
			}
			return static_cast<WORD>(DefSubclassProc(this->hwnd(), WM_GETDLGCODE, p.wparam, p.lparam));
		});

		this->on_subclass_msg().wm_r_button_down([this](msg::wm_r_button_down p) noexcept
		{
			this->_show_context_menu(true, p.has_ctrl(), p.has_shift());
		});
	}

	int _show_context_menu(bool followCursor, bool hasCtrl, bool hasShift) noexcept
	{
		if (!this->_contextMenu.hmenu()) return -1; // no context menu assigned

		POINT coords{};
		int itemBelowCursor = -1;
		if (followCursor) { // usually fired with a right-click
			LVHITTESTINFO lvhti{};
			GetCursorPos(&lvhti.pt); // relative to screen
			ScreenToClient(this->hwnd(), &lvhti.pt); // now relative to list view
			ListView_HitTest(this->hwnd(), &lvhti); // item below cursor, if any
			coords = lvhti.pt;
			itemBelowCursor = lvhti.iItem; // -1 if none
			if (itemBelowCursor != -1) { // an item was right-clicked
				if (!hasCtrl && !hasShift) {
					if ((ListView_GetItemState(this->hwnd(), itemBelowCursor, LVIS_SELECTED) & LVIS_SELECTED) == 0) {
						// If right-clicked item isn't currently selected, unselect all and select just it.
						ListView_SetItemState(this->hwnd(), -1, 0, LVIS_SELECTED);
						ListView_SetItemState(this->hwnd(), itemBelowCursor, LVIS_SELECTED, LVIS_SELECTED);
					}
					ListView_SetItemState(this->hwnd(), itemBelowCursor, LVIS_FOCUSED, LVIS_FOCUSED); // focus clicked
				}
			} else if (!hasCtrl && !hasShift) {
				ListView_SetItemState(this->hwnd(), -1, 0, LVIS_SELECTED); // unselect all
			}
			SetFocus(this->hwnd()); // because a right-click won't set the focus by default
		} else { // usually fired with the context menu keyboard key
			int itemFocused = ListView_GetNextItem(this->hwnd(), -1, LVNI_FOCUSED);
			if (itemFocused != -1 && ListView_IsItemVisible(this->hwnd(), itemFocused)) { // item focused and visible
				RECT rcItem{};
				ListView_GetItemRect(this->hwnd(), itemFocused, &rcItem, LVIR_BOUNDS); // relative to list view
				coords = {rcItem.left + 16, rcItem.top + (rcItem.bottom - rcItem.top) / 2};
			} else { // no focused and visible item
				coords = {6, 10};
			}
		}

		// The popup menu is created with hDlg as parent, so the menu messages go to it.
		// The lvhti coordinates are relative to list view, and will be mapped into screen-relative.
		this->_contextMenu.show_at_point(GetParent(this->hwnd()), coords, this->hwnd());
		return itemBelowCursor; // -1 if none
	}
};

}//namespace wl