/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <Windows.h>
#include <VersionHelpers.h>
#include "interfaces.h"
#include "../str.h"

namespace _wli {

// Base to all GDI objects.
// Calls DeleteObject() in destructor.
// Can be instantiated only by derived classes.
template<typename H>
class gdi_obj {
protected:
	H _h = nullptr;

	gdi_obj() = default;
	gdi_obj(H hGdiObj) noexcept : _h{hGdiObj} { }

	gdi_obj& operator=(H hGdiObj) noexcept
	{
		this->delete_object();
		this->_h = hGdiObj;
		return *this;
	}

public:
	virtual ~gdi_obj() { this->delete_object(); }
	gdi_obj(gdi_obj&& other) noexcept { this->operator=(std::move(other)); } // movable only

	gdi_obj& operator=(gdi_obj&& other) noexcept
	{
		this->delete_object();
		std::swap(this->_h, other._h);
		return *this;
	}

	// Returns the underlying handle converted to HGDIOBJ.
	[[nodiscard]] HGDIOBJ hgdiobj() const noexcept { return this->_h; }

	// Calls DeleteObject().
	void delete_object() noexcept
	{
		if (this->_h) {
			DeleteObject(this->_h);
			this->_h = nullptr;
		}
	}
};

}//namespace _wli

namespace wl::gdi {

/// Manages an HBITMAP resource.
/// Calls DeleteObject() in destructor.
class bitmap final : public _wli::gdi_obj<HBITMAP> {
public:
	/// Default constructor.
	bitmap() = default;

	/// Constructor. Takes ownership of handle.
	bitmap(HBITMAP hBmp) noexcept : gdi_obj{hBmp} { }

	/// Move constructor.
	bitmap(bitmap&& other) noexcept : gdi_obj{std::move(other)} {  }

	/// Assignment operator. Takes ownership of handle.
	bitmap& operator=(HBITMAP hBmp) noexcept { this->gdi_obj::operator=(hBmp); return *this; }

	/// Move assignment operator.
	bitmap& operator=(bitmap&& other) noexcept { this->gdi_obj::operator=(std::move(other)); return *this; }

	/// Returns the underlying HBITMAP handle.
	[[nodiscard]] HBITMAP hbitmap() const noexcept { return this->_h; }
};

/// Manages an HBRUSH resource.
/// Calls DeleteObject() in destructor.
class brush final : public _wli::gdi_obj<HBRUSH> {
public:
	/// Default constructor.
	brush() = default;

	/// Constructor. Takes ownership of handle.
	brush(HBRUSH hBrush) noexcept : gdi_obj{hBrush} { }

	/// Move constructor.
	brush(brush&& other) noexcept : gdi_obj{std::move(other)} {  }

	/// Assignment operator. Takes ownership of handle.
	brush& operator=(HBRUSH hBrush) noexcept { this->gdi_obj::operator=(hBrush); return *this; }

	/// Move assignment operator.
	brush& operator=(brush&& other) noexcept { this->gdi_obj::operator=(std::move(other)); return *this; }

	/// Returns the underlying HBRUSH handle.
	[[nodiscard]] HBRUSH hbrush() const noexcept { return this->_h; }
};

/// Manages an HFONT resource.
/// Calls DeleteObject() in destructor.
class font final : public _wli::gdi_obj<HFONT> {
public:
	/// Font decoration options.
	enum class deco : BYTE {
		/// No decorations.
		NONE      = 0b0000'0000,
		/// Bold.
		BOLD      = 0b0000'0001,
		/// Italic.
		ITALIC    = 0b0000'0010,
		/// Strikeout.
		STRIKEOUT = 0b0000'0100,
		/// Underline.
		UNDERLINE = 0b0000'1000
	};

	/// Default constructor.
	font() = default;

	/// Constructor. Takes ownership of handle.
	font(HFONT hFont) noexcept : gdi_obj{hFont} { }

	/// Move constructor.
	font(font&& other) noexcept : gdi_obj{std::move(other)} { }

	/// Assignment operator. Takes ownership of handle.
	font& operator=(HFONT hFont) noexcept { this->gdi_obj::operator=(hFont); return *this; }

	/// Move assignment operator.
	font& operator=(font&& other) noexcept { this->gdi_obj::operator=(std::move(other)); return *this; }

	/// Returns the underlying HFONT handle.
	[[nodiscard]] HFONT hfont() const noexcept { return this->_h; }

	/// Simplified way to create a font.
	font& create(std::wstring_view faceName, BYTE size, deco style = deco::NONE)
	{
		this->delete_object();
		LOGFONT lf{};
		lstrcpyW(lf.lfFaceName, faceName.data());
		lf.lfHeight = -(size + 3);

		auto hasDeco = [=](deco yourDeco) noexcept -> BOOL {
			return (static_cast<BYTE>(style) &
				static_cast<BYTE>(yourDeco)) != 0 ? TRUE : FALSE;
		};

		lf.lfWeight    = hasDeco(deco::BOLD) ? FW_BOLD : FW_DONTCARE;
		lf.lfItalic    = hasDeco(deco::ITALIC);
		lf.lfUnderline = hasDeco(deco::UNDERLINE);
		lf.lfStrikeOut = hasDeco(deco::STRIKEOUT);

		return this->create_indirect(lf);
	}

