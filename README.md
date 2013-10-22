# fcitx-skk

fcitx-skk is an input method engine for Fcitx, which uses libskk as its backend.

Requirements:
 - libskk
 - Qt4 (optional), for rule and dictionary configuration UI.
 - fcitx 4.2.8
 - skk-jisyo

Build dependency:
 - cmake

You can specify the skk dictionary path by -DSKK_DEFAULT_PATH=path_you_want

By default it's /usr/share/skk/SKK-JISYO.L
