#include "fractol.h"

int	buddha_map_scale(t_fractal *fractal)
{
	int	configured;
	int	effective;
	int	max_dimension;

	configured = ft_round(fractal->buddha->map_n);
	if (configured < 1)
		configured = 1;
	if (!fractal->buddha->map_adaptive)
		return (configured);
	max_dimension = fractal->width;
	if (fractal->height > max_dimension)
		max_dimension = fractal->height;
	effective = ft_round((double)configured
			* (double)BUDDHA_MAP_REFERENCE_SIZE / (double)max_dimension);
	if (effective < 1)
		effective = 1;
	if (effective > configured)
		effective = configured;
	return (effective);
}

static void	set_projection_bounds(t_comps *comps)
{
	double	base;
	double	x_half_span;
	double	y_half_span;

	base = (double)comps->width - 1.0;
	if (comps->height < comps->width)
		base = (double)comps->height - 1.0;
	x_half_span = 2.0 * ((double)comps->height - 1.0) / base;
	y_half_span = 2.0 * ((double)comps->width - 1.0) / base;
	comps->x_cmin = -x_half_span;
	comps->x_cmax = x_half_span;
	comps->y_cmin = y_half_span;
	comps->y_cmax = -y_half_span;
	comps->x_span = comps->x_cmax - comps->x_cmin;
	comps->y_span = comps->y_cmax - comps->y_cmin;
}

static void	set_sample_bounds(t_comps *comps)
{
	comps->sample_x_min = comps->x_cmin;
	comps->sample_y_min = comps->y_cmin;
	comps->sample_x_span = comps->x_span;
	comps->sample_y_span = comps->y_span;
}

t_complex	buddha_sample_point(t_comps comps, double pixel_x,
		double pixel_y)
{
	t_complex	c;

	if (comps.width == comps.height)
	{
		c.x = map_2(pixel_x, comps.sample_x_min, comps.slopex_to);
		c.y = map_2(pixel_y, comps.sample_y_min, comps.slopey_to);
	}
	else
	{
		c.x = map_2(pixel_y, comps.sample_x_min, comps.slopex_to);
		c.y = map_2(pixel_x, comps.sample_y_min, comps.slopey_to);
	}
	return (c);
}

t_comps	set_comps(t_fractal *fractal, bool map)
{
	t_comps	comps;
	double	sample_scale;

	memset(&comps, 0, sizeof(comps));
	comps.width = fractal->width;
	comps.height = fractal->height;
	comps.zoom = fractal->zoom;
	set_projection_bounds(&comps);
	set_sample_bounds(&comps);
	sample_scale = fractal->buddha->n;
	if (sample_scale < 1.0)
		sample_scale = 1.0;
	comps.map_scale = buddha_map_scale(fractal);
	if (map)
	{
		comps.slopex_to = comps.sample_x_span
			/ ((double)comps.height * (double)comps.map_scale);
		comps.slopey_to = comps.sample_y_span
			/ ((double)comps.width * (double)comps.map_scale);
	}
	else
	{
		comps.slopex_to = comps.sample_x_span / (double)comps.height;
		comps.slopey_to = comps.sample_y_span / (double)comps.width;
		comps.samples = (double)fractal->size
			* sample_scale * sample_scale;
		comps.sample_budget = buddha_sample_budget(fractal);
	}
	comps.slopex_back = ((double)comps.height - 1.0) / comps.x_span;
	comps.slopey_back = ((double)comps.width - 1.0) / comps.y_span;
	comps.move_x = fractal->move_x;
	comps.move_y = fractal->move_y;
	comps.inv_zoom = 1.0 / comps.zoom;
	comps.complex_f = fractal->complex_f;
	comps.square_formula = fractal->buddha->square_specialized
		&& fractal->complex_f == &square_complex;
	comps.interior_rejection = fractal->buddha->interior_rejection;
	comps.sample_counts = fractal->sample_counts;
	comps.b_max_i = fractal->b_max_i;
	comps.b_min_i = fractal->b_min_i;
	comps.bound = fractal->bound;
	return (comps);
}

static double	importance_score(double sum, double sum_squares,
		int samples, bool rms)
{
	if (!rms)
		return (sum);
	return (sqrt(sum_squares / (double)samples));
}

static int	map_escape_length(t_complex c, t_comps comps, t_complex *cache)
{
	if (cache && comps.square_formula)
		return (buddha_escape_orbit(c, comps, comps.b_max_i, cache,
				BUDDHA_ORBIT_CACHE_POINTS));
	return (buddha_escape_length(c, comps, comps.b_max_i));
}

static int	map_visible_hits(t_complex c, t_comps comps,
		int orbit_length, t_complex *cache)
{
	if (cache && comps.square_formula
		&& orbit_length <= BUDDHA_ORBIT_CACHE_POINTS)
		return (buddha_cached_visible_hits(cache, comps, orbit_length));
	return (buddha_visible_hits(c, comps, orbit_length));
}

