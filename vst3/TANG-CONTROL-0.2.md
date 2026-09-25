# Tang Control 0.2 — DAW test candidate

## What's changed

- 55 sound/routing controls, including all eleven previously missing firmware parameters. Existing parameter IDs remain available.
- Each routing, bypass, mute and solo bit has an independent boolean host parameter. Old packed masks remain for old-session migration, but are not automatable. Delay 2 topology has a three-state choice (Off / Parallel / Series).
- SEND SESSION applies the complete sound/routing snapshot and reads every value back. A mismatch disconnects with an error instead of reporting success. An interrupted patch can be partially applied: use READ TANG to inspect before resending. No claim of atomic hardware writes or rollback.
- READ TANG and DISCONNECT invalidate the remaining batch. An already transmitted request is drained; it cannot be undone.
- FOLLOW HARDWARE polls real controls at roughly 5 Hz and imports them. It disables UI writes; DAW automation and hardware-follow are separate authorities. Switch it off to let the DAW control parameters. SEND SESSION switches it off explicitly.
- FOLLOW DAW PLAY optionally follows host Play/Stop. START/STOP remain available. Offline host rendering pauses automatic control dispatch; render the external insert in real time.
- Delay tempo choices Free / 1/64 / 1/32 / 1/16 use host BPM. They respect 8192/4096 frames at 48 kHz. Invalid or absent BPM, or a division exceeding RAM, leaves the last time unchanged and shows a warning. Manual time sliders are disabled while tempo sync is selected. This is control-rate tempo following, not sample-accurate clock/phase synchronization.
- Input and final expanded output peaks come from FPGA snapshots. They refresh on readback/hardware-follow, not from the native audio engine.

## First DAW test

1. Install the **0.2** VST3/AU and rescan. Close the web serial connection. One plugin instance owns each Tang.
2. Open TANG CONTROL, enter the serial port, CONNECT, then READ TANG. Connecting does not overwrite the patch or start audio.
3. Turn on CONTROLLER ONLY to preserve the DAW samples. Audio is still **interface output → PCM1808 → FPGA → PCM5102 → interface line input**. USB transports control only. Monitor the PCM5102 directly if no recording input is available.
4. Change one audible parameter with its branch enabled and mix above zero. The status counts verified batches. Compare hardware screen and plugin after READ TANG.
5. Enable FOLLOW HARDWARE and turn an encoder. Values should follow. Disable FOLLOW HARDWARE before recording DAW automation; automate the individual named switches rather than legacy masks.
6. Save/reopen the session, reconnect and use SEND SESSION explicitly. Check verification count and the actual hardware. The sixteen encoder assignments are not recalled: the current protocol exposes only the active bank, so reliable two-bank recall needs firmware support.
7. Try FOLLOW DAW PLAY and short delay subdivisions at 120 BPM: A 1/16 fits, B 1/32 fits, B 1/16 reports out of range. Hardware must be rendered in real time; calibrate round-trip latency using the DAW's external insert.

## Validation

ControllerCheck tests the production threaded link through a fake serial port: split packets and CRC, full apply/readback, disconnect/read cancellation, mismatched readback, corrupt replies, passive hardware-follow, mode conversion, tempo bounds and old/new state migration. host_test.py checks actual VST3 audio and parameter recall. Physical hardware and manual DAW results must be reported separately. No firmware/clocks changed.

This guide describes a test candidate, not analog certification or a notarized commercial Mac release.
