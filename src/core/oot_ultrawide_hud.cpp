// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstring>
#include <utility>

#include "common/zstd_compression.h"
#include "core/oot_ultrawide_hud.h"

namespace Core::OotUltrawideHud {

namespace {

template <typename T>
std::optional<T> Read(std::span<const u8> data, size_t offset) {
    if (offset > data.size() || data.size() - offset < sizeof(T)) {
        return std::nullopt;
    }
    T value;
    std::memcpy(&value, data.data() + offset, sizeof(T));
    return value;
}

std::optional<std::string_view> CString(std::span<const u8> data, size_t offset) {
    if (offset >= data.size()) {
        return std::nullopt;
    }
    const char* begin = reinterpret_cast<const char*>(data.data()) + offset;
    const void* end = std::memchr(begin, 0, data.size() - offset);
    if (end == nullptr) {
        return std::nullopt;
    }
    return std::string_view{begin, static_cast<size_t>(static_cast<const char*>(end) - begin)};
}

struct SarcEntry {
    std::string_view name;
    size_t begin;
    size_t end;
};

// SARC: "SARC", u16 header size, u16 BOM, u32 file size, u32 data offset; SFAT at 0x14: "SFAT",
// u16 header size, u16 count, u32 hash key, then count {u32 hash, u32 attributes (0x01 << 24 | name
// offset / 4), u32 begin, u32 end}; SFNT: "SFNT", u16 header size, u16, then the names.
std::optional<std::vector<SarcEntry>> SarcEntries(std::span<const u8> sarc) {
    if (sarc.size() < 0x20 || std::memcmp(sarc.data(), "SARC", 4) != 0 ||
        std::memcmp(sarc.data() + 0x14, "SFAT", 4) != 0) {
        return std::nullopt;
    }
    const auto data_offset = Read<u32>(sarc, 0x0C);
    const auto count = Read<u16>(sarc, 0x1A);
    if (!data_offset || !count) {
        return std::nullopt;
    }
    const size_t names = 0x20 + 16 * size_t{*count} + 8;
    std::vector<SarcEntry> out;
    for (u16 i = 0; i < *count; ++i) {
        const size_t node = 0x20 + 16 * size_t{i};
        const auto attributes = Read<u32>(sarc, node + 4);
        const auto begin = Read<u32>(sarc, node + 8);
        const auto end = Read<u32>(sarc, node + 12);
        if (!attributes || !begin || !end || (*attributes >> 24) != 1 || *end < *begin ||
            size_t{*data_offset} + *end > sarc.size()) {
            return std::nullopt;
        }
        const auto name = CString(sarc, names + 4 * size_t{*attributes & 0xFFFFFF});
        if (!name) {
            return std::nullopt;
        }
        out.push_back({*name, size_t{*data_offset} + *begin, size_t{*data_offset} + *end});
    }
    return out;
}

// A "BFFH" layout file: u32 at 0x10 is the offset of its first "BFSI" section, whose data (u32 at
// +4) starts with "UIL"; there, +0x20 points to the pane name table (u64 offsets of C strings),
// +0x28 to the pane table (u64 offsets of 68-byte pane records) and +0x38 holds the pane count.
// A pane record has its translation x at +28 (y at +32), its parent index at +40.
std::optional<size_t> PaneX(std::span<const u8> layout, std::string_view pane) {
    if (layout.size() < 0x20 || std::memcmp(layout.data(), "BFFH", 4) != 0) {
        return std::nullopt;
    }
    const auto section = Read<u32>(layout, 0x10);
    const auto base = section ? Read<u32>(layout, size_t{*section} + 4) : std::nullopt;
    if (!base || *base + size_t{0x40} > layout.size() ||
        std::memcmp(layout.data() + *base, "UIL", 3) != 0) {
        return std::nullopt;
    }
    const auto names = Read<u64>(layout, *base + size_t{0x20});
    const auto records = Read<u64>(layout, *base + size_t{0x28});
    const auto count = Read<u32>(layout, *base + size_t{0x38});
    if (!names || !records || !count) {
        return std::nullopt;
    }
    for (u32 i = 0; i < *count; ++i) {
        const auto name_at = Read<u64>(layout, *names + 8 * size_t{i});
        const auto record = Read<u64>(layout, *records + 8 * size_t{i});
        if (!name_at || !record || *record + 68 > layout.size()) {
            return std::nullopt;
        }
        if (CString(layout, *name_at) == pane) {
            return *record + 28;
        }
    }
    return std::nullopt;
}

} // Anonymous namespace

std::optional<std::span<const u8>> SarcFile(std::span<const u8> sarc, std::string_view name) {
    const auto entries = SarcEntries(sarc);
    if (!entries) {
        return std::nullopt;
    }
    for (const auto& entry : *entries) {
        if (entry.name == name) {
            return sarc.subspan(entry.begin, entry.end - entry.begin);
        }
    }
    return std::nullopt;
}

bool ShiftPanes(std::vector<u8>& sarc, std::span<const PaneShift> shifts) {
    const auto entries = SarcEntries(sarc);
    if (!entries) {
        return false;
    }
    std::vector<std::pair<size_t, f32>> writes;
    for (const PaneShift& shift : shifts) {
        bool found = false;
        for (const auto& entry : *entries) {
            if (entry.name != shift.layout) {
                continue;
            }
            const std::span<const u8> layout{sarc.data() + entry.begin, entry.end - entry.begin};
            const auto x = PaneX(layout, shift.pane);
            if (!x || Read<f32>(layout, *x) != shift.original_x) {
                return false;
            }
            writes.emplace_back(entry.begin + *x, shift.new_x);
            found = true;
        }
        if (!found) {
            return false;
        }
    }
    for (const auto& [offset, value] : writes) {
        std::memcpy(sarc.data() + offset, &value, sizeof(value));
    }
    return true;
}

std::optional<std::vector<u8>> AnchorHudToEdges(std::span<const u8> compressed,
                                                std::span<const u8> dictionary) {
    auto archive = Common::Compression::TryDecompressDataZSTDWithDictionary(compressed, dictionary,
                                                                          256ULL << 20);
    if (!archive || !ShiftPanes(*archive, HudEdgeShifts)) {
        return std::nullopt;
    }
    auto out = Common::Compression::CompressDataZSTDWithDictionary(*archive, dictionary);
    if (out.empty()) {
        return std::nullopt;
    }
    return out;
}

} // namespace Core::OotUltrawideHud
