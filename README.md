<h1 align="center">Fract-ol</h1>

![Buddha2](readme_imgs/buddha_2.png)

![Mandel1](readme_imgs/mandelbrot_1.png)
![Mandel2](readme_imgs/mandelbrot_2.png)

![Julia1](readme_imgs/julia_1.png)
![Julia2](readme_imgs/julia_2.png)
![Julia3](readme_imgs/julia_3.png)
![Julia4](readme_imgs/julia_4.png)

![Buddha1](readme_imgs/buddha_1.png)
![Buddha3](readme_imgs/buddha_3.png)
![Buddha4](readme_imgs/buddha_4.png)

![Fern1](readme_imgs/fern_2.png)
![Fern2](readme_imgs/fern_3.png)

## Buddha rendering checks

The Buddha renderer accepts square, landscape, and portrait output. Its
projection preserves square complex-plane pixels: the shorter image dimension
keeps the historical four-unit field of view and the longer dimension reveals
proportionally more orbit space instead of stretching the fractal. The pilot
and importance samplers use that same aspect-adjusted region. Because the
established Buddha orientation projects complex X vertically and complex Y
horizontally, rectangular sampling transposes its grid correspondence so the
additional samples feed the extended image axis. Square sampling remains
byte-for-byte unchanged.

Set the Buddha viewport directly at startup with named options after the type:

```sh
./fractol buddha 1080 1080 0 --center -0.21 0 --zoom 1.2
./fractol buddha 256 512 1 --zoom 2.5 --center 0.125 -0.75 \
  --importance-map
```

`--center` takes the real and imaginary complex-plane coordinates shown by the
Buddha settings printout. `--zoom` is the renderer's view scale: `1` is the
baseline scale, values greater than `1` zoom in, and values below `1` zoom out.
The named options may appear in any order. Omitting either option preserves the
selected Buddha type's existing default for that part of the view. Exported
Buddha PNG commands include the current center and zoom in hexadecimal
floating-point form so the viewport can be reconstructed exactly.

If a portrait output is taller than both its width and the screen height, the
window shows a true 90-degree clockwise rotation with swapped window
dimensions. This is display-only; PNG export keeps the requested width, height,
and upright orientation. The extra rotated image buffer and rotation pass are
created only for that oversized-portrait case.

The Buddha path prints map, sampling, and channel-total times by default.
Enable detailed phase and histogram profiling for any normal render:

```sh
FRACTOL_BUDDHA_PROFILE=1 ./fractol buddha 512 512 0
```

The profile reports importance-map, sampling, buffer-reduction, denoise,
statistics, and per-channel total times. It also prints compact histogram
statistics that are useful when comparing rendering changes. It additionally
reports the memory, clear time, and reduction time for the deterministic
per-worker sample histograms. Worker histograms are reused one sample buffer at
a time, so three-buffer rendering retains one full matrix per worker rather
than three. At 1080×1080 with four workers this reduces that pool from about
106.8 MiB to 35.6 MiB. Importance allocation is stored as one flat unsigned
32-bit count per working pixel instead of a double matrix, reducing that map
from 8.90 MiB to 4.45 MiB at 1080×1080. NLM variance smoothing uses two
double-precision rows rather than borrowing another full-image matrix.

Buddha types 1 and 2 use independent sample buffers. Their unbiased sample
variance feeds a joint-channel non-local means filter. The optimized interior
path uses rolling patch sums, a worker-local patch ring that evaluates each
three-channel comparison term once, and an exponential lookup table; exact
patch matching remains in use around image borders. Select two or three buffers
at startup, or disable NLM without changing the sample-buffer configuration:

```sh
FRACTOL_BUDDHA_BUFFERS=2 ./fractol buddha 512 512 0
FRACTOL_BUDDHA_BUFFERS=3 ./fractol buddha 512 512 0
FRACTOL_BUDDHA_NLM=0 ./fractol buddha 512 512 0
```

`n` controls the total sample budget across all buffers: `n=6` schedules about
36 samples per input cell in total, independent of the buffer count. Two
buffers devote more samples to each estimate, but estimate variance with only
one degree of freedom. Three buffers provide a more stable noise estimate. The
buffers use separately SplitMix-seeded xoroshiro128+ streams; sharing the
deterministic importance allocation does not correlate their random positions.
Both modes use nine density matrices when NLM is enabled: three means, three
variances, and three thread-safe output matrices. For Buddha 1, NLM defaults to
the less aggressive `patch=2`, `kc=0.75` while retaining the 15-pixel search
radius. The filter can be tuned at startup:

