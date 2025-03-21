# Goal
Create an interactive sprite moving around on the desktop without a visible window.
It should run on Windows in a transparent, click-through overlay window.

- Uses the `WinAPI` for window creation and event handling
- Draws a sprite using `GDI+` (or `Direct2D`)


# Setup
- Start VScode from the `Developer Command Prompt for VS 2022` using `code .` to set up the emvironment


# Headers and Libraries
- User32.lib:	Handles window creation, input, messages, etc.
- Gdi32.lib:	Handles basic 2D drawing (lines, rectangles, etc.)
- Gdiplus.lib:	Adds more advanced graphics (sprites, anti-aliasing, images)
- windows.h:	Includes most core WinAPI functions
- gdiplus.h:	GDI+ drawing (image rendering, smoother shapes)


# Build and run
Press `Ctrl + Shift + B` to build, or run `nmake` in the terminal. 

Generated files:
- main.exe:     The program — the compiled Windows executable.
- main.ilk:     Intermediate linker file used for incremental linking — helps speed up rebuilds.
- main.pdb:     Program Database — stores debugging symbols like variable names, line numbers, etc.


Run the executable from the terminal using `.\main.exe`