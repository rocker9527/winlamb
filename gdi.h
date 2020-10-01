/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <stdexcept>
#include <system_error>
#include <type_traits>
#include <Windows.h>
#include "internals/gdi_obj.h"
#include "internals/interfaces.h"
#include "str.h"

/// GDI objects.
/// @see https://docs.microsoft.com/en-us/windows/win32/gdi/windows-gdi
namespace wl::gdi {

/// Simply holds a device context handle (HDC), not freeing it in destructor.
/// Cannot be directly instantiated.
/// @see https://docs.microsoft.com/en-us/windows/win32/gdi/about-device-contexts
class dc {
protected:
	HDC _hDc = nullptr;

public:
	virtual ~dc() { }

	/// Default constructor.
	dc() = default;

	/// Copy constructor.
	dc(const dc&) = default;

	/// Constructor. Simply stores the HDC handle.
	explicit dc(HDC hDc) noexcept : _hDc{hDc} { }

	/// Copy assignment operator.
	dc& operator=(const dc&) = default;

	/// Equality operator.
	bool operator==(const dc& other) const noexcept { return this->_hDc == other._hDc; }
	/// Inequality operator.
	bool operator!=(const dc& other) const noexcept { return !this->operator==(other); }

	/// Returns the underlying HDC handle.
	[[nodiscard]] HDC hdc() const noexcept { return this->_hDc; }

	/// Calls DrawEdge().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-drawedge
	const dc& draw_edge(RECT rc, UINT edge, UINT grfFlags) const
	{
		if (!DrawEdge(this->_hDc, &rc, edge, grfFlags)) {
			throw std::runtime_error(
				str::unicode_to_ansi(
					str::format(L"DrawEdge failed %d, %d.", rc.left, rc.top) ));
		}
		return *this;
	}

	/// Calls DrawEdge().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-drawedge
	const dc& draw_edge(POINT leftTop, SIZE rightBottom, UINT edge, UINT grfFlags) const
	{
		return this->draw_edge(_ptsz_to_rc(leftTop, rightBottom), edge, grfFlags);
	}

	/// Calls DrawText().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-drawtextw
	const dc& draw_text(std::wstring_view text, RECT rc,
		UINT format = DT_LEFT | DT_TOP) const
	{
		if (!DrawTextW(this->_hDc, text.data(),
			static_cast<int>(text.length()), &rc, format))
		{
			throw std::runtime_error(
				str::unicode_to_ansi(
					str::format(L"DrawText failed \"%s\".", text) ));
		}
		return *this;
	}

	/// Calls DrawText().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-drawtextw
	const dc& draw_text(std::wstring_view text, POINT leftTop, SIZE rightBottom,
		UINT format = DT_LEFT | DT_TOP) const
	{
		return this->draw_text(text, _ptsz_to_rc(leftTop, rightBottom), format);
	}

	/// Calls FillRect();
	/// This function includes the left and top borders,
	/// but excludes the right and bottom borders from painting.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-fillrect
	const dc& fill_rect(RECT rc, const brush& b) const
	{
		if (!FillRect(this->_hDc, &rc, b.hbrush())) {
			throw std::runtime_error(
				str::unicode_to_ansi(
					str::format(L"FillRect failed %d, %d.", rc.left, rc.top) ));
		}
		return *this;
	}

	/// Calls FillRect();
	/// This function includes the left and top borders,
	/// but excludes the right and bottom borders from painting.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-fillrect
	const dc& fill_rect(POINT leftTop, SIZE rightBottom, const brush& b) const
	{
		return this->fill_rect(_ptsz_to_rc(leftTop, rightBottom), b.hbrush());
	}

	/// Calls FillRgn().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-fillrgn
	const dc& fill_rgn(const rgn& r, const brush& b) const
	{
		if (!FillRgn(this->_hDc, r.hrgn(), b.hbrush())) {
			throw std::runtime_error("FillRgn failed.");
		}
		return *this;
	}

	/// Calls GetDeviceCaps().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getdevicecaps
	[[nodiscard]] int get_device_caps(int index) const noexcept
	{
		return GetDeviceCaps(this->_hDc, index);
	}

	/// Calls GetTextExtentPoint32().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-gettextextentpoint32w
	[[nodiscard]] SIZE get_text_extent_point(std::wstring_view text) const
	{
		SIZE sz{};
		if (!GetTextExtentPoint32W(this->_hDc,
			text.data(), static_cast<int>(text.length()), &sz) )
		{
			throw std::runtime_error(
				str::unicode_to_ansi(
					str::format(L"GetTextExtentPoint32 failed \"%s\".", text) ));
		}
		return sz;
	}

