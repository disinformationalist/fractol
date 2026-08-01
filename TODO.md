# Potential work

This is a parking list, not a committed implementation order. Re-profile before
starting an optimization and retain deterministic comparisons wherever
possible.

## Buddha / NLM

- Clean up and define the normal non-fast Buddha renderer. Its view-transformed
  parameter-sampling policy currently differs from the fast renderer's fixed
  parameter domain, so it should eventually be made internally consistent,
  documented, and covered by center/zoom equivalence tests.
- Consider paired-offset NLM evaluation. Patch distance and weight are
  symmetric between two pixels, so the `(+dx, +dy)` and `(-dx, -dy)` work may
  be computed once and applied to both outputs. The design must use tiled or
  worker-local accumulation to avoid output races, preserve the current border
  behavior, and demonstrate that reduction and memory costs do not erase the
  saved comparisons.
- Investigate an offset-aware fast border path. The current exact border
  matcher is intentionally simple but becomes a noticeable share of small-image
  NLM time.
- Revisit two-buffer versus three-buffer defaults after collecting matched
  renders and profiles at the final sampling settings.

## Memory and large renders

- Investigate tiled worker-histogram reduction if per-worker full-image
  histograms become the next memory limit.
- Revisit tiled rendering and tiled PNG export when images exceed the practical
  single-workspace limit.
- Consider a wider importance-count representation only if a real render needs
  more than the guarded 32-bit per-buffer sample budget.

## Rendering quality and workflow

- Build a small repeatable comparison set covering square, landscape, portrait,
  two/three buffers, raw/filtered NLM, and representative tone modes.
- Continue replacing the fixed-grid deep-zoom importance proposal in measured
  stages. The recurrence-aware viewport-footprint score, deterministic
  beam-quadtree refinement, and frozen global/window/retained-leaf defensive
  mixture with full inverse-mixture weighting are implemented. Next, evaluate
  a Mandelbrot-boundary component and variance-aware mixture tuning using
  matched renders before enabling either. Keep proposals frozen across all NLM
  buffers.
- Compare the opt-in deterministic stratified-jitter pilot against the default
  centered grid in matched zoom-420/1000 renders. Initial profiling found no
  consistent discovery improvement, so do not enable it without better evidence.
- Consider local mutation and reverse-orbit/Newton guidance only after the
  defensive mixture and adaptive tree are correct; retain a nonzero global
  component so undiscovered support remains reachable.
- Consider metadata-driven command reconstruction once the first stable
  metadata format and startup controls are settled.



## Some interesting center/zoom settings

	/* fractal->move_x = -0.0425;//.21//-x coord 
	fractal->move_y = 0.9862;//-y coord
	fractal->zoom = 420;//420;//10;//1.2;//420;//zoom lvl,  */

	fractal->move_x =  -.21;   //    .34;  //  -.158;
	fractal->move_y =  0;      //   -.7;   // -1.033;
	fractal->zoom   =  1.2;   //     5;    //  200;

	/* fractal->move_x = -0.042865;
	fractal->move_y = 0.989751;
	fractal->zoom = 55150;//55150 */
