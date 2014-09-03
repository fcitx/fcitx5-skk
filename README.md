# fcitx-skk

fcitx-skk is an input method engine for Fcitx, which uses libskk as its backend.

## Requirements:

 - libskk
 - Qt4 (optional), for rule and dictionary configuration UI.
 - fcitx 4.2.8
 - skk-jisyo

### For Ubuntu User

Please install this packages before build this Program.

 - libskk-dev
 - libqt4-dev
 - fcitx-libs-dev
 - skkdic

    $ sudo aptitude install libskk-dev libqt4-dev fcitx-libs-dev skkdic


## Build dependency:

 - cmake
 - C++ Compiler(g++)

You can specify the skk dictionary path by -DSKK_DEFAULT_PATH=path_you_want

By default it's /usr/share/skk/SKK-JISYO.L

## Installation 

    git clone https://github.com/fcitx/fcitx-skk.git
    cd fcitx-skk
    cmake .
    sudo make install
