# Build Guide — CompilerDesignV2

## Prerequisites

Install **MSYS2** (https://www.msys2.org) and then install the required packages from the **UCRT64** environment:

```bash
# Run inside the MSYS2 UCRT64 shell
pacman -S mingw-w64-ucrt-x86_64-gcc
pacman -S mingw-w64-ucrt-x86_64-spdlog
pacman -S mingw-w64-ucrt-x86_64-fmt
```

The expected paths are:

- Compiler: `C:\msys64\ucrt64\bin\g++.exe`
- spdlog CMake config: `C:\msys64\ucrt64\lib\cmake\spdlog`
- fmt CMake config: `C:\msys64\ucrt64\lib\cmake\fmt`

---

## Method 1 — Direct g++ (simplest)

Run from the repository root (`C:\Projects\CompilerDesignV2`):

```powershell
& 'C:\msys64\ucrt64\bin\g++.exe' -Wall -g driver.cpp lexor.cpp parser.cpp first_and_follow.cpp handler.cpp -o driver.exe -lspdlog -lfmt
```

The output binary `driver.exe` is placed in the working directory.

---

## Method 2 — CMake

### Important: avoid vcpkg ABI mismatch

vcpkg's default `x64-windows` triplet builds MSVC-ABI libraries. These are **incompatible** with the MinGW linker. Always pass `-DCMAKE_TOOLCHAIN_FILE=` (empty) to prevent vcpkg from being auto-injected, and point CMake at the MSYS2 libraries instead.

### Configure (first time, or after deleting `build/`)

```powershell
cmake -B build -S . `
  -G "MinGW Makefiles" `
  -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/gcc.exe `
  -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe `
  -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64 `
  -DCMAKE_TOOLCHAIN_FILE=
```

Expected output ends with:

```
-- Configuring done
-- Generating done
-- Build files have been written to: .../build
```

### Build

```powershell
cmake --build build
```

Expected output:

```
[100%] Built target driver
```

The binary is at `build\driver.exe`.

### Rebuild after source changes

```powershell
cmake --build build
```

CMake detects changed files automatically; no need to reconfigure.

### Clean rebuild

Delete the `build/` directory and repeat the configure step above.

---

## Running the Program

Run from the repository root so that relative paths resolve correctly:

```powershell
.\driver.exe           # Method 1 output
# or
.\build\driver.exe     # Method 2 output
```

The program prompts interactively for three input files in sequence:

| Prompt                 | File to provide                     |
| ---------------------- | ----------------------------------- |
| Script / source file   | `inputs\bubblesort.src`             |
| Grammar file           | `inputs\a_good_grammar_grammar.grm` |
| Attribute grammar file | `inputs\AttributeGrammar.txt`       |

The driver runs the **lexer** over the script, then runs **first/follow set generation**, **parsing table construction**, **parsing**, and **symbol table construction**, logging progress to the console.

### Output files generated

| File              | Description                                |
| ----------------- | ------------------------------------------ |
| `firstSet`        | Computed FIRST sets for each non-terminal  |
| `followSet`       | Computed FOLLOW sets for each non-terminal |
| `output.txt`      | Parser output / derivation                 |
| `symbolTable.txt` | Symbol table from semantic analysis        |
