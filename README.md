# Tenshi (Basic Compiler)
Implements a modified version of the Dark Basic language, while altering some of
the more obscure features.

Most of the code here is old and uses frameworks that are known to contain bugs.
This project is no longer a priority, though I may still work on it from time to
time.


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

function getfiletypename$()
        local n = get file type()
        if n = 0 then exitfunction "[Reg]"
        if n = 1 then exitfunction "[Dir]"
endfunction "[???]"
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
- Parser: Generic lexing/parsing system. This is still being ported over from
          the prior version.
- System: File system functionality, UUID generation, and high-resolution
          timing.

etc.


## Building

How to build:

- Grab Visual Studio 2015. (Other versions might work, but are untested.)
- Run Setup.bat in the root directory.
- Build LLVM (See ThirdParty/LLVM/ReadMe.txt)
- Open Tenshi.sln and select "Build Solution."

That's it. (Probably.)

Building with GCC or Clang presently is unlikely to work, however an
installation of msys will be needed for some parts of the build steps. Look at
some of the build batch files to figure this out.
