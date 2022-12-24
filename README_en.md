# GenshinHdiffPatcher

[中文](README.md) | [English](README_en.md)

> Simple, modern and fast hdiff pack patch tool of certain anime game.

<div style="text-align: center">[ Easy4Use · UltraFast · Detailed · LightWeight ]</div>

## Quick start

You can get executable files from the following places or compile it yourself.

### Prebuilt version

- [Release](https://github.com/Wansn-w/GenshinHDiffPatcher/releases) \[recommend\]
- [Actions](https://github.com/Wansn-w/GenshinHDiffPatcher/actions)

### Compile

#### Automatic compilation

You only need to run the batch file to build automatically.
For Windows, you can use `build.bat`; for Unix, use `build.sh`. **Todo**

#### Manual

1. Install the following dependencies

    - Cmake
    - Ninja (CMake generator, **Optional**)
    - gcc & g++ (They are usually pre-installed in unix, for Windows, you can use [MingW](https://www.mingw-w64.org/))

2. Build

    If you do not have the above Ninja installed, please ignore the `-GNinja` option.

    ```shell
    cmake -GNinja -DCMAKE_BUILD_TYPE=Release -Bbuild
    cmake --build build -j6
    ```

   For machines with a small number of cores, you may need to reduce the number after the `-j` parameter appropriately.

3. enjoy
   The final executable file is located in the project-folder/build directory.

   
### Usage

You can enter the game path in the first parameter, such as "N:\\ Genshin Impact\", and then enter the path of the game differential package in the later parameters, such as "K:\\ game_3.2.0_3.3.0_hdiff_fiuTl2cLqp198WVG.zip".
The differential package path can be either a zip file or an unzipped path.

```
Automatically update certain anime game's hdiff package command line
Usage:
  GenshinPatcher [OPTION...] <gameDir> <diffFiles> [diffFiles2] [diffFiles3]...

  -s, --safe-patch  Run the patch safely.
  -h, --help        Show this msg
```

### Credit

- [ChatGPT](https://chat.openai.com) - For batch scripting and other help.
- [HDiffPatch](https://github.com/sisong/HDiffPatch) - The difference method used in Genshin games.
