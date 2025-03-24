# Compiler and flags
CC = cl                           # The Microsoft C/C++ compiler
CFLAGS = /nologo /EHsc /Zi /DUNICODE /D_UNICODE /I nlohmann  # Compiler flags:
                                                 # - /nologo = no splash banner
                                                 # - /EHsc = enable C++ exceptions
                                                 # - /Zi = include debug info
                                                 # - /DUNICODE /D_UNICODE = define UNICODE macros for wide strings

LIBS = user32.lib gdi32.lib gdiplus.lib          # Libraries we’re linking against
LFLAGS = /link $(LIBS)                           # Linker flags (tacked on after source)

# Source and output
SRC = main.cpp sprite.cpp          # Your source files
OUT = main.exe                     # Final executable name

# Default target (what runs when you type just `nmake`)
all: $(OUT)

# How to build the .exe from .cpp
$(OUT): $(SRC)
	$(CC) $(CFLAGS) /Fe$(OUT) $(SRC) $(LFLAGS) 
# /Fe tells MSVC what to name the executable outpu


# Clean up everything that gets generated
clean:
	del /Q *.exe *.obj *.ilk *.pdb
