/*
 * SPDX-FileCopyrightText: 2005 Takuro Ashie
 * SPDX-FileCopyrightText: 2017-2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_SKK_UTILS_H_
#define _FCITX5_SKK_UTILS_H_

#include <cstddef>
#include <cstdint>
#include <fcitx-utils/charutils.h>
#include <fcitx-utils/key.h>
#include <fcitx/event.h>
#include <string>
#include <string_view>

namespace util {

std::string utf8_string_substr(const std::string &s, size_t start, size_t len);

bool match_key_event(const fcitx::KeyList &list, const fcitx::Key &key,
                     fcitx::KeyStates ignore_mask = fcitx::KeyStates());
std::string convert_to_wide(std::string_view str);
std::string convert_to_half(const std::string &str);
std::string convert_to_katakana(const std::string &hira, bool half = false);

bool key_is_keypad(const fcitx::Key &key);
std::string keypad_to_string(const fcitx::KeyEvent &key);
void launch_program(std::string_view command);

bool surrounding_get_safe_delta(unsigned int from, unsigned int to,
                                int32_t *delta);

bool surrounding_get_anchor_pos_from_selection(
    const std::string &surrounding_text, const std::string &selected_text,
    unsigned int cursor_pos, unsigned int *anchor_pos);

inline char get_ascii_code(const fcitx::Key &key) {
    auto chr = fcitx::Key::keySymToUnicode(key.sym());
    if (fcitx::charutils::isprint(chr)) {
        return chr;
    }
    return 0;
}

inline char get_ascii_code(const fcitx::KeyEvent &event) {
    return get_ascii_code(event.rawKey());
}

const fcitx::KeyList &selection_keys();
} // namespace util

#endif // _FCITX5_SKK_UTILS_H_
