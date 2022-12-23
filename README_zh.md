# GenshinHdiffPatcher

[中文](README_zh.md) | [English](README.md)

> 简单、现代化、快速的原神更新包合并工具。

<div style="text-align: center">[ 使用简单 · 疾速 · 过程详细 · 轻量 ]</div>

## 快速开始

你可以从下方直接下载可执行文件，或者自己编译。

### 预购建版本

- [Release](/releases) \[recommend\]
- [Actions](/actions)

### 编译

#### 自动

只需要执行批处理文件即可完成自动编译操作。Windows用`build.bat`，Unix用`build.sh`

#### 手动

1. 安装如下依赖

    - Cmake
    - Ninja (CMake生成器, **可选**)
    - gcc & g++ (他们通常预装于unix系统中。对于Windows，可以使用 [MingW](https://www.mingw-w64.org/))

2. 构建

   如果你没有安装上面的Ninja生成器，请忽略 `-GNinja` 参数。

    ```shell
    cmake -GNinja -DCMAKE_BUILD_TYPE=Release -Bbuild
    cmake --build build -j6
    ```

   对于CPU核心数量较少的机器，你可能需要降低`-j` 参数后面的数字大小。

3. 完成
   最终生成的可执行文件位于 项目文件夹/build 目录中。


### 用法

你可以在第一个参数中填入游戏路径,例如"N:\\Genshin Impact\\",然后在之后的参数中填入游戏差分包的路径,如"K:\\game_3.2.0_3.3.0_hdiff_fiuTl2cLqp198WVG.zip"。 差分包路径既可以是一个zip文件,也可以是解压后的路径.

```
Automatically update certain anime game's hdiff package command line
Usage:
  GenshinPatcher [OPTION...] <gameDir> <diffFiles> [diffFiles2] [diffFiles3]...

  -s, --safe-patch  安全地进行修补。你仍需要手动替换文件。
  -h, --help        展示这个消息。
```

### Credit

- [ChatGPT](https://chat.openai.com) - For batch scripting and other help.
- [HDiffPatch](https://github.com/sisong/HDiffPatch) - The difference method used in Genshin games.




