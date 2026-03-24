/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  SPDX-FileCopyrightText: 2005 Takuro Ashie
 *  SPDX-FileCopyrightText: 2012 CSSlayer <wengxt@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "utils.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fcitx-utils/utf8.h>
#include <limits>
#include <string>
#include <sys/types.h>

std::string util::utf8_string_substr(const std::string &s, size_t start,
                                     size_t len) {
    auto iter = fcitx::utf8::nextNChar(s.begin(), start);
    auto end = fcitx::utf8::nextNChar(iter, len);
    std::string result(iter, end);
    return result;
}

bool util::surrounding_get_safe_delta(uint from, uint to, int32_t *delta) {
    const int64_t kInt32AbsMax =
        std::llabs(static_cast<int64_t>(std::numeric_limits<int32_t>::max()));
    const int64_t kInt32AbsMin =
        std::llabs(static_cast<int64_t>(std::numeric_limits<int32_t>::min()));
    const int64_t kInt32SafeAbsMax = std::min(kInt32AbsMax, kInt32AbsMin);

    const int64_t diff = static_cast<int64_t>(from) - static_cast<int64_t>(to);
    if (std::llabs(diff) > kInt32SafeAbsMax) {
        return false;
    }

    *delta = static_cast<int32_t>(diff);
    return true;
}

// Returns true if |surrounding_text| contains |selected_text|
// from |cursor_pos| to |*anchor_pos|.
// Otherwise returns false.
static bool search_anchor_pos_forward(const std::string &surrounding_text,
                                      const std::string &selected_text,
                                      size_t selected_chars_len,
                                      uint cursor_pos, uint *anchor_pos) {

    size_t len = fcitx::utf8::length(surrounding_text);
    if (len < cursor_pos) {
        return false;
    }

    size_t offset =
        fcitx::utf8::ncharByteLength(surrounding_text.begin(), cursor_pos);

    if (surrounding_text.compare(offset, selected_text.size(), selected_text) !=
        0) {
        return false;
    }
    *anchor_pos = cursor_pos + selected_chars_len;
    return true;
}

// Returns true if |surrounding_text| contains |selected_text|
// from |*anchor_pos| to |cursor_pos|.
// Otherwise returns false.
bool search_anchor_pos_backward(const std::string &surrounding_text,
                                const std::string &selected_text,
                                size_t selected_chars_len, uint cursor_pos,
                                uint *anchor_pos) {
    if (cursor_pos < selected_chars_len) {
        return false;
    }

    // Skip |iter| to (potential) anchor pos.
    const uint skip_count = cursor_pos - selected_chars_len;
    if (skip_count > cursor_pos) {
        return false;
    }
    size_t offset =
        fcitx::utf8::ncharByteLength(surrounding_text.begin(), skip_count);

    if (surrounding_text.compare(offset, selected_text.size(), selected_text) !=
        0) {
        return false;
    }
    *anchor_pos = skip_count;
    return true;
}

bool util::surrounding_get_anchor_pos_from_selection(
    const std::string &surrounding_text, const std::string &selected_text,
    uint cursor_pos, uint *anchor_pos) {
    if (surrounding_text.empty()) {
        return false;
    }

    if (selected_text.empty()) {
        return false;
    }

    const size_t selected_chars_len = fcitx::utf8::length(selected_text);

    if (search_anchor_pos_forward(surrounding_text, selected_text,
                                  selected_chars_len, cursor_pos, anchor_pos)) {
        return true;
    }

    return search_anchor_pos_backward(surrounding_text, selected_text,
                                      selected_chars_len, cursor_pos,
                                      anchor_pos);
}
