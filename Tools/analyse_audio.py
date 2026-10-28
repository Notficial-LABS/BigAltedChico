#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Analyse the reproducible processor WAVs. Requires numpy, scipy, soundfile.
No latency is inferred from the silence trimmed at export: files preserve time zero.
"""
import os
os.environ.setdefault("OPENBLAS_NUM_THREADS", "1")
import csv
from pathlib import Path
import numpy as np
import soundfile as sf
from scipy.optimize import minimize_scalar
ROOT = Path(__file__).resolve().parents[1] / 'Working' / 'Tests'

def tone_frequency(x, rate):
    window = np.hanning(len(x))
    spectrum = abs(np.fft.rfft(x * window, n=2**int(np.ceil(np.log2(len(x)*4)))))
    index = np.argmax(spectrum[1:]) + 1
    coarse = index * rate / ((len(spectrum)-1)*2)
    t = np.arange(len(x)) / rate
    def objective(f):
        return -abs(np.sum(x * window * np.exp(-2j*np.pi*f*t)))**2
    result = minimize_scalar(objective, bounds=(max(1, coarse-3), coarse+3), method='bounded', options={'xatol':1e-7})
    frequency = result.x
    a = np.column_stack((np.sin(2*np.pi*frequency*t), np.cos(2*np.pi*frequency*t), np.ones(len(x))))
    fit = a @ np.linalg.lstsq(a, x, rcond=None)[0]
    snr = 10*np.log10((np.sum(fit**2)+1e-20)/(np.sum((x-fit)**2)+1e-20))
    return frequency, snr

rows = list(csv.DictReader((ROOT/'renders.csv').open()))
steady = []
for row in rows:
    x, rate = sf.read(ROOT/'audio'/row['file'])
    assert np.isfinite(x).all()
    assert max(abs(x)) < 1, f"Clipping: {row['file']}"
    if float(row['expected_hz']) <= 0: continue
    delay = int(row['reported_samples'])
    f, snr = tone_frequency(x[int(.8*rate)+delay:int(2.2*rate)+delay], rate)
    cents = 1200*np.log2(f/float(row['expected_hz']))
    steady.append({'file':row['file'],'expected_hz':row['expected_hz'],'measured_hz':f'{f:.4f}',
                   'error_cents':f'{cents:.2f}','tone_to_residual_db':f'{snr:.2f}'})
with (ROOT/'steady-pitch.csv').open('w') as f:
    writer=csv.DictWriter(f,fieldnames=steady[0].keys());writer.writeheader();writer.writerows(steady)

transients=[]
for row in rows:
    if not row['file'].startswith('transients'):continue
    x,rate=sf.read(ROOT/'audio'/row['file']);delay=int(row['reported_samples'])
    widths=[]; shifts=[]
    for event in np.arange(.75,2.26,.25):
        center=round(event*rate)+delay
        lo=center-round(.1*rate); hi=center+round(.1*rate)
        energy=x[lo:hi]**2; cumulative=np.cumsum(energy); cumulative/=cumulative[-1]
        q=np.searchsorted(cumulative,[.05,.95]);widths.append((q[1]-q[0])*1000/rate)
        shifts.append((np.sum(np.arange(lo,hi)*energy)/sum(energy)-center)*1000/rate)
    transients.append({'file':row['file'],'energy_5_95_width_ms':f'{np.median(widths):.3f}',
                       'energy_centroid_offset_from_reported_ms':f'{np.median(shifts):.3f}'})
with (ROOT/'transients.csv').open('w') as f:
    writer=csv.DictWriter(f,fieldnames=transients[0].keys());writer.writeheader();writer.writerows(transients)
# Rapid: 40 ms Hann windows, 5 ms steps. Evaluate centers 25..55 ms into
# each 80 ms note, after aligning by reported latency. This is a diagnostic
# of brief-note pitch stability, not a perceptual quality score.
rapid=[]
for row in rows:
    if not row['file'].startswith('rapid'):continue
    x,rate=sf.read(ROOT/'audio'/row['file']);delay=int(row['reported_samples'])
    errors=[]
    for step in range(4,27):
        hz=[85,230,455,852][step%4]
        target=hz*2**(7/12) if row['tuning']=='0' else 440*2**(round(12*np.log2(hz/440))/12) if row['tuning']=='1' else 523.25113
        for offset in [.025,.04,.055]:
            center=round((.25+step*.08+offset)*rate)+delay
            window=x[center-960:center+960]
            f,_=tone_frequency(window,rate)
            errors.append(abs(1200*np.log2(f/target)))
    rapid.append({'file':row['file'],'median_abs_cents':f'{np.median(errors):.2f}',
                  'frames_within_50_cents_percent':f'{100*np.mean(np.array(errors)<50):.2f}'})
with (ROOT/'rapid-notes.csv').open('w') as f:
    writer=csv.DictWriter(f,fieldnames=rapid[0].keys());writer.writeheader();writer.writerows(rapid)
print('Steady pitch:');print((ROOT/'steady-pitch.csv').read_text())
print('Transients:');print((ROOT/'transients.csv').read_text())
print('Rapid notes:');print((ROOT/'rapid-notes.csv').read_text())
tracker=[]
for processing in ['Quality','Live']:
    for fixture in ['bass','high','rapid']:
        trace=np.genfromtxt(ROOT/f'{fixture}-{processing}-tracker.csv',delimiter=',',skip_header=1)
        delays=[]
        for step in range(27 if fixture=='rapid' else 1):
            edge=.25+step*.08
            target=[85,230,455,852][step%4] if fixture=='rapid' else 82.4069 if fixture=='bass' else 880
            end=edge+.08 if fixture=='rapid' else 1.0
            frames=trace[(trace[:,0]/48000>=edge)&(trace[:,0]/48000<end)]
            matches=frames[abs(frames[:,1]-target)/target<.02]
            if len(matches): delays.append((matches[0,0]/48000-edge)*1000)
        tracker.append({'processing':processing,'fixture':fixture,'detected_segments':len(delays),
                        'total_segments':27 if fixture=='rapid' else 1,
                        'median_input_tracking_delay_ms':f'{np.median(delays):.3f}' if delays else 'NA'})
with (ROOT/'tracker-response.csv').open('w') as f:
    writer=csv.DictWriter(f,fieldnames=tracker[0].keys());writer.writeheader();writer.writerows(tracker)
print('Tracker response:');print((ROOT/'tracker-response.csv').read_text())