	/// Calls LineTo().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-lineto
	const dc& line_to(POINT pos) const
	{
		if (!LineTo(this->_hDc, pos.x, pos.y)) {
			throw std::runtime_error(
				str::unicode_to_ansi(
					str::format(L"LineTo failed %d,%d.", pos.x, pos.y) ));
		}
		return *this;
	}

	/// Calls MoveToEx(), then LineTo() 4 times to draw a rectangle.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-movetoex
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-lineto
	const dc& line_to_rect(RECT rc) const
	{
		return this->move_to({rc.left, rc.top})
			.line_to({rc.right, rc.top})
			.line_to({rc.right, rc.bottom})
			.line_to({rc.left, rc.bottom})
			.line_to({rc.left, rc.top});
	}

	/// Calls MoveToEx(), then LineTo() 4 times to draw a rectangle.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-movetoex
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-lineto
	const dc& line_to_rect(POINT leftTop, SIZE rightBottom) const
	{
		return this->line_to_rect(_ptsz_to_rc(leftTop, rightBottom));
	}

	/// Calls MoveToEx().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-movetoex
	const dc& move_to(POINT pos, POINT& prevPosBuf) const
	{
		if (!MoveToEx(this->_hDc, pos.x, pos.y, &prevPosBuf)) {
			throw std::runtime_error(
				str::unicode_to_ansi(
					str::format(L"MoveToEx failed %d,%d.", pos.x, pos.y) ));
		}
		return *this;
	}

	/// Calls MoveToEx().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-movetoex
	const dc& move_to(POINT pos) const
	{
		if (!MoveToEx(this->_hDc, pos.x, pos.y, nullptr)) {
			throw std::runtime_error(
				str::unicode_to_ansi(
					str::format(L"MoveToEx failed %d,%d.", pos.x, pos.y) ));
		}
		return *this;
	}

	/// Calls Polygon().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-polygon
	const dc& polygon(const POINT* points, size_t numPoints) const
	{
		if (!Polygon(this->_hDc, points, static_cast<int>(numPoints))) {
			throw std::runtime_error("Polygon failed.");
		}
		return *this;
	}

	/// Calls Polygon().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-polygon
	const dc& polygon(const std::vector<POINT>& points) const
	{
		return this->polygon(&points[0], points.size());
	}

	/// Calls Polygon() 4 times to draw a rectangle.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-polygon
	const dc& polygon_rect(RECT rc) const
	{
		POINT pts[] = {
			{rc.left, rc.top},
			{rc.right, rc.top},
			{rc.right, rc.bottom},
			{rc.left, rc.bottom}
		};
		return this->polygon(pts, 4);
	}

	/// Calls Polygon() 4 times to draw a rectangle.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-polygon
	const dc& polygon_rect(POINT leftTop, SIZE rightBottom) const
	{
		return this->polygon_rect(_ptsz_to_rc(leftTop, rightBottom));
	}

	/// Calls RestoreDC().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-restoredc
	const dc& restore_dc(int nSavedDC = -1) const noexcept
	{
		RestoreDC(this->_hDc, nSavedDC);
		return *this;
	}

	/// Calls SaveDC().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-savedc
	const dc& save_dc() const
	{
		if (!SaveDC(this->_hDc)) {
			throw std::runtime_error("SaveDC failed.");
		}
		return *this;
	}

	/// Calls SelectObject().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-selectobject
	template<typename GDIOBJ,
		typename = std::enable_if_t<
			std::is_same_v<GDIOBJ, bitmap> ||
			std::is_same_v<GDIOBJ, brush> ||
			std::is_same_v<GDIOBJ, font> ||
			std::is_same_v<GDIOBJ, pen> ||
			std::is_same_v<GDIOBJ, rgn>
		>>
	const dc& select_object(const GDIOBJ& obj) const noexcept
	{
		SelectObject(this->_hDc, obj.hgdiobj());
		return *this;
	}

	/// Calls SetBkColor().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-setbkcolor
	const dc& set_bk_color(COLORREF color) const
	{
		if (SetBkColor(this->_hDc, color) == CLR_INVALID) {
			throw std::runtime_error("SetBkColor failed, CLR_INVALID.");
		}
		return *this;
	}

