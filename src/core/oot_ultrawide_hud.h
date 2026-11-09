// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "common/common_types.h"

// The Legend of Zelda: Ocarina of Time with Ivan0xFF's 21:9 (3440x1440) ultrawide mod: the HUD's
// edge anchors moved to the screen edges (from Lichii #831, e0053016c2).
namespace Core::OotUltrawideHud {

constexpr u64 ProgramId = 0x0400486024DEC000ULL;

/// The UI layouts (SARC of "BFFH" layout files, zstd against the dictionary "zs.zsdic" in the SARC
/// Pack/ZsDic.pack.zs).
constexpr std::string_view LayoutArchivePath =
    "UI/Layout2Archive/Common.Product.100.Nin_Ounce_NVN2.blarc.zs";
constexpr std::string_view DictionaryPackPath = "Pack/ZsDic.pack.zs";
constexpr std::string_view LayoutDictionaryName = "zs.zsdic";

/// A pane of a UI layout moved in x (layout units: the 3840x2160 grid, origin in the middle).
struct PaneShift {
    std::string_view layout;
    std::string_view pane;
    f32 original_x;
    f32 new_x;
};

/// PlayerStatus_00's N_ScaleL_00 (hearts, magic) and N_ScaleR_00 (item buttons) sit at x = -1920 /
/// +1920, the 16:9 grid's edges. The mod's UI canvas squeeze (16/9 over 43/18) puts the grid into
/// the centred 16:9; the 21:9 screen's edges are at +-1920 / squeeze = +-2580.
constexpr f32 GridHalfWidth = 1920.0f;
constexpr f32 EdgeShift = 660.0f; // 1920 * (43/18) / (16/9) - 1920
inline constexpr PaneShift HudEdgeShifts[]{
    {"PlayerStatus_00.buil", "N_ScaleL_00", -GridHalfWidth, -GridHalfWidth - EdgeShift},
    {"PlayerStatus_00.buil", "N_ScaleR_00", GridHalfWidth, GridHalfWidth + EdgeShift},
};

[[nodiscard]] std::optional<std::span<const u8>> SarcFile(std::span<const u8> sarc,
                                                          std::string_view name);

/// Moves the panes in every copy of their layout, in place; false (sarc unchanged) when one is not
/// found with its original x.
[[nodiscard]] bool ShiftPanes(std::vector<u8>& sarc, std::span<const PaneShift> shifts);

/// The layout archive with the HUD's edge anchors moved out, or nothing when it is not the archive
/// this expects.
[[nodiscard]] std::optional<std::vector<u8>> AnchorHudToEdges(std::span<const u8> compressed,
                                                              std::span<const u8> dictionary);

} // namespace Core::OotUltrawideHud