	/// Creates a font with all possible options.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createfontindirectw
	font& create_indirect(const LOGFONT& lplf)
	{
		this->delete_object();
		this->_h = CreateFontIndirectW(&lplf);
		if (!this->_h) {
			throw std::runtime_error(
				str::unicode_to_ansi(
					str::format(L"CreateFontIndirect failed for \"%s\".", lplf.lfFaceName) ));
		}
		return *this;
	}

	/// Creates the same exact font used by UI, usually Segoe 9.
	font& create_ui()
	{
		this->delete_object();

		NONCLIENTMETRICS ncm{};
		ncm.cbSize = sizeof(ncm);
		if (!IsWindowsVistaOrGreater()) {
			ncm.cbSize -= sizeof(ncm.iBorderWidth);
		}
		SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
		return this->create_indirect(ncm.lfMenuFont);
	}

	/// Sends WM_SETFONT to a control, to set the font on it.
	/// @see https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-setfont
	const font& set_on_control(const i_control& child) const noexcept
	{
		SendMessageW(child.hwnd(), WM_SETFONT,
			reinterpret_cast<WPARAM>(_h), TRUE);
		return *this;
	}

	/// Sends WM_SETFONT to many controls at once, to set the font on them.
	/// @see https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-setfont
	const font& set_on_control(
		std::initializer_list<std::reference_wrapper<const i_control>> children) const noexcept
	{
		for (const i_control& child : children) {
			this->set_on_control(child);
		}
		return *this;
	}

	/// Checks if a font is currently installed.
	/// Face names are case-insensitive.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-enumfontfamiliesw
	[[nodiscard]] static bool face_name_exists(std::wstring_view faceName)
	{
		// http://cboard.cprogramming.com/windows-programming/90066-how-determine-if-font-support-unicode.html

		bool isInstalled = false;

		HDC hdc = GetDC(nullptr);
		if (!hdc) {
			throw std::system_error(GetLastError(), std::system_category(),
				str::unicode_to_ansi(
					str::format(L"GetDC failed when checking if \"%s\" exists") ));
		}

		EnumFontFamiliesW(hdc, faceName.data(),
			[](const LOGFONT*, const TEXTMETRIC*, DWORD, LPARAM lp) noexcept -> int
			{
				bool* pIsInstalled = reinterpret_cast<bool*>(lp);
				*pIsInstalled = true; // if we're here, font does exist
				return 0;
			},
			reinterpret_cast<LPARAM>(&isInstalled));

		ReleaseDC(nullptr, hdc);
		return isInstalled;
	}
};

/// Manages an HPEN resource.
/// Calls DeleteObject() in destructor.
class pen final : public _wli::gdi_obj<HPEN> {
public:
	/// Default constructor.
	pen() = default;

	/// Constructor. Takes ownership of handle.
	pen(HPEN hPen) noexcept : gdi_obj{hPen} { }

	/// Move constructor.
	pen(pen&& other) noexcept : gdi_obj{std::move(other)} {  }

	/// Assignment operator. Takes ownership of handle.
	pen& operator=(HPEN hPen) noexcept { this->gdi_obj::operator=(hPen); return *this; }

	/// Move operator.
	pen& operator=(pen&& other) noexcept { this->gdi_obj::operator=(std::move(other)); return *this; }

	/// Returns the underlying HPEN handle.
	[[nodiscard]] HPEN hpen() const noexcept { return this->_h; }
};

/// Manages an HRGN resource.
/// Calls DeleteObject() in destructor.
class rgn final : public _wli::gdi_obj<HRGN> {
public:
	/// Default constructor.
	rgn() = default;

	/// Constructor. Takes ownership of handle.
	rgn(HRGN hRgn) noexcept : gdi_obj{hRgn} { }

	/// Move constructor.
	rgn(rgn&& other) noexcept : gdi_obj{std::move(other)} {  }

	/// Assignment operator. Takes ownership of handle.
	rgn& operator=(HRGN hRgn) noexcept { this->gdi_obj::operator=(hRgn); return *this; }

	/// Move assignment operator.
	rgn& operator=(rgn&& other) noexcept { this->gdi_obj::operator=(std::move(other)); return *this; }

	/// Returns the underlying HRGN handle.
	[[nodiscard]] HRGN hrgn() const noexcept { return this->_h; }
};

}//namespace wl::gdi

DEFINE_ENUM_FLAG_OPERATORS(wl::gdi::font::deco)

namespace _wli {

// Used on all old native controls, which don't automatically have it.
inline wl::gdi::font globalUiFont;

}//namespace _wli