static double	map_cell_importance(int pixel_x, int pixel_y, t_comps comps,
		bool rms, t_complex *orbit_cache)
{
	t_complex	c;
	double		sum;
	double		sum_squares;
	int			hits;
	int			orbit_length;
	int			sub_x;
	int			sub_y;

	sum = 0.0;
	sum_squares = 0.0;
	sub_y = -1;
	while (++sub_y < comps.map_scale)
	{
		sub_x = -1;
		while (++sub_x < comps.map_scale)
		{
			c = buddha_sample_point(comps,
					(double)(pixel_x * comps.map_scale + sub_x) + 0.5,
					(double)(pixel_y * comps.map_scale + sub_y) + 0.5);
			orbit_length = map_escape_length(c, comps, orbit_cache);
			hits = 0;
			if (orbit_length < comps.b_max_i
				&& orbit_length > comps.b_min_i)
				hits = map_visible_hits(c, comps, orbit_length, orbit_cache);
			sum += (double)hits;
			sum_squares += (double)hits * (double)hits;
		}
	}
	return (importance_score(sum, sum_squares,
			comps.map_scale * comps.map_scale, rms));
}

static void	channel_limits(t_buddha *b, int minimum[3], int maximum[3])
{
	minimum[0] = b->min1;
	minimum[1] = b->min2;
	minimum[2] = b->min3;
	maximum[0] = b->max1;
	maximum[1] = b->max2;
	maximum[2] = b->max3;
}

static void	map_cell_all(t_fractal *fractal, int pixel_x, int pixel_y,
		t_comps comps, t_complex *orbit_cache)
{
	t_complex	c;
	double		sum[3];
	double		sum_squares[3];
	int			minimum[3];
	int			maximum[3];
	int			orbit_length;
	int			hits;
	int			channel;
	int			sub_x;
	int			sub_y;
	int			pilot_samples;

	memset(sum, 0, sizeof(sum));
	memset(sum_squares, 0, sizeof(sum_squares));
	channel_limits(fractal->buddha, minimum, maximum);
	pilot_samples = comps.map_scale * comps.map_scale;
	sub_y = -1;
	while (++sub_y < comps.map_scale)
	{
		sub_x = -1;
		while (++sub_x < comps.map_scale)
		{
			c = buddha_sample_point(comps,
					(double)(pixel_x * comps.map_scale + sub_x) + 0.5,
					(double)(pixel_y * comps.map_scale + sub_y) + 0.5);
			orbit_length = map_escape_length(c, comps, orbit_cache);
			channel = -1;
			while (++channel < 3)
				if (orbit_length < maximum[channel]
					&& orbit_length > minimum[channel])
					break ;
			if (channel == 3)
				continue ;
			hits = map_visible_hits(c, comps, orbit_length, orbit_cache);
			channel = -1;
			while (++channel < 3)
			{
				if (orbit_length >= maximum[channel]
					|| orbit_length <= minimum[channel])
					continue ;
				sum[channel] += (double)hits;
				sum_squares[channel] += (double)hits * (double)hits;
			}
		}
	}
	channel = -1;
	while (++channel < 3)
		fractal->densities[channel][pixel_y][pixel_x]
			= importance_score(sum[channel], sum_squares[channel],
				pilot_samples, fractal->buddha->importance_rms);
}

void	*buddha_set_map(void *arg)
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
	comps = set_comps(fractal, true);
	cache = NULL;
	if (fractal->buddha->orbit_cache && comps.square_formula
		&& comps.b_max_i <= BUDDHA_ORBIT_CACHE_POINTS)
		cache = orbit_cache;
	index = (size_t)piece->x_s;
	end = (size_t)piece->x_e;
	while (index < end)
	{
		x = (int)(index % (size_t)fractal->width);
		y = (int)(index / (size_t)fractal->width);
		fractal->densities[fractal->hist_num][y][x]
			= map_cell_importance(x, y, comps,
				fractal->buddha->importance_rms, cache);
		index++;
	}
	return (NULL);
}

static void	*buddha_set_map_all(void *arg)
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
	comps = set_comps(fractal, true);
	comps.b_max_i = fractal->buddha->max1;
	if (fractal->buddha->max2 > comps.b_max_i)
		comps.b_max_i = fractal->buddha->max2;
	if (fractal->buddha->max3 > comps.b_max_i)
		comps.b_max_i = fractal->buddha->max3;
	cache = NULL;
	if (fractal->buddha->orbit_cache && comps.square_formula
		&& comps.b_max_i <= BUDDHA_ORBIT_CACHE_POINTS)
		cache = orbit_cache;
	index = (size_t)piece->x_s;
	end = (size_t)piece->x_e;
	while (index < end)
	{
		x = (int)(index % (size_t)fractal->width);
		y = (int)(index / (size_t)fractal->width);
		map_cell_all(fractal, x, y, comps, cache);
		index++;
	}
	return (NULL);
}

static void	run_map_workers(t_fractal *fractal, void *(*worker_fn)(void *))
{
	int		workers;
	int		worker;
	t_piece	pieces[fractal->worker_histogram_count];

	workers = fractal->worker_histogram_count;
	worker = -1;
	while (++worker < workers)
	{
		pieces[worker].x_s = (int)((size_t)worker
				* (size_t)fractal->size / (size_t)workers);
		pieces[worker].x_e = (int)((size_t)(worker + 1)
				* (size_t)fractal->size / (size_t)workers);
		pieces[worker].fractal = fractal;
		if (pthread_create(&fractal->threads[worker], NULL,
				worker_fn, &pieces[worker]) != 0)
			thread_error(fractal, worker);
	}
	join_threads(fractal->threads, workers, 1);
}

void	buddha_map(t_fractal *fractal)
{
	run_map_workers(fractal, buddha_set_map);
}

void	buddha_map_all(t_fractal *fractal)
{
	run_map_workers(fractal, buddha_set_map_all);
}
