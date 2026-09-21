"""Render audio through the compiled VST3, with a six-second reverb tail."""
import sys
from pathlib import Path
import numpy as np
from pedalboard import load_plugin
from pedalboard.io import AudioFile
path=Path(sys.argv[1]).resolve()
if sys.platform == 'win32' and path.is_dir():path=path/'Contents'/'x86_64-win'/path.name
plugin=load_plugin(str(path))
for key,value in dict(delay_a=480.,delay_b=720.,feedback=.62,filter=3400.,wavefolder=.08,
                      space=.72,motion=.5,master_mix=.64,lfo_rate=.13,output=0.,bypass=False,
                      delays_in_series=False).items():setattr(plugin,key,value)
with AudioFile(sys.argv[2]) as f: sr=f.samplerate; audio=f.read(f.frames)
audio=np.pad(audio,((0,0),(0,int(sr*6))))
out=plugin(audio,sr,buffer_size=512)
out[:,-int(sr*2):]*=np.linspace(1,0,int(sr*2))
assert np.isfinite(out).all()
with AudioFile(sys.argv[3],'w',sr,num_channels=2,bit_depth=24) as f:f.write(out)
print('Actual VST3 render:',len(out[0])/sr,'seconds; peak:',float(np.max(np.abs(out))))