```sh
FRACTOL_BUDDHA_NLM_PATCH=2 FRACTOL_BUDDHA_NLM_KC=1.0 \
  ./fractol buddha 1080 1080 0
FRACTOL_BUDDHA_NLM_SEARCH=10 ./fractol buddha 1080 1080 0
```

After a filtered render completes, `F4` toggles between the retained unfiltered
buffer mean and the latest filtered result. `F5`/`F6` adjust `kc`, `F7`/`F8`
adjust patch radius, and `F9`/`F10` adjust search radius. Changing one of these
settings reruns only NLM from the retained unfiltered mean; it does not resample
orbits.

Smootherstep coloring uses a resolution-stable percentile white point. The
default maps the 99.9th density percentile to white. Change it at startup with
either a fraction or percentage:

```sh
FRACTOL_BUDDHA_WHITE_PERCENTILE=0.9995 ./fractol buddha 512 512 0
FRACTOL_BUDDHA_WHITE_PERCENTILE=99.95 ./fractol buddha 512 512 0
```

While the Buddha window is open, `,` lowers the percentile for more clipping
and `.` raises it for less clipping. Recoloring does not rerender the density.

The default tone normalization remains the independent per-channel percentile
mode. Press `7` to cycle through independent channel percentiles, one linked
RGB percentile, and the legacy per-channel maxima. The startup equivalent is:

```sh
FRACTOL_BUDDHA_NORMALIZATION=channel ./fractol buddha 512 512 0
FRACTOL_BUDDHA_NORMALIZATION=linked ./fractol buddha 512 512 0
FRACTOL_BUDDHA_NORMALIZATION=max ./fractol buddha 512 512 0
FRACTOL_BUDDHA_EXPOSURE=1.15 ./fractol buddha 512 512 0
```

Channel powers are signed. Positive powers use the normal density curve;
negative powers use a safe inverse-density curve that brightens faint positive
orbit traces, darkens dense traces, keeps zero-density pixels black, and clamps
each channel before RGB packing. Press `3` to cycle negative powers through
safe inverse, deterministic legacy channel spill, and hybrid duotone + spill.
Legacy spill recreates the old direct negative-power formula and its visible
high-bit spill without undefined integer overflow. When smootherstep is
enabled, spill is composed first and smootherstep is applied to the resulting
RGB bytes.

Hybrid mode builds a resolution-stable base from percentile-normalized channel
densities and the startup tone powers, maps it from sage green (`#B3D38E`) to
deep indigo (`#29314F`), then mixes in the legacy spill texture. `1`/`2`
decrease/increase the hybrid spill mix by `0.05`. The default mix is `0.20`.
Startup selection and tuning use:

```sh
FRACTOL_BUDDHA_NEGATIVE_MODE=hybrid \
FRACTOL_BUDDHA_HYBRID_SPILL=0.20 ./fractol buddha 1080 1080 1
```

Press `8` to invert all three power signs, or adjust individual signs with the
existing channel power keys. `+`/`-` changes global pre-quantization exposure,
`5` swaps the current and alternate tone states for A/B comparison, and `6`
restores the startup tone defaults. Press `4` to switch Q/A, W/S, and E/D
between signed-power editing and per-channel blue/green/red gain editing.
`Tab` selects the normal coarse or fine increment (`0.05` or `0.01`) across
the entire signed range. Press `X` to toggle a separate `0.001` extra-fine
increment from any current value. Crossing zero skips only the degenerate
exact-zero power.

Capture the three importance maps at render time with:

```sh
./fractol buddha 512 512 0 --importance-map
FRACTOL_BUDDHA_IMPORTANCE=1 ./fractol buddha 512 512 0
```

The startup option initially shows a log-scaled RGB view of the blue, green,
and red importance maps. Press `M` after rendering to toggle between it and
the Buddhabrot. While the map is displayed, press `N` to cycle through combined
RGB, blue-only, green-only, and red-only views. Map capture performs block
averaging and logarithmic normalization in double precision, then retains
three normalized 16-bit channels (six bytes per output pixel). When the option
is not enabled, no map or backup image is retained and the display feature adds
no map-copy or map-coloring work to the render.

Each map cell estimates the number of useful orbit hits visible in the current
viewport. The exact per-buffer budget is distributed over cells with detected
pilot-map support. A zero-importance cell is not given a lone, heavily
weighted fallback sample; that fallback can expose an entire single orbit as a
bright colored arc when NLM is disabled. An all-zero map still falls back to
uniform sampling. The weight is computed from the cell's actual integer sample
count, preserving the inverse allocation/weight relationship. A linear prefix
scan of those counts splits contiguous, non-overlapping work ranges between
workers without retaining a full image-sized CDF.