	/// Calls SetBkMode().
	/// @param mode TRANSPARENT or OPAQUE.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-setbkmode
	const dc& set_bk_mode(int mode) const
	{
		if (!SetBkMode(this->_hDc, mode)) {
			throw std::runtime_error("SetBkMode failed.");
		}
		return *this;
	}

	/// Calls SetTextColor().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-settextcolor
	const dc& set_text_color(COLORREF color) const
	{
		if (SetTextColor(this->_hDc, color) == CLR_INVALID) {
			throw std::runtime_error("SetTextColor failed, CLR_INVALID.");
		}
		return *this;
	}

	/// Calls TextOut().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-textoutw
	const dc& text_out(POINT pos, std::wstring_view text) const
	{
		if (!TextOutW(this->_hDc, pos.x, pos.y,
			text.data(), static_cast<int>(text.length()) ))
		{
			throw std::runtime_error(
				str::unicode_to_ansi(
					str::format(L"TextOut failed \"%s\".", text) ));
		}
		return *this;
	}

private:
	// Transforms POINT + SIZE into RECT.
	[[nodiscard]] static RECT _ptsz_to_rc(POINT pt, SIZE sz) noexcept
	{
		return {
			pt.x,
			pt.y,
			pt.x + sz.cx, // right
			pt.y + sz.cy // bottom
		};
	}
};

/// Manages a device context.
/// Calls CreateCompatibleDC() in constructor, DeleteDC() in destructor.
/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createcompatibledc
/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-deletedc
class dc_compatible final : public dc {
public:
	/// Destructor. Calls delete_dc().
	~dc_compatible() { this->delete_dc(); }

	/// Constructor. Calls CreateCompatibleDC().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createcompatibledc
	explicit dc_compatible(const dc& other) :
		dc{CreateCompatibleDC(other.hdc())}
	{
		if (!this->_hDc) {
			throw std::runtime_error("CreateCompatibleDC failed.");
		}
		this->save_dc();
	}

	/// Move constructor.
	dc_compatible(dc_compatible&& other) noexcept { this->operator=(std::move(other)); }

	/// Move assignment operator.
	dc_compatible& operator=(dc_compatible&& other) noexcept
	{
		this->delete_dc();
		std::swap(this->_hDc, other._hDc);
		return *this;
	}

	/// Calls DeleteDC().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-deletedc
	void delete_dc() noexcept
	{
		if (this->_hDc) {
			this->restore_dc(); // https://www.codeproject.com/Articles/1033/Saving-Drawing-Contexts
			DeleteDC(this->_hDc);
			this->_hDc = nullptr;
		}
	}
};

/// Manages a device context.
/// Calls CreateDC() in constructor, DeleteDC() in destructor.
/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createdcw
/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-deletedc
class dc_created final : public dc {
public:
	/// Destructor. Calls delete_dc().
	~dc_created() { this->delete_dc(); }

	/// Constructor. Calls CreateDC().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createdcw
	dc_created(LPCWSTR pwszDriver, LPCWSTR pwszDevice, LPCWSTR pszPort, const DEVMODEW& pdm) :
		dc{CreateDCW(pwszDriver, pwszDevice, pszPort, &pdm)}
	{
		if (!this->_hDc) {
			throw std::runtime_error("CreateDC failed.");
		}
		this->save_dc();
	}

	/// Move constructor.
	dc_created(dc_created&& other) noexcept { this->operator=(std::move(other)); }

	/// Move assignment operator.
	dc_created& operator=(dc_created&& other) noexcept
	{
		this->delete_dc();
		std::swap(this->_hDc, other._hDc);
		return *this;
	}

	/// Calls DeleteDC().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-deletedc
	void delete_dc() noexcept
	{
		if (this->_hDc) {
			this->restore_dc(); // https://www.codeproject.com/Articles/1033/Saving-Drawing-Contexts
			DeleteDC(this->_hDc);
			this->_hDc = nullptr;
		}
	}
};

/// Manages a device context.
/// Calls GetDC() in constructor, ReleaseDC() in destructor.
/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdc
/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-releasedc
class dc_get final : public dc {
private:
	HWND _hWnd = nullptr;

public:
	/// Destructor. Calls release_dc().
	~dc_get() { this->release_dc(); }

	/// Constructor. Calls GetDC().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdc
	explicit dc_get(const i_window& w) : dc_get{w.hwnd()} { }

	/// Constructor. Calls GetDC().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdc
	explicit dc_get(HWND hWnd) : dc{GetDC(hWnd)}
	{
		if (!this->_hDc) {
			throw std::runtime_error("GetDC failed.");
		}
		this->_hWnd = hWnd;
		this->save_dc();
	}

