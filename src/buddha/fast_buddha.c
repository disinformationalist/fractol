#include "fractol.h"

static void	build_work_ends(t_fractal *fractal, int *ends)
{
	size_t	budget;
	size_t	cumulative;
	size_t	index;
	size_t	target;
	int		worker;
	int		workers;

	budget = buddha_sample_budget(fractal);
	workers = fractal->worker_histogram_count;
	cumulative = 0;
	index = 0;
	worker = -1;
	while (++worker < workers - 1)
	{
		target = (size_t)((long double)(worker + 1)
				* (long double)budget / (long double)workers);
		while (index < (size_t)fractal->size && cumulative < target)
		{
			cumulative += (size_t)fractal->sample_counts[index];
			index++;
		}
		ends[worker] = (int)index;
	}
	ends[worker] = fractal->size;
}

static void	sample_pixel(t_fractal *fractal, int x, int y,
		t_comps comps, Xoro128 *rng, int worker, t_complex *orbit_cache)
{
	t_complex	c;
	double		weight;
	double		sample_x;
	double		sample_y;
	size_t		pixel_samples;
	size_t		sample;

	pixel_samples = (size_t)comps.sample_counts[
			(size_t)y * (size_t)comps.width + (size_t)x];
	if (pixel_samples == 0)
		return ;
	weight = (double)comps.sample_budget / (double)pixel_samples;
	comps.density_flat = fractal->worker_histograms[worker];
	sample = 0;
	while (sample++ < pixel_samples)
	{
		sample_x = (double)x + xoro128d(rng);
		sample_y = (double)y + xoro128d(rng);
		c = buddha_sample_point(comps, sample_x, sample_y);
		buddha_iteration(c, weight, comps, orbit_cache);
	}
}

static void	*buddha_set_fast(void *arg)
{
	t_piece		*piece;
	t_fractal	*fractal;
	t_comps		comps;
	size_t		index;
	size_t		end;
	int			x;
	int			y;
	t_complex	orbit_cache[BUDDHA_ORBIT_CACHE_POINTS];
	t_complex	*cache;

	piece = (t_piece *)arg;
	fractal = piece->fractal;
	comps = set_comps(fractal, false);
	cache = NULL;
	if (fractal->buddha->orbit_cache && comps.square_formula
		&& comps.b_max_i <= BUDDHA_ORBIT_CACHE_POINTS)
		cache = orbit_cache;
	index = (size_t)piece->x_s;
	end = (size_t)piece->x_e;
	y = (int)(index / (size_t)fractal->width);
	x = (int)(index - (size_t)y * (size_t)fractal->width);
	while (index < end)
	{
		sample_pixel(fractal, x, y, comps, &piece->rng, piece->id, cache);
		index++;
		x++;
		if (x == fractal->width)
		{
			x = 0;
			y++;
		}
	}
	return (NULL);
}

static void	start_fast_piece(t_fractal *fractal, t_piece *piece,
		int worker, int start, int end, uint64_t seed)
{
	piece->id = worker;
	piece->x_s = start;
	piece->x_e = end;
	piece->fractal = fractal;
	sxoro128(&piece->rng, seed + (uint64_t)worker * 142857ULL);
	if (pthread_create(&fractal->threads[worker], NULL,
			buddha_set_fast, piece) != 0)
		thread_error(fractal, worker);
}

static void	run_fast_buffer(t_fractal *fractal, int buffer, int *ends)
{
	int			workers;
	t_piece		pieces[fractal->worker_histogram_count];
	uint64_t	seed;
	int			start;
	int			end;
	int			worker;

	workers = fractal->worker_histogram_count;
	seed = 0xFABDECAF
		+ (uint64_t)buffer * 0x9E3779B97F4A7C15ULL;
	buddha_clear_worker_histograms(fractal);
	start = 0;
	worker = -1;
	while (++worker < workers - 1)
	{
		end = ends[worker];
		start_fast_piece(fractal, &pieces[worker], worker,
			start, end, seed);
		start = end;
	}
	end = fractal->size;
	start_fast_piece(fractal, &pieces[worker], worker, start, end, seed);
	join_threads(fractal->threads, workers, 1);
	buddha_reduce_worker_histograms(fractal, buffer);
}

void	fast_buddha(t_fractal *fractal)
{
	int	buffer;
	int	ends[fractal->worker_histogram_count];

	build_work_ends(fractal, ends);
	buffer = -1;
	while (++buffer < fractal->buffs)
		run_fast_buffer(fractal, buffer, ends);
}
