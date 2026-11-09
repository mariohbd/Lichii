// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdlib>

namespace VideoCommon::RsManVram {

/// Switches of the RsMan-Dev VRAM series (zortos293/Lichii#267). EDEN_NX2_RSMAN_VRAM=0 turns all
/// of them off; each part also has its own EDEN_NX2_RSMAN_VRAM_<PART>=0. All on by default.
/// Read once by each caller into a static: getenv scans the whole environment block on Windows.
[[nodiscard]] inline bool Enabled(const char* part) {
    static const bool master = [] {
        const char* v = std::getenv("EDEN_NX2_RSMAN_VRAM");
        return v == nullptr || v[0] != '0';
    }();
    if (!master) {
        return false;
    }
    const char* v = std::getenv(part);
    return v == nullptr || v[0] != '0';
}

} // namespace VideoCommon::RsManVram
