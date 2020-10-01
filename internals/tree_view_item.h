/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <optional>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <Windows.h>
#include <CommCtrl.h>
#include "../str.h"

namespace wl {

/// A single item of a tree view.
class tree_view_item final {
private:
	HWND _hTree = nullptr;
	HTREEITEM _hTreeItem = nullptr;

public:
	/// Constructor.
	explicit tree_view_item(HWND owner, HTREEITEM hTreeItem) noexcept :
		_hTree{owner}, _hTreeItem{hTreeItem} { }

	/// Equality operator.
	[[nodiscard]] bool operator==(const tree_view_item& other) const noexcept
	{
		return this->_hTree == other._hTree
			&& this->_hTreeItem == other._hTreeItem;
	}

	/// Inequality operator.
	[[nodiscard]] bool operator!=(const tree_view_item& other) const noexcept { return !this->operator==(other); }

	/// Returns the HWND of the tree view to which this item belongs.
	[[nodiscard]] HWND htree() const noexcept { return this->_hTree; }

	/// Returns the underlying HTREEITEM handle.
	[[nodiscard]] HTREEITEM htreeitem() const noexcept { return this->_hTreeItem; }	

	/// Adds a new child node.
	/// @return The newly added child node.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_insertitem
	tree_view_item add_child(std::wstring_view text) const
	{
		TVINSERTSTRUCTW tvi{};
		tvi.hParent = this->_hTreeItem;
		tvi.hInsertAfter = TVI_LAST;
		tvi.itemex.mask = TVIF_TEXT;
		tvi.itemex.pszText = const_cast<wchar_t*>(text.data());

		HTREEITEM newItem = TreeView_InsertItem(this->_hTree, &tvi);
		if (!newItem) {
			throw std::runtime_error(
				wl::str::unicode_to_ansi(
					wl::str::format(L"TreeView_InsertItem failed \"%s\".", text) ));
		}
		return tree_view_item{this->_hTree, newItem};
	}

	/// Adds a new child node.
	/// You must attach an image list to see the icon.
	/// @return The newly added child node.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_insertitem
	tree_view_item add_child_with_icon(std::wstring_view text, int iIcon) const
	{
		TVINSERTSTRUCTW tvi{};
		tvi.hParent = this->_hTreeItem;
		tvi.hInsertAfter = TVI_LAST;
		tvi.itemex.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.itemex.pszText = const_cast<wchar_t*>(text.data());
		tvi.itemex.iImage = iIcon;
		tvi.itemex.iSelectedImage = iIcon;

		HTREEITEM newItem = TreeView_InsertItem(this->_hTree, &tvi);
		if (!newItem) {
			throw std::runtime_error(
				wl::str::unicode_to_ansi(
					wl::str::format(L"TreeView_InsertItem failed \"%s\".", text) ));
		}
		return tree_view_item{this->_hTree, newItem};
	}

	/// Retrieves the first child node, if any.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_getchild
	[[nodiscard]] std::optional<tree_view_item> first_child() const noexcept
	{
		HTREEITEM hti = TreeView_GetChild(this->_hTree, this->_hTreeItem);
		if (hti) {
			return tree_view_item{this->_hTree, hti};
		}
		return {};
	}

	/// Retrieves the next sibling node, if any.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_getnextsibling
	[[nodiscard]] std::optional<tree_view_item> next_sibling() const noexcept
	{
		HTREEITEM hti = TreeView_GetNextSibling(this->_hTree, this->_hTreeItem);
		if (hti) {
			return tree_view_item{this->_hTree, hti};
		}
		return {};
	}

	/// Retrieves the next visible node, if any.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_getnextvisible
	[[nodiscard]] std::optional<tree_view_item> next_visible() const noexcept
	{
		HTREEITEM hti = TreeView_GetNextVisible(this->_hTree, this->_hTreeItem);
		if (hti) {
			return tree_view_item{this->_hTree, hti};
		}
		return {};
	}

	/// Retrieves the parent node, if any.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_getparent
	[[nodiscard]] std::optional<tree_view_item> parent() const noexcept
	{
		HTREEITEM hti = TreeView_GetParent(this->_hTree, this->_hTreeItem);
		if (hti) {
			return tree_view_item{this->_hTree, hti};
		}
		return {};
	}

	/// Tells if the node is a root node.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_getparent
	[[nodiscard]] bool is_root() const noexcept { return TreeView_GetParent(this->_hTree, this->_hTreeItem) == nullptr; }

	/// Deletes the node.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_deleteitem
	void remove() const
	{
		if (!TreeView_DeleteItem(this->_hTree, this->_hTreeItem)) {
			throw std::runtime_error("TreeView_DeleteItem failed.");
		}
	}

	/// Ensures that a tree-view item is visible, expanding the
	/// parent item or scrolling the tree-view control, if needed.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_ensurevisible
	const tree_view_item& ensure_visible() const noexcept
	{
		TreeView_EnsureVisible(this->_hTree, this->_hTreeItem);
		return *this;
	}

	/// Expands or collapses the node.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_expand
	const tree_view_item& set_expanded(bool isExpanded) const noexcept
	{
		TreeView_Expand(this->_hTree, this->_hTreeItem,
			isExpanded ? TVE_EXPAND : TVE_COLLAPSE);
		return *this;
	}

	/// Toggles the node, expanded or collapsed.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_expand
	const tree_view_item& toggle_expanded() const noexcept
	{
		TreeView_Expand(this->_hTree, this->_hTreeItem, TVE_TOGGLE);
		return *this;
	}

	/// Tells if the node is currently expanded.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_getitemstate
	[[nodiscard]] bool expanded() const noexcept
	{
		return (
			TreeView_GetItemState(this->_hTree,
				this->_hTreeItem, TVIS_EXPANDED) & TVIS_EXPANDED)
			!= 0;
	}

	/// Sets the text.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_setitem
	const tree_view_item& set_text(std::wstring_view text) const
	{
		TVITEMEX tvi{};
		tvi.hItem = this->_hTreeItem;
		tvi.mask = TVIF_TEXT;
		tvi.pszText = const_cast<wchar_t*>(text.data());

		if (!TreeView_SetItem(this->_hTree, &tvi)) {
			throw std::runtime_error(
				wl::str::unicode_to_ansi(
					wl::str::format(L"TreeView_SetItem failed to set text \"%s\".", text) ));
		}
		return *this;
	}

	/// Retrieves the text.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-treeview_getitem
	[[nodiscard]] std::wstring text() const
	{
		wchar_t tmpBuf[MAX_PATH]{}; // arbitrary length

		TVITEMEX tvi{};
		tvi.hItem = this->_hTreeItem;
		tvi.mask = TVIF_TEXT;
		tvi.cchTextMax = ARRAYSIZE(tmpBuf);
		tvi.pszText = tmpBuf;

		TreeView_GetItem(this->_hTree, &tvi);
		return tvi.pszText;
	}
};

}//namespace wl