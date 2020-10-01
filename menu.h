/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <optional>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <Windows.h>
#include "str.h"

namespace wl {

/// Simply holds a menu handle (HMENU), not freeing it in destructor.
/// @see https://docs.microsoft.com/en-us/windows/win32/menurc/about-menus
class menu {
protected:
	HMENU _hMenu = nullptr;

public:
	virtual ~menu() { }

	/// Default constructor.
	menu() = default;

	/// Copy constructor.
	menu(const menu&) = default;

	/// Constructor. Simply stores the HMENU handle.
	explicit menu(HMENU hMenu) noexcept : _hMenu{hMenu} { }

	/// Copy assignment operator.
	menu& operator=(const menu&) = default;

	/// Equality operator.
	bool operator==(const menu& other) const noexcept { return this->_hMenu == other._hMenu; }
	/// Inequality operator.
	bool operator!=(const menu& other) const noexcept { return !this->operator==(other); }

	/// Returns the underlying HMENU handle.
	[[nodiscard]] HMENU hmenu() const noexcept { return this->_hMenu; }

	/// Appends a new item.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-appendmenuw 
	const menu& append_item(int cmdId, std::wstring_view text) const
	{
		if (!AppendMenuW(this->_hMenu, MF_STRING, cmdId, text.data())) {
			throw std::system_error(GetLastError(), std::system_category(),
				"AppendMenu failed.");
		}
		return *this;
	}

	/// Appends a new separator.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-appendmenuw 
	const menu& append_separator() const
	{
		if (!AppendMenuW(this->_hMenu, MF_SEPARATOR, 0, nullptr)) {
			throw std::system_error(GetLastError(), std::system_category(),
				"AppendMenu failed.");
		}
		return *this;
	}

	/// Calls CreateMenuPopup() and appends the new submenu.
	/// @returns The newly appended submenu.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-appendmenuw 
	menu append_submenu(std::wstring_view text) const
	{
		HMENU pop = _create_submenu();

		if (!AppendMenuW(this->_hMenu, MF_STRING | MF_POPUP,
			reinterpret_cast<UINT_PTR>(pop), text.data()))
		{
			throw std::system_error(GetLastError(), std::system_category(),
				"AppendMenu failed.");
		}

		return menu{pop};
	}

	/// Inserts a new item right before the item with the given command ID.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-insertmenuw
	const menu& insert_item_before_cmd(
		int cmdIdBefore, int newCmdId, std::wstring_view text) const
	{
		return this->_insert_item_before(cmdIdBefore, newCmdId, text, false);
	}
	/// Inserts a new item right before the item at the given position.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-insertmenuw
	const menu& insert_item_before_pos(
		size_t posBefore, int newCmdId, std::wstring_view text) const
	{
		return this->_insert_item_before(
			static_cast<UINT>(posBefore), newCmdId, text, true);
	}

	/// Calls CreateMenuPopup() and inserts the new submenu right before the item with the given command ID.
	/// @returns The newly inserted submenu.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-insertmenuw
	menu insert_submenu_before_cmd(int cmdIdBefore, std::wstring_view text) const
	{
		return this->_insert_submenu_before(cmdIdBefore, text, false);
	}
	/// Calls CreateMenuPopup() and inserts the new submenu right before the item at the given position.
	/// @returns The newly inserted submenu.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-insertmenuw
	menu insert_submenu_before_pos(size_t posBefore, std::wstring_view text) const
	{
		return this->_insert_submenu_before(
			static_cast<UINT>(posBefore), text, true);
	}

	/// Retrieves the command ID of the menu item at the given position, if any
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmenuitemid
	[[nodiscard]] std::optional<int> cmd_by_pos(size_t pos) const
	{
		int cmdId = GetMenuItemID(this->_hMenu, static_cast<int>(pos));
		if (cmdId == -1) {
			return {};
		}
		return {cmdId};
	}

	/// Retrieves the sub menu at the given position, if any.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getsubmenu
	[[nodiscard]] std::optional<menu> sub_menu(size_t pos) const
	{
		HMENU hSub = GetSubMenu(this->_hMenu, static_cast<int>(pos));
		if (!hSub) {
			return {};
		}
		return {menu{hSub}};
	}

	/// Retrieves the number of items.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmenuitemcount
	size_t item_count() const
	{
		int count = GetMenuItemCount(this->_hMenu);
		if (count == -1) {
			throw std::system_error(GetLastError(), std::system_category(),
				"GetMenuItemCount failed.");
		}
		return count;
	}

	/// Deletes all items at once.
	const menu& delete_all_items() const
	{
		for (size_t i = this->item_count(); i-- > 0; ) {
			this->delete_by_pos(i);
		}
		return *this;
	}

	/// Deletes the item with the given command ID.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-deletemenu
	const menu& delete_by_cmd(int cmdId) const { return this->_delete(cmdId, false); }
	/// Deletes the item at the given position.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-deletemenu
	const menu& delete_by_pos(size_t pos) const { return this->_delete(static_cast<UINT>(pos), true); }