The three channel maps are generated in one escape-orbit traversal. The
combined pass is byte-for-byte equivalent to three separate passes when using
the same mean-hit metric. By default the allocation metric is RMS visible hits,
which uses the pilot pass's second moment and gives more weight to rare,
high-contribution cells. The effective pilot subdivision is resolution-aware:
configured `map_n=5` becomes 3 at 1080×1080. These settings have explicit A/B
fallbacks:

```sh
FRACTOL_BUDDHA_IMPORTANCE_METRIC=mean \
FRACTOL_BUDDHA_MAP_ADAPTIVE=0 \
FRACTOL_BUDDHA_GENERIC_ORBIT=1 \
FRACTOL_BUDDHA_PROFILE=1 ./fractol buddha 1080 1080 0
```

Remove one fallback at a time to compare the RMS metric, adaptive pilot, and
specialized quadratic orbit loop using identical deterministic seeds.

The specialized square path also rejects samples analytically inside the main
cardioid and period-2 bulb. Channels capped at 512 iterations use an 8 KiB
orbit cache local to each worker. Longer channels retain the exact two-pass
path because repeatedly filling a partial cache was not reliably faster. The
optimizations cover pilot-map and final sampling work where applicable and can
be disabled independently for deterministic comparisons:

```sh
FRACTOL_BUDDHA_INTERIOR_REJECTION=0 \
FRACTOL_BUDDHA_ORBIT_CACHE=0 \
FRACTOL_BUDDHA_PROFILE=1 ./fractol buddha 1080 1080 0
```

## PNG exports

Press `F3` to export the displayed image as a 16-bit RGB PNG. The MLX window
remains an 8-bit preview. PNG rows are generated and written one at a time, so
16-bit export does not retain a second full-size image buffer. Buddha exports
are tone-mapped directly from the retained double-precision density matrices;
supersampled output is accumulated before 16-bit quantization. Mandelbrot and
Julia colors are recomputed from their orbit values, and Fern colors are
reconstructed from its density map. Buddha adjustment filtering is applied to
the double-precision tone values, and Gaussian export caches only five
high-precision source rows while it streams the output. Importance maps export
their selected RGB or single-channel view directly from the retained 16-bit
normalized map values. NLM-filtered density output retains the full
high-precision path.

The completion message includes the actual generated filename, for example:

```text
EXPORT buddha_12.png COMPLETE (16-bit RGB)
```

Exports are tagged as sRGB and contain versioned PNG text metadata describing
the fractal, output and working dimensions, view, iteration and color state,
supersampling, and worker layout. Julia exports also store the exact constant;
Fern exports store the species, palette, and deterministic RNG configuration;
and Buddha exports store the type, sample and buffer counts, importance-map
configuration, channel ranges, formula, tone mapping, NLM, filtering, display
state, and RNG configuration. Floating-point values use hexadecimal notation
so they can be parsed back without decimal-rounding loss. `CLI Baseline`
provides a convenient starting command, while the remaining fields record
settings changed after startup.

The embedded fields can be inspected with a PNG metadata reader, for example:

```sh
exiftool -G1 -s buddha_12.png
```

Two small fixed-seed, single-worker computational reference cases cover Buddha
types 1 and 2 without requiring an X display:

```sh
make test-buddha
make reference-buddha
make benchmark-buddha
make benchmark-buddha-nlm
```

`test-buddha` checks the stored histogram sums, peaks, weighted checksums, and
nonzero counts. It also verifies one-, two-, and three-buffer workspace and
worker behavior, unbiased variance, exact importance budgets over detected
support, repeatable raw/filtered NLM ownership and retuning, bit-for-bit
deterministic parallel renders, and the regular PNG exporter's color format and
reconstruction metadata. Landscape/portrait projection scale and oversized
portrait rotation policy are covered as well, including density in the extended
non-square bands. The aspect matrix covers 1:1, 5:4, 4:3, 3:2, 16:9, and 2:1
in both orientations, checking exact sampling-area scaling, conjugate pilot-map
symmetry, continuity across the former square boundary, deterministic
rectangular sampling, and retained outer-band coverage before and after
three-buffer NLM. `reference-buddha`
prints the values used when deliberately updating those references.
`benchmark-buddha` compares the specialized and generic square orbit loops,
combined and separate channel-map construction, analytic interior rejection,
and cached/two-pass orbit accumulation while verifying identical orbit and
histogram results. `benchmark-buddha-nlm` times the initial and repeated NLM
passes over the same deterministic density and variance data, and verifies that
refiltering preserves the result.
