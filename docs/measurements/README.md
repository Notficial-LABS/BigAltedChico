# Measurement tables

Curated numerical results from the existing local macOS development checks. Raw WAVs, DAW sessions and machine-specific logs are excluded from Git and remain in `Working/Tests/`.

- `latency.csv`: 192 impulse comparisons; neutral wet, dry, plugin bypass and host bypass. Measured peak delay equals the processor report.
- `steady-pitch.csv`: spectral frequency estimates for steady synthetic fixtures.
- `rapid-notes.csv`: short-note stability diagnostics.
- `transients.csv`: transient energy width and centroid shift.

See [LATENCY](../LATENCY.md) for method, limits and the historical development-build context. These are not claims of testing Windows, Linux, every DAW, or every real voice. Run the current checks/analysis to generate fresh data locally.