	/// Enables or disables the item with the given command ID.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enablemenuitem
	const menu& enable_by_cmd(int cmdId, bool isEnabled) const { return this->_enable(cmdId, isEnabled, false); }
	/// Enables or disables the item at the given position.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enablemenuitem
	const menu& enable_by_pos(size_t pos, bool isEnabled) const { return this->_enable(static_cast<UINT>(pos), isEnabled, true); }

	/// Enables or disables the items with the given command IDs at once.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enablemenuitem
	const menu& enable_by_cmd(std::initializer_list<int> cmdIds, bool isEnabled) const
	{
		for (int cmdId : cmdIds) {
			this->enable_by_cmd(cmdId, isEnabled);
		}
		return *this;
	}
	/// Enables or disables the items at the given positions at once.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enablemenuitem
	const menu& enable_by_pos(std::initializer_list<size_t> poss, bool isEnabled) const
	{
		for (size_t pos : poss) {
			this->enable_by_pos(pos, isEnabled);
		}
		return *this;
	}

	/// Sets the text of the item with the given command ID.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setmenuiteminfow
	const menu& set_text_by_cmd(int cmdId, std::wstring_view text) const { return this->_set_text(cmdId, text, false); }
	/// Sets the text of the item at the given position.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setmenuiteminfow
	const menu& set_text_by_pos(size_t pos, std::wstring_view text) const { return this->_set_text(static_cast<UINT>(pos), text, true); }

	/// Retrieves the text of the item with the given command ID.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmenuiteminfow
	[[nodiscard]] std::wstring text_by_cmd(int cmdId) const { return this->_text(cmdId, false); }
	/// Retrieves the text of the item at the given position.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmenuiteminfow
	[[nodiscard]] std::wstring text_by_pos(size_t pos) const { return this->_text(static_cast<UINT>(pos), true); }

	/// Shows the floating menu anchored at the given coordinates with TrackPopupMenu().
	/// If hCoordsRelativeTo is null, coordinates must be relative to hParent.
	/// This function will block until the menu disappears.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-trackpopupmenu
	const menu& show_at_point(
		HWND hParent, POINT pt, HWND hWndCoordsRelativeTo) const
	{
		POINT ptParent = pt; // receives coordinates relative to hParent
		if (!ClientToScreen(hWndCoordsRelativeTo ?
			hWndCoordsRelativeTo : hParent, &ptParent)) // to screen coordinates
		{
			throw std::runtime_error("ClientToScreen failed.");
		}

		SetForegroundWindow(hParent);

		if (!TrackPopupMenu(this->_hMenu, TPM_LEFTBUTTON,
			ptParent.x, ptParent.y, 0, hParent, nullptr)) // owned by hParent, so messages go to it
		{
			throw std::system_error(GetLastError(), std::system_category(),
				"TrackPopupMenu failed.");
		}

		PostMessageW(hParent, WM_NULL, 0, 0); // http://msdn.microsoft.com/en-us/library/ms648002%28VS.85%29.aspx
		return *this;
	}

private:
	const menu& _delete(UINT cmdOrPos, bool byPos) const
	{
		if (!DeleteMenu(this->_hMenu, cmdOrPos, byPos ? MF_BYPOSITION : MF_BYCOMMAND)) {
			throw std::system_error(GetLastError(), std::system_category(),
				str::unicode_to_ansi(
					str::format(L"DeleteMenu %d failed.", cmdOrPos) ));
		}
		return *this;
	}

	const menu& _enable(UINT cmdOrPos, bool isEnabled, bool byPos) const
	{
		UINT flags = (isEnabled ? MF_ENABLED : MF_GRAYED)
			| (byPos ? MF_BYPOSITION : MF_BYCOMMAND);

		if (EnableMenuItem(this->_hMenu, cmdOrPos, flags) == -1) {
			throw std::logic_error(
				str::unicode_to_ansi(
					str::format(L"The menu item %d doesn't exist.", cmdOrPos) ));
		}
		return *this;
	}

	const menu& _insert_item_before(
		UINT cmdOrPosBefore, int newCmdId, std::wstring_view text, bool byPos) const
	{
		UINT flag = byPos ? MF_BYPOSITION : MF_BYCOMMAND;

		if (!InsertMenuW(this->_hMenu, cmdOrPosBefore,
			flag | MF_STRING, newCmdId, text.data() ))
		{
			throw std::system_error(GetLastError(), std::system_category(),
				str::unicode_to_ansi(
					str::format(L"InsertMenu failed for \"%s\".", text) ));
		}
		return *this;
	}

	menu _insert_submenu_before(
		UINT cmdOrPosBefore, std::wstring_view text, bool byPos) const
	{
		UINT flag = byPos ? MF_BYPOSITION : MF_BYCOMMAND;
		HMENU pop = _create_submenu();

		if (!InsertMenuW(this->_hMenu, cmdOrPosBefore, flag | MF_POPUP,
			reinterpret_cast<UINT_PTR>(pop), text.data() ))
		{
			throw std::system_error(GetLastError(), std::system_category(),
				str::unicode_to_ansi(
					str::format(L"InsertMenu failed for \"%s\".", text) ));
		}

		return menu{pop};
	}

