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

static uint64_t	pilot_hash(uint64_t value)
{
	value ^= value >> 30;
	value *= 0xBF58476D1CE4E5B9ULL;
	value ^= value >> 27;
	value *= 0x94D049BB133111EBULL;
	return (value ^ (value >> 31));
}

static double	pilot_offset(int fine_x, int fine_y, int map_scale, int axis)
{
	uint64_t	key;

	key = BUDDHA_PILOT_JITTER_SEED;
	key ^= (uint64_t)(uint32_t)fine_x * 0x9E3779B97F4A7C15ULL;
	key ^= (uint64_t)(uint32_t)fine_y * 0xD2B74407B1CE6E93ULL;
	key ^= (uint64_t)(uint32_t)map_scale * 0xCA5A826395121157ULL;
	if (axis != 0)
		key ^= 0xA24BAED4963EE407ULL;
	return (((double)(pilot_hash(key) >> 11) + 0.5) * 0x1.0p-53);
}

static t_complex	pilot_sample(t_buddha *buddha, t_comps comps,
		int pixel_x, int pixel_y, int sub_x, int sub_y)
{
	t_complex	c;
	double		real_offset;
	double		imaginary_offset;
	int			fine_x;
	int			fine_y;
	int			real_index;
	int			imaginary_index;
	int			imaginary_pair;
	int			imaginary_count;
	bool		mirrored;
	bool		self_pair;

	fine_x = pixel_x * comps.map_scale + sub_x;
	fine_y = pixel_y * comps.map_scale + sub_y;
	if (!buddha->importance_pilot_jitter || comps.map_scale <= 1)
		return (buddha_sample_point(comps, (double)fine_x + 0.5,
				(double)fine_y + 0.5));
	real_index = fine_x;
	imaginary_index = fine_y;
	imaginary_count = comps.height * comps.map_scale;
	if (comps.width != comps.height)
	{
		real_index = fine_y;
		imaginary_index = fine_x;
		imaginary_count = comps.width * comps.map_scale;
	}
	imaginary_pair = imaginary_count - 1 - imaginary_index;
	mirrored = imaginary_index > imaginary_pair;
	self_pair = imaginary_index == imaginary_pair;
	if (mirrored)
		imaginary_index = imaginary_pair;
	real_offset = 0.5;
	imaginary_offset = 0.5;
	real_offset = pilot_offset(real_index, imaginary_index,
			comps.map_scale, 0);
	if (!self_pair)
		imaginary_offset = pilot_offset(real_index, imaginary_index,
				comps.map_scale, 1);
	if (comps.width == comps.height)
		c = buddha_sample_point(comps, (double)real_index + real_offset,
				(double)imaginary_index + imaginary_offset);
	else
		c = buddha_sample_point(comps,
				(double)imaginary_index + imaginary_offset,
				(double)real_index + real_offset);
	if (mirrored)
		c.y = -c.y;
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

static double	map_visible_score(t_complex c, t_comps comps,
		int orbit_length, t_complex *cache, t_buddha_score_scratch *scratch,
		bool recurrence, int *hits)
{
	if (!recurrence)
	{
		*hits = map_visible_hits(c, comps, orbit_length, cache);
		return ((double)*hits);
	}
	if (cache && comps.square_formula
		&& orbit_length <= BUDDHA_ORBIT_CACHE_POINTS)
		return (buddha_cached_visible_footprint(cache, comps,
				orbit_length, scratch, hits));
	return (buddha_visible_footprint(c, comps, orbit_length, scratch, hits));
}

static double	map_cell_importance(t_piece *piece, int pixel_x, int pixel_y,
		t_comps comps, bool rms, t_complex *orbit_cache,
		t_buddha_score_scratch *score_scratch)
{
	t_complex	c;
	double		sum;
	double		sum_squares;
	double		score;
	int			hits;
	int			orbit_length;
	int			channel;
	int			sub_x;
	int			sub_y;

	channel = piece->fractal->hist_num;
	sum = 0.0;
	sum_squares = 0.0;
	sub_y = -1;
	while (++sub_y < comps.map_scale)
	{
		sub_x = -1;
		while (++sub_x < comps.map_scale)
		{
			c = pilot_sample(piece->fractal->buddha, comps,
					pixel_x, pixel_y, sub_x, sub_y);
			piece->pilot_stats.samples[channel]++;
			orbit_length = map_escape_length(c, comps, orbit_cache);
			hits = 0;
			if (orbit_length < comps.b_max_i
				&& orbit_length > comps.b_min_i)
			{
				piece->pilot_stats.eligible[channel]++;
				score = map_visible_score(c, comps, orbit_length,
						orbit_cache, score_scratch,
						piece->fractal->buddha->importance_recurrence, &hits);
			}
			else
				score = 0.0;
			if (hits > 0)
				piece->pilot_stats.useful[channel]++;
			piece->pilot_stats.visible_hits[channel] += (uint64_t)hits;
			piece->pilot_stats.score_sum[channel] += (long double)score;
			sum += score;
			sum_squares += score * score;
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

static void	map_cell_all(t_piece *piece, int pixel_x, int pixel_y,
		t_comps comps, t_complex *orbit_cache,
		t_buddha_score_scratch *score_scratch)
{
	t_fractal	*fractal;
	t_complex	c;
	double		sum[3];
	double		sum_squares[3];
	int			minimum[3];
	int			maximum[3];
	int			orbit_length;
	int			hits;
	double		score;
	int			channel;
	int			sub_x;
	int			sub_y;
	int			pilot_samples;

	fractal = piece->fractal;
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
			c = pilot_sample(fractal->buddha, comps,
					pixel_x, pixel_y, sub_x, sub_y);
			orbit_length = map_escape_length(c, comps, orbit_cache);
			channel = -1;
			while (++channel < 3)
			{
				piece->pilot_stats.samples[channel]++;
				if (orbit_length < maximum[channel]
					&& orbit_length > minimum[channel])
					piece->pilot_stats.eligible[channel]++;
			}
			channel = -1;
			while (++channel < 3)
				if (orbit_length < maximum[channel]
					&& orbit_length > minimum[channel])
					break ;
			if (channel >= 3)
				continue ;
			score = map_visible_score(c, comps, orbit_length, orbit_cache,
					score_scratch, fractal->buddha->importance_recurrence,
					&hits);
			channel = -1;
			while (++channel < 3)
			{
				if (orbit_length >= maximum[channel]
					|| orbit_length <= minimum[channel])
					continue ;
				if (hits > 0)
					piece->pilot_stats.useful[channel]++;
				piece->pilot_stats.visible_hits[channel] += (uint64_t)hits;
				piece->pilot_stats.score_sum[channel] += (long double)score;
				sum[channel] += score;
				sum_squares[channel] += score * score;
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
	t_buddha_score_scratch	score_scratch;
	t_complex	*cache;

	piece = (t_piece *)arg;
	fractal = piece->fractal;
	comps = set_comps(fractal, true);
	cache = NULL;
	if (fractal->buddha->orbit_cache && comps.square_formula
		&& comps.b_max_i <= BUDDHA_ORBIT_CACHE_POINTS)
		cache = orbit_cache;
	if (fractal->buddha->importance_recurrence)
		memset(&score_scratch, 0, sizeof(score_scratch));
	index = (size_t)piece->x_s;
	end = (size_t)piece->x_e;
	while (index < end)
	{
		x = (int)(index % (size_t)fractal->width);
		y = (int)(index / (size_t)fractal->width);
		fractal->densities[fractal->hist_num][y][x]
			= map_cell_importance(piece, x, y, comps,
				fractal->buddha->importance_rms, cache, &score_scratch);
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
	t_buddha_score_scratch	score_scratch;
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
	if (fractal->buddha->importance_recurrence)
		memset(&score_scratch, 0, sizeof(score_scratch));
	index = (size_t)piece->x_s;
	end = (size_t)piece->x_e;
	while (index < end)
	{
		x = (int)(index % (size_t)fractal->width);
		y = (int)(index / (size_t)fractal->width);
		map_cell_all(piece, x, y, comps, cache, &score_scratch);
		index++;
	}
	return (NULL);
}

static void	run_map_workers(t_fractal *fractal, void *(*worker_fn)(void *))
{
	int		workers;
	int		worker;
	int		channel;
	t_piece	pieces[fractal->worker_histogram_count];

	workers = fractal->worker_histogram_count;
	memset(&fractal->buddha->pilot_stats, 0,
		sizeof(fractal->buddha->pilot_stats));
	memset(pieces, 0, sizeof(pieces));
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
	worker = -1;
	while (++worker < workers)
	{
		channel = -1;
		while (++channel < BUDDHA_CHANNELS)
		{
			fractal->buddha->pilot_stats.samples[channel]
				+= pieces[worker].pilot_stats.samples[channel];
			fractal->buddha->pilot_stats.eligible[channel]
				+= pieces[worker].pilot_stats.eligible[channel];
			fractal->buddha->pilot_stats.useful[channel]
				+= pieces[worker].pilot_stats.useful[channel];
			fractal->buddha->pilot_stats.visible_hits[channel]
				+= pieces[worker].pilot_stats.visible_hits[channel];
			fractal->buddha->pilot_stats.score_sum[channel]
				+= pieces[worker].pilot_stats.score_sum[channel];
		}
	}
}

void	buddha_map(t_fractal *fractal)
{
	buddha_clear_importance_proposal(fractal->buddha);
	run_map_workers(fractal, buddha_set_map);
	buddha_refine_importance(fractal, fractal->hist_num);
}

void	buddha_map_all(t_fractal *fractal)
{
	buddha_clear_importance_proposal(fractal->buddha);
	run_map_workers(fractal, buddha_set_map_all);
	buddha_refine_importance(fractal, -1);
}