	/// Move constructor.
	dc_get(dc_get&& other) noexcept { this->operator=(std::move(other)); }

	/// Move assignment operator.
	dc_get& operator=(dc_get&& other) noexcept
	{
		this->release_dc();
		std::swap(this->_hDc, other._hDc);
		std::swap(this->_hWnd, other._hWnd);
		return *this;
	}

	/// Calls ReleaseDC().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-releasedc
	void release_dc() noexcept
	{
		if (this->_hDc) {
			this->restore_dc(); // https://www.codeproject.com/Articles/1033/Saving-Drawing-Contexts
			ReleaseDC(this->_hWnd, this->_hDc);
			this->_hWnd = nullptr;
			this->_hDc = nullptr;
		}
	}
};

/// Manages a device context.
/// Calls GetWindowDC() in constructor, ReleaseDC() in destructor.
/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowdc
/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-releasedc
class dc_get_window final : public dc {
private:
	HWND _hWnd = nullptr;

public:
	/// Destructor. Calls release_dc().
	~dc_get_window() { this->release_dc(); }

	/// Constructor. Calls GetWindowDC().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowdc
	explicit dc_get_window(const i_window& w) : dc_get_window{w.hwnd()} { }

	/// Constructor. Calls GetWindowDC().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowdc
	explicit dc_get_window(HWND hWnd) : dc{GetWindowDC(hWnd)}
	{
		if (!this->_hDc) {
			throw std::runtime_error("GetWindowDC failed.");
		}
		this->_hWnd = hWnd;
		this->save_dc();
	}

	/// Move constructor.
	dc_get_window(dc_get_window&& other) noexcept { this->operator=(std::move(other)); }

	/// Move assignment operator.
	dc_get_window& operator=(dc_get_window&& other) noexcept
	{
		this->release_dc();
		std::swap(this->_hDc, other._hDc);
		std::swap(this->_hWnd, other._hWnd);
		return *this;
	}

	/// Calls ReleaseDC().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-releasedc
	void release_dc() noexcept
	{
		if (this->_hDc) {
			this->restore_dc(); // https://www.codeproject.com/Articles/1033/Saving-Drawing-Contexts
			ReleaseDC(this->_hWnd, this->_hDc);
			this->_hWnd = nullptr;
			this->_hDc = nullptr;
		}
	}
};

/// Manages a device context.
/// Calls BeginPaint() in constructor, EndPaint() in destructor.
/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-beginpaint
/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-endpaint
class dc_paint : public dc {
private:
	HWND _hWnd = nullptr;
	PAINTSTRUCT _ps{};
	SIZE _szClient{};

public:
	/// Destructor. Calls end_paint().
	virtual ~dc_paint() { this->end_paint(); }

	/// Constructor. Calls BeginPaint().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-beginpaint
	explicit dc_paint(const i_window* w) : dc_paint{w->hwnd()} { }

	/// Constructor. Calls BeginPaint().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-beginpaint
	explicit dc_paint(HWND hWnd)
	{
		this->_hDc = BeginPaint(hWnd, &_ps);
		if (!this->_hDc) {
			throw std::runtime_error("BeginPaint failed.");
		}
		this->_hWnd = hWnd;

		RECT rcClient{};
		if (!GetClientRect(hWnd, &rcClient)) {
			throw std::system_error(GetLastError(), std::system_category(),
				"GetClientRect failed.");
		}
		this->_szClient = {rcClient.right, rcClient.bottom}; // cache width & height
		this->save_dc();
	}

	/// Move constructor.
	dc_paint(dc_paint&& other) noexcept { this->operator=(std::move(other)); }

	/// Move assignment operator.
	dc_paint& operator=(dc_paint&& other) noexcept
	{
		this->end_paint();
		std::swap(this->_hDc, other._hDc);
		std::swap(this->_hWnd, other._hWnd);
		std::swap(this->_ps, other._ps);
		std::swap(this->_szClient, other._szClient);
		return *this;
	}

	/// Returns the HWND being painted.
	[[nodiscard]] HWND hwnd() const noexcept { return this->_hWnd; }

	/// Returns the PAINTSTRUCT in use.
	[[nodiscard]] const PAINTSTRUCT& paintstruct() const noexcept { return this->_ps; }

	/// Returns the size of client area being painted.
	[[nodiscard]] SIZE sz_client() const noexcept { return this->_szClient; }

