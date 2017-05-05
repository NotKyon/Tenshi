# Tenshi (Basic Compiler)
Tenshi is a Basic-like language inspired by
[Dark Basic](https://github.com/LeeBamberTGC/Dark-Basic-Pro). Features have been
added, some syntax has been changed, but the spirit behind it is generally
the same.


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


## Building

How to build *on Windows*:

- Install the 64-bit version of [msys2](http://msys2.github.io/).
- In msys2, run `pacman -S cmake` then `pacman -S mingw-w64-x86_64-llvm`.
- Use CMake to generate a makefile or ninja build script, then issue
  `cmake --build`.

How to build *on macOS*:

- Install [Mac Ports](https://www.macports.org/install.php).
- In the Terminal, run `sudo port install cmake llvm-3.9 ninja`
- Create a build directory, next to your clone of this repository and cd into
  it. e.g., `mkdir build-Tenshi && cd build-Tenshi`
- Use CMake to generate a ninja build script, but
  **also specify the LLVM path**: `LLVM_DIR=/opt/local/libexec/llvm-3.9 cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja ../Tenshi`
- If all went well, then build by issuing the command `cmake --build`

That should be it!

It should be possible to build everything with Visual Studio as well, however
that is not tested.
