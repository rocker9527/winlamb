# WinLamb

A lightweight modern C++17 library for [Win32 API](https://en.wikipedia.org/wiki/Windows_API), using [lambda closures](https://www.cprogramming.com/c++11/c++11-lambda-closures.html) to handle window messages.

**Note:** If you're looking for the old C++11, version, [see this branch](https://github.com/rodrigocfd/winlamb/tree/cpp11).

## Usage

WinLamb is a header-only library. You can clone the repository or simply [download](https://github.com/rodrigocfd/winlamb/archive/master.zip) the files. Once referenced in your source code, it should work right away.

There's an included `win10.exe.manifest` file, which includes Common Controls and [Windows 10 support](https://docs.microsoft.com/pt-br/windows/desktop/SysInfo/targeting-your-application-at-windows-8-1).

For detailed information, [check out the documentation](https://rodrigocfd.github.io/winlamb).

## Example

This is a simple Win32 program written with WinLamb. Each window has a class, and messages are handled with lambda closures. There's no need to write a message loop or window registering.

* Declaration file: `My_Window.h`

````cpp
#include "winlamb/window_main.h"

class My_Window : public wl::window_main {
public:
    My_Window();
};
````

* Implementation file: `My_Window.cpp`

````cpp
#include "My_Window.h"

RUN(My_Window) // optional, generate WinMain call and instantiate MyWindow

MyWindow::My_Window()
{
    setup().title = L"Hello world";
    setup().style |= WS_MINIMIZEBOX;

    on_msg().wm_create([this]() -> int
    {
        set_title(L"A new title for my window");
        return 0;
    });

    on_msg().wm_l_button_down([](wl::msg::wm_l_button_down p)
    {
        bool isCtrlDown = p.has_ctrl();
        long xPos = p.pos().x;
    });
}
````

Note that Win32 uses [Unicode strings](https://docs.microsoft.com/en-us/windows/win32/learnwin32/working-with-strings) natively, that means to use `wchar_t`, `std::wstring` and `std::wstring_view`.

## License

Licensed under [MIT license](https://opensource.org/licenses/MIT), see [LICENSE.txt](LICENSE.txt) for details.
