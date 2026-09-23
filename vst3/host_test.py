"""Real VST3 hosting checks. Usage: python host_test.py <bundle.vst3>."""
import json
import sys
import time
from pathlib import Path
import numpy as np
from pedalboard import load_plugin

path=Path(sys.argv[1]).resolve()
# Pedalboard's Windows scanner loads the module inside a VST3 bundle.
if sys.platform == 'win32' and path.is_dir(): path=path / 'Contents' / 'x86_64-win' / path.name
plugin = load_plugin(str(path))
print(plugin.name, plugin.version, flush=True)
print('Parameters:', list(plugin.parameters), flush=True)
assert plugin.is_effect and not plugin.is_instrument

def put(**values):
    for key, value in values.items():
        setattr(plugin, key, value)

results = []
for sr in (44100, 48000, 96000):
    t = np.arange(sr, dtype=np.float32) / sr
    signal = np.stack((.24*np.sin(2*np.pi*220*t)+.1*np.sin(2*np.pi*1700*t),
                       .2*np.sin(2*np.pi*330*t)+.07*np.sin(2*np.pi*3200*t)))
    put(master_mix=0., output=0., bypass=False)
    dry = plugin(signal, sr, buffer_size=257)
    print('Dry max error:',float(np.max(np.abs(signal-dry))), 'Output dB:',float(plugin.output), flush=True)
    assert np.array_equal(signal, dry), 'Dry signal is not sample-exact'
    put(master_mix=.7, feedback=.6, space=.5, wavefolder=.3, motion=.4)
    wet = plugin(signal, sr, buffer_size=257)
    assert np.isfinite(wet).all() and np.max(np.abs(wet)) <= 1.001
    rms_delta = float(np.sqrt(np.mean((wet-signal)**2)))
    assert rms_delta > .01, 'Wet processing has no meaningful effect'
    saved = plugin.raw_state
    put(feedback=.1, wavefolder=.9)
    plugin.raw_state = saved
    assert abs(float(plugin.feedback)-.6)<.002 and abs(float(plugin.wavefolder)-.3)<.002
    put(bypass=True, output=6.)
    bypass = plugin(signal, sr, buffer_size=1024)
    assert np.array_equal(signal, bypass), 'Bypass is not sample-exact'
    put(bypass=False, output=0., master_mix=1., feedback=.85, wavefolder=1., motion=1., space=1.)
    silence=plugin(np.zeros((2,sr),np.float32),sr,buffer_size=64)
    print('Silence peak after host reset:',float(np.max(np.abs(silence))),flush=True)
    if np.max(np.abs(silence)) != 0.:
        print('Silence diagnostic:',silence[:,:16].tolist(), 'nonzero', np.count_nonzero(silence),flush=True)
    assert np.max(np.abs(silence)) == 0., 'Silent input generates noise'
    # Distinct delay topology with identical parameter values and fresh state.
    put(feedback=.4, space=0., motion=0., wavefolder=0., delay_a=110., delay_b=270., delays_in_series=False)
    impulse = np.zeros((2,sr),np.float32); impulse[:,0]=.7
    parallel=plugin(impulse,sr,buffer_size=512)
    put(delays_in_series=True)
    serial=plugin(impulse,sr,buffer_size=512)
    assert float(np.sum(np.abs(parallel-serial)))>.1, 'Series/parallel are identical'
    results.append(dict(sample_rate=sr,dry_exact=True,bypass_exact=True,state_roundtrip=True,
                        silent_input=True,series_parallel_differ=True,wet_rms_difference=rms_delta))
    print('PASS', sr, flush=True)

put(master_mix=.7, bypass=False, output=0.)
start=time.perf_counter()
plugin(np.tile(signal,(1,10)),sr,buffer_size=512)
elapsed=time.perf_counter()-start
print(json.dumps({'checks':results,'render_10_seconds_at_96k_seconds':round(elapsed,3)},indent=2))
