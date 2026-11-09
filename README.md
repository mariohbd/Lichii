# Fritchii v5.5 + frame generation (unofficial)

This branch is [Fritchii](https://github.com/Frizti/Fritchii) v5.5 with three changes, for
Zelda: Ocarina of Time on PC. It is not affiliated with Fritchii, Lichii or Nintendo.

| Change | Switch in `nx2_switches.txt` |
| --- | --- |
| Frame generation (FSR3 / LSFG) runs again, as in v5.3 (v5.5 turned it off in every Fritchii build) | `EDEN_NX2_FRAMEGEN=0` keeps it off |
| Its options (type, multiplier, target frame rate...) are shown again in Graphics | |
| With Ivan0xFF's 3440x1440 [ultrawide mod](https://github.com/craftycorvid/oot-mod), the HUD moves to the 21:9 screen edges ([Lichii #831](https://github.com/zortos293/Lichii/pull/831)) | `EDEN_NX2_OOT_HUD_EDGES=0` keeps it centred |

Fritchii v5.5 turned frame generation off because an old `frame_gen=true` made the camera flicker
for some players. If you see that, untick it in Graphics or set `EDEN_NX2_FRAMEGEN=0`.

The Fritchii README follows.

---

