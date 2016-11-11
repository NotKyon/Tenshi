# Tenshi (Basic Compiler)
This is a modified version of the
[Dark Basic](https://github.com/LeeBamberTGC/Dark-Basic-Pro) language. Features
have been added, some syntax has been changed, but the spirit behind it is the
same.

This is a clean implementation; no code from the original repository has been
used.


## What does the language look like?
Let's jump right in with a "Hello, world!" sample.

`hello.te`:
```
// Let's greet the world.
"Hello, world!"
```

That looks awfully bare. This is simply a string literal, just text, how does
that do anything? The compiler treats string literals and string expressions as
something called "autoprint statements." No explicit function invocation is
necessary. This can be useful for dialogue heavy programs, such as an RPG or
visual novel.

Let's try something else. How about listing the files in a directory?

`list-files.te`:
```
"Files in current directory:"
find first
repeat
        "  " + getFileTypeName$() + " <" + get filename$() + "> - " + get file date$() + " " + get file time$()
until not find next()

fn getfiletypename$()
        local n = get file type()
        if n = 0 then return "[Reg]"
        if n = 1 then return "[Dir]"
endfn "[???]"
```

Well, that's a bit further expanded from the previous code listing. Important
features to note:

- Functions don't need to be declared prior to their uses, however the compiler
  still validates their uses. (The function needs to be defined in the file, but
  that's it.)
- The language is case insensitive. `getfiletypename$()` and
  `getFileTypeName$()` refer to the same function.
- Spaces can exist in key commands, allowing for a bit more readability.
- This is very obviously a variant of BASIC. (Syntax derived from Dark Basic.)

There's probably a lot more to say, but this is an old project and I don't
remember very much. If this interests you, why not look through the code and
through the various documentation directories?


## AxTech Libraries (AxLibs)
These contain various independent libraries for different tasks. They are used
internally by Tenshi. See `Code/Libs/`.

- Platform: Platform specific header-only definitions.
- Core: Manages basic portable types (uint32, intptr, ...), the String class,
        asserts, etc
- Collections: Holds collection types (TList, TIntrusiveList, TArray,
               TDictionary)
- Allocation: Memory management system, used throughout.
- Async: Parallel tasking library. Implements an efficient task scheduler for
         games.
- Parser: Generic lexing/parsing system.
- System: File system functionality, UUID generation, and high-resolution
          timing.


## Building

How to build:

- Install the 64-bit version of [msys2](http://msys2.github.io/).
- In msys2, run `pacman -S cmake` then `pacman -S mingw-w64-x86_64-llvm`.
- Use CMake to generate a makefile or ninja build script, then issue
  `cmake --build`.

That should be it!

It should be possible to build everything with Visual Studio as well, however
that is not tested.
