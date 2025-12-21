# fcitx5-skk

fcitx5-skk is an input method engine for Fcitx5, which uses libskk as its backend.

## Requirements:

 - libskk
 - Qt5 (optional), for rule and dictionary configuration UI.
 - fcitx 5
 - fcitx-qt 5
 - skk-jisyo

### For Ubuntu User

Please install this packages before build this Program.

 - libskk-dev
 - libfcitx5-qt-dev
 - libfcitx5core-dev
 - qtbase5-dev
 - skkdic

    $ sudo apt install libskk-dev libfcitx5-qt-dev libfcitx5core-dev qtbase5-dev skkdic


## Build dependency:

 - cmake
 - C++ Compiler(g++)
 - extra-cmake-modules
 - gettext

You can specify the skk dictionary path by -DSKK_PATH=path_you_want

By default it's /usr/share/skk/

## Installation 

    git clone https://github.com/fcitx/fcitx5-skk.git
    cd fcitx5-skk
    cmake .
    sudo make install