	/// Retrieves the color of the current background brush.
	[[nodiscard]] COLORREF bg_brush_color() const
	{
		ULONG_PTR hbrBg = GetClassLongPtrW(this->_hWnd, GCLP_HBRBACKGROUND);
		if (!hbrBg) {
			throw std::system_error(GetLastError(), std::system_category(),
				"GetClassLongPtr failed.");
		}

		if (hbrBg > 100) {
			// The hbrBackground is a brush handle, not a system color constant.
			// This 100 value is arbitrary, based on system color constants like COLOR_BTNFACE.
			LOGBRUSH logBrush{};
			GetObjectW(reinterpret_cast<HBRUSH>(hbrBg), sizeof(LOGBRUSH), &logBrush);
			return logBrush.lbColor;
		}
		return GetSysColor(static_cast<int>(hbrBg - 1));
	}

	/// Calls EndPaint().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-endpaint
	virtual void end_paint() noexcept
	{
		this->restore_dc(); // https://www.codeproject.com/Articles/1033/Saving-Drawing-Contexts
		EndPaint(this->_hWnd, &this->_ps);
		this->_szClient = {};
		this->_ps = {};
		this->_hWnd = nullptr;
		this->_hDc = nullptr;
	}
};

/// Manages a double-buffered device context.
/// Calls BeginPaint() in constructor, EndPaint() in destructor.
/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-beginpaint
/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-endpaint
class dc_paint_buffered final : public dc_paint {
private:
	HBITMAP _hBmp = nullptr;
	HBITMAP _hBmpOld = nullptr;

public:
	/// Destructor. Calls end_paint().
	~dc_paint_buffered() { this->end_paint(); }

	/// Constructor. Calls BeginPaint() and creates the double buffer.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-beginpaint
	explicit dc_paint_buffered(const i_window* w) : dc_paint_buffered{w->hwnd()} { }

	/// Constructor. Calls BeginPaint() and creates the double buffer.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-beginpaint
	explicit dc_paint_buffered(HWND hWnd) : dc_paint{hWnd}
	{
		this->_hDc = CreateCompatibleDC(this->paintstruct().hdc); // overwrite our painting HDC
		if (!this->_hDc) {
			throw std::runtime_error("CreateCompatibleDC failed.");
		}

		this->_hBmp = CreateCompatibleBitmap(this->paintstruct().hdc,
			this->sz_client().cx, this->sz_client().cy);
		if (!this->_hBmp) {
			throw std::runtime_error("CreateCompatibleBitmap failed.");
		}

		this->_hBmpOld = reinterpret_cast<HBITMAP>(SelectObject(this->_hDc, this->_hBmp));

		RECT rcClient = {0, 0, this->sz_client().cx, this->sz_client().cy};
		if (!FillRect(this->_hDc, &rcClient,
			reinterpret_cast<HBRUSH>(GetClassLongPtrW(this->hwnd(), GCLP_HBRBACKGROUND)) ))
		{
			throw std::runtime_error("FillRect failed.");
		}
		this->save_dc();
	}

	/// Move constructor.
	dc_paint_buffered(dc_paint_buffered&& other) noexcept : // movable only
		dc_paint{std::move(other)}
	{
		this->operator=(std::move(other));
	}

	/// Move assignment operator.
	dc_paint_buffered& operator=(dc_paint_buffered&& other) noexcept
	{
		this->end_paint();
		this->dc_paint::operator=(std::move(other));
		std::swap(this->_hBmp, other._hBmp);
		std::swap(this->_hBmpOld, other._hBmpOld);
		return *this;
	}

	/// Releases the double buffer and calls EndPaint().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-endpaint
	void end_paint() noexcept override
	{
		if (this->_hBmp) {
			this->restore_dc(); // https://www.codeproject.com/Articles/1033/Saving-Drawing-Contexts

			BITMAP bm{}; // http://www.ureader.com/msg/14721900.aspx
			GetObjectW(this->_hBmp, sizeof(bm), &bm);
			BitBlt(this->paintstruct().hdc, 0, 0, bm.bmWidth, bm.bmHeight,
				this->_hDc, 0, 0, SRCCOPY);

			DeleteObject(SelectObject(this->_hDc, this->_hBmpOld));
			DeleteObject(this->_hBmp);
			this->_hBmp = this->_hBmpOld = nullptr;

			DeleteDC(this->_hDc); // restore back, because we overwrote it
			this->_hDc = this->paintstruct().hdc;
		}
		this->dc_paint::end_paint();
	}
};

}//namespace wl::gdi