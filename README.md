# fcitx-skk

fcitx-skk is an input method engine for Fcitx, which uses libskk as its backend.

Requirements:
 - libskk(libskk-dev)
 - Qt4 (optional), for rule and dictionary configuration UI.(libqt4-dev)
 - fcitx 4.2.8(fcitx-libs-dev)
 - skk-jisyo(skkdic)


Build dependency:
 - cmake
 - C++ Compiler(g++)

You can specify the skk dictionary path by -DSKK_DEFAULT_PATH=path_you_want

By default it's /usr/share/skk/SKK-JISYO.L

Installation
------------------

    git clone https://github.com/fcitx/fcitx-skk.git
    cd fcitx-skk
    cmake .
    sudo make install
