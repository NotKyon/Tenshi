[LLVM]
LLVM is used by Tenshi. This directory is only required for Visual Studio when
building Tenshi.

Currently, you must build LLVM separately and copy its files into the LLVM
directory here. (<Root>/ThirdParty/LLVM/)

Eventually a shell script will be used to checkout LLVM from its Git repo, build
it, and copy the right files to this location.

The directory layout is:

	=target=                        =source=
	LLVM/Bin/x64/Debug/             <LLVM-Build-Dir>/Debug/bin/
	LLVM/Bin/x64/Release/           <LLVM-Build-Dir>/Release/bin/
	LLVM/Lib/x64/Debug/             <LLVM-Build-Dir>/Debug/lib/
        LLVM/Lib/x64/Release/           <LLVM-Build-Dir>/Release/lib/
        LLVM/Include/                   <LLVM-Source-Dir>/include/

The items from "=source=" should be copied into the corresponding "=target="
directory.

Additionally, LLVM/Include/ will need:

	<LLVM-Build-Dir>/include/llvm/Config/
	<LLVM-Build-Dir>/include/llvm/Support/


[GNU]
Various GNU and GNU-related projects are used by Tenshi. This includes:

- GNU Binutils (LD, AR)
- Various MinGW libraries (on Windows only)

All binaries and libraries are expected to be 64-bit.
For best results on Windows, grab from here: http://nuwen.net/mingw.html
(Nuwen's distro is x86-64 native.)

Directory layout:

	GNU/bin/
	Hosts binary files.

	GNU/lib/
	Hosts libraries needed by programs built with Tenshi.

Needed files:

	GNU/bin/ld.exe

	GNU/lib/crt2.o
	GNU/lib/crtbegin.o
	GNU/lib/crtend.o

	GNU/lib/libgcc.a
	GNU/lib/libkernel32.a
	GNU/lib/libmingw32.a
	GNU/lib/libmingwex.a
	GNU/lib/libmsvcrt.a

If you're using Nuwen's MinGW distribution, the files are located here:

	MinGW/x86_64-w64-mingw32/bin/ld.exe

	MinGW/x86_64-w64-mingw32/lib/crt2.o
	MinGW/x86_64-w64-mingw32/lib/crtbegin.o
	MinGW/x86_64-w64-mingw32/lib/crtend.o

	MinGW/x86_64-w64-mingw32/lib/libkernel32.a
	MinGW/x86_64-w64-mingw32/lib/libmingw32.a
	MinGW/x86_64-w64-mingw32/lib/libmingwex.a
	MinGW/x86_64-w64-mingw32/lib/libmsvcrt.a

	MinGW/lib/gcc/x86_64-w64-mingw32/<GCC-Version>/libgcc.a

At this time of writing, "<GCC-Version>" is 4.9.2.