	const menu& _set_text(UINT cmdOrPos, std::wstring_view text, bool byPos) const
	{
		MENUITEMINFOW mii{};
		mii.cbSize = sizeof(MENUITEMINFOW);
		mii.fMask = MIIM_STRING;
		mii.dwTypeData = const_cast<wchar_t*>(text.data());

		if (!SetMenuItemInfoW(this->_hMenu, cmdOrPos, byPos, &mii)) {
			throw std::system_error(GetLastError(), std::system_category(),
				str::unicode_to_ansi(
					str::format(L"InsertMenu failed for \"%s\".", text) ));
		}
		return *this;
	}

	[[nodiscard]] const std::wstring _text(UINT cmdOrPos, bool byPos) const
	{
		MENUITEMINFOW mii{};
		mii.cbSize = sizeof(MENUITEMINFOW);
		mii.fMask = MIIM_STRING;

		if (!GetMenuItemInfoW(this->_hMenu, cmdOrPos, byPos, &mii)) { // retrieve length
			throw std::system_error(GetLastError(), std::system_category(),
				str::unicode_to_ansi(
					str::format(L"GetMenuItemInfo failed to retrieve text length from %d.", cmdOrPos) ));
		}
		++mii.cch; // add room for terminating null

		std::wstring buf(mii.cch, L'\0');
		mii.dwTypeData = &buf[0];
		if (!GetMenuItemInfoW(this->_hMenu, cmdOrPos, byPos, &mii)) {
			throw std::system_error(GetLastError(), std::system_category(),
				str::unicode_to_ansi(
					str::format(L"GetMenuItemInfo failed to retrieve text from %d.", cmdOrPos) ));
		}
		return buf;
	}

	[[nodiscard]] static HMENU _create_submenu()
	{
		HMENU pop = CreatePopupMenu();
		if (!pop) {
			throw std::system_error(GetLastError(), std::system_category(),
				"CreatePopupMenu failed.");
		}
		return pop;
	}
};

/// Manages a horizontal main window menu.
/// Calls CreateMenu() in constructor.
/// Supposed to be attached to a window, which will destroy it automatically.
/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createmenu
class menu_main final : public menu {
public:
	/// Default constructor. Calls CreateMenu().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createmenu
	menu_main() : menu{CreateMenu()}
	{
		if (!this->_hMenu) {
			throw std::system_error(GetLastError(), std::system_category(),
				"CreateMenu failed.");
		}
	}

	/// Move constructor.
	menu_main(menu_main&& other) noexcept { this->operator=(std::move(other)); }

	/// Move assignment operator.
	menu_main& operator=(menu_main&& other) noexcept
	{
		DestroyMenu(this->_hMenu);
		this->_hMenu = nullptr;
		std::swap(this->_hMenu, other._hMenu);
		return *this;
	}
};

/// Manages a popup menu.
/// Calls CreatePopupMenu() in constructor, DestroyMenu() in destructor.
/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createpopupmenu
/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-destroymenu
class menu_popup final : public menu {
public:
	/// Destructor. Calls destroy().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-destroymenu
	~menu_popup() { this->destroy(); }

	/// Default constructor. Calls CreatePopupMenu().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createpopupmenu
	menu_popup() : menu{CreatePopupMenu()}
	{
		if (!this->_hMenu) {
			throw std::system_error(GetLastError(), std::system_category(),
				"CreatePopupMenu failed.");
		}
	}

	/// Move constructor.
	menu_popup(menu_popup&& other) noexcept { this->operator=(std::move(other)); }

	/// Move assignment operator.
	menu_popup& operator=(menu_popup&& other) noexcept
	{
		this->destroy();
		std::swap(this->_hMenu, other._hMenu);
		return *this;
	}

	/// Calls DestroyMenu().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-destroymenu
	void destroy() noexcept
	{
		if (this->_hMenu) {
			DestroyMenu(this->_hMenu);
			this->_hMenu = nullptr;
		}
	}
};

/// Manages a menu loaded from the resource.
/// Calls LoadMenu() in constructor.
/// Loaded resources are automatically destroyed by the system.
/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadmenuw
class menu_resource final : public menu {
public:
	/// Default constructor.
	menu_resource() = default;

	/// Constructor. Calls load().
	explicit menu_resource(int menuId) { this->load(menuId); }

	/// Move constructor.
	menu_resource(menu_resource&& other) noexcept { this->operator=(std::move(other)); }

	/// Move assignment operator.
	menu_resource& operator=(menu_resource&& other) noexcept
	{
		this->_hMenu = nullptr;
		std::swap(this->_hMenu, other._hMenu);
		return *this;
	}

	/// Calls LoadMenu().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadmenuw	
	menu_resource& load(int menuId)
	{
		this->_hMenu = LoadMenuW(GetModuleHandle(nullptr), MAKEINTRESOURCEW(menuId));
		if (!this->_hMenu) {
			throw std::system_error(GetLastError(), std::system_category(),
				"LoadMenu failed.");
		}
		return *this;
	}
};

}//namespace wl