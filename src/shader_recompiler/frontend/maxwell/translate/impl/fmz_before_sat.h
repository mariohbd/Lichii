// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdlib>

namespace Shader::Maxwell {

/// FFMA and HFMA2 with .FMZ and .SAT: on (default) the FMZ select runs before the saturation, so
/// 0 * Inf + c gives sat(c) like the hardware; off, FMZ is skipped under SAT as before and the
/// NaN saturates to 0 (zortos293/Lichii#811, the blown-out white lights on Gerudo Valley's
/// walls). EDEN_NX2_FMZ_BEFORE_SAT=0 turns it off; the kept pipeline stamp includes it.
inline bool FmzBeforeSat() {
    static const bool enabled = [] {
        const char* const value = std::getenv("EDEN_NX2_FMZ_BEFORE_SAT");
        return value == nullptr || value[0] != '0';
    }();
    return enabled;
}

} // namespace Shader::Maxwell
