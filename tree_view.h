/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <optional>
#include <Windows.h>
#include <CommCtrl.h>
#include "internals/base_native_control.h"
#include "internals/control_visuals.h"
#include "internals/interfaces.h"
#include "internals/tree_view_item.h"
#include "image_list.h"

namespace wl {

/// Native tree view control.
/// @see https://docs.microsoft.com/en-us/windows/win32/controls/tree-view-controls
class tree_view final : i_control {
private:
	_wli::base_native_control _base;

public:
	/// Default constructor.
	tree_view() = default;

	/// Move constructor.
	tree_view(tree_view&&) = default;

	/// Move assignment operator.
	tree_view& operator=(tree_view&&) = default;

	/// Calls CreateWindowEx().
	/// Should be called during parent's WM_CREATE processing.
	/// Position and size will be adjusted to match current system DPI.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	/// @see https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-create
	tree_view& create(const i_window* parent, int id, POINT pos, SIZE size,
		DWORD tvStyles = TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_HASBUTTONS,
		DWORD tvExStyles = 0)
	{
		HWND h = this->_base.create_window(parent, id, WC_TREEVIEWW, {}, pos, size,
			WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP | tvStyles,
			WS_EX_CLIENTEDGE);

		pos = _wli::multiply_dpi(pos);
		size = _wli::multiply_dpi(size);

		if (tvExStyles) {
			this->set_extended_tv_style(true, tvExStyles);
		}
		return *this;
	}

	/// Assigns this object to an existing control in a dialog window.
	tree_view& assign(const i_window* parent, int ctrlId) { this->_base.assign(parent, ctrlId); return *this; }

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
	const tree_view& enable(bool isEnabled) const noexcept { EnableWindow(this->hwnd(), isEnabled); return *this; }

	/// Adds a new root node, returning it.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_insertitem
	tree_view_item add_root_item(std::wstring_view text) const
	{
		TVINSERTSTRUCTW tvi{};
		tvi.hParent = TVI_ROOT;
		tvi.hInsertAfter = TVI_LAST;
		tvi.itemex.mask = TVIF_TEXT;
		tvi.itemex.pszText = const_cast<wchar_t*>(text.data());

		HTREEITEM newItem = TreeView_InsertItem(this->hwnd(), &tvi);
		if (!newItem) {
			throw std::runtime_error(
				wl::str::unicode_to_ansi(
					wl::str::format(L"TreeView_InsertItem failed \"%s\".", text) ));
		}
		return tree_view_item{this->hwnd(), newItem};
	}

	/// Adds a new root node, returning it.
	/// You must attach an image list to see the icon.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_insertitem
	tree_view_item add_root_item_with_icon(std::wstring_view text, int iIcon) const
	{
		TVINSERTSTRUCTW tvi{};
		tvi.hParent = TVI_ROOT;
		tvi.hInsertAfter = TVI_LAST;
		tvi.itemex.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.itemex.pszText = const_cast<wchar_t*>(text.data());
		tvi.itemex.iImage = iIcon;
		tvi.itemex.iSelectedImage = iIcon;

		HTREEITEM newItem = TreeView_InsertItem(this->hwnd(), &tvi);
		if (!newItem) {
			throw std::runtime_error(
				wl::str::unicode_to_ansi(
					wl::str::format(L"TreeView_InsertItem failed \"%s\".", text) ));
		}
		return tree_view_item{this->hwnd(), newItem};
	}

	/// Returns the total number of nodes.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_getcount
	[[nodiscard]] size_t item_count() const noexcept { return TreeView_GetCount(this->hwnd()); }

	/// Deletes all nodes at once.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_deleteallitems
	const tree_view& delete_all_items() const
	{
		if (!TreeView_DeleteAllItems(this->hwnd())) {
			throw std::runtime_error("TreeView_DeleteAllItems failed to delete all items.");
		}
		return *this;
	}

	/// Retrieves all root nodes.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_getroot
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_getnextsibling
	[[nodiscard]] std::vector<tree_view_item> root_items() const
	{
		std::vector<tree_view_item> roots;

		HTREEITEM hti = TreeView_GetRoot(this->hwnd());
		while (hti) {
			roots.emplace_back(this->hwnd(), hti);
			hti = TreeView_GetNextSibling(this->hwnd(), hti);
		}
		return roots;
	}

	/// Retrieves the first root node, if any.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_getroot
	[[nodiscard]] std::optional<tree_view_item> first_root() const noexcept
	{
		HTREEITEM hti = TreeView_GetRoot(this->hwnd());
		if (hti) {
			return tree_view_item{this->hwnd(), hti};
		}
		return {};
	}

	/// Retrieves the first visible node, if any.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_getfirstvisible
	[[nodiscard]] std::optional<tree_view_item> first_visible() const noexcept
	{
		HTREEITEM hti = TreeView_GetFirstVisible(this->hwnd());
		if (hti) {
			return tree_view_item{this->hwnd(), hti};
		}
		return {};
	}

	/// Retrieves the last visible node, if any.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_getlastvisible
	[[nodiscard]] std::optional<tree_view_item> last_visible() const noexcept
	{
		HTREEITEM hti = TreeView_GetLastVisible(this->hwnd());
		if (hti) {
			return tree_view_item{this->hwnd(), hti};
		}
		return {};
	}

	/// Retrieves the number of visible nodes.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_getvisiblecount
	[[nodiscard]] size_t visible_item_count() const noexcept
	{
		return TreeView_GetVisibleCount(this->hwnd());
	}

	/// Retrieves the selected node, if any.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_getselection
	[[nodiscard]] std::optional<tree_view_item> selected_item() const noexcept
	{
		HTREEITEM hti = TreeView_GetSelection(this->hwnd());
		if (hti) {
			return tree_view_item{this->hwnd(), hti};
		}
		return {};
	}

	/// Sets or unsets the given TVS_EX styles.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_setextendedstyle
	const tree_view& set_extended_tv_style(
		bool isSet, DWORD tvExStyles) const noexcept
	{
		TreeView_SetExtendedStyle(this->hwnd(), tvExStyles, isSet ? tvExStyles : 0);
		return *this;
	}

	/// Retrieves the TVS_EX styles.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_getextendedstyle
	[[nodiscard]] size_t extended_tv_style() const noexcept
	{
		return TreeView_GetExtendedStyle(this->hwnd());
	}

	/// Sets the associated image list.
	/// The imageList object is shared, and must remain valid.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_setimagelist
	tree_view& set_image_list(
		const wl::image_list& imgList, DWORD tvsilType = TVSIL_NORMAL) noexcept
	{
		// This method is non-const because it's usually called during object creation,
		// which chains many non-const methods.
		TreeView_SetImageList(this->hwnd(), imgList.himagelist(), tvsilType);
		return *this;
	}
};

}//namespace wl