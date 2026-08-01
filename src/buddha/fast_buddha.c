#include "fractol.h"

typedef struct s_buddha_proposal
{
	t_comps	comps;
	double	window[4];
	double	window_area;
	double	global_mix;
	double	window_mix;
	double	leaf_mix;
	double	leaf_total;
	size_t	global_samples;
	size_t	window_samples;
	size_t	leaf_samples;
	int		channel;
}	t_buddha_proposal;

static double	clamp_coordinate(double value, double high)
{
	if (value < 0.0)
		return (0.0);
	if (value > high)
		return (high);
	return (value);
}

static void	sorted_pair(double first, double second, double pair[2])
{
	pair[0] = fmin(first, second);
	pair[1] = fmax(first, second);
}

static void	proposal_window(t_comps comps, double window[4])
{
	double	c_x[2];
	double	c_y[2];
	double	pixel_x[2];
	double	pixel_y[2];

	sorted_pair(comps.move_x + comps.x_cmin * comps.inv_zoom,
		comps.move_x + comps.x_cmax * comps.inv_zoom, c_x);
	sorted_pair(-comps.move_y + comps.y_cmin * comps.inv_zoom,
		-comps.move_y + comps.y_cmax * comps.inv_zoom, c_y);
	if (comps.width == comps.height)
	{
		sorted_pair((c_x[0] - comps.sample_x_min) / comps.slopex_to,
			(c_x[1] - comps.sample_x_min) / comps.slopex_to, pixel_x);
		sorted_pair((c_y[0] - comps.sample_y_min) / comps.slopey_to,
			(c_y[1] - comps.sample_y_min) / comps.slopey_to, pixel_y);
	}
	else
	{
		sorted_pair((c_y[0] - comps.sample_y_min) / comps.slopey_to,
			(c_y[1] - comps.sample_y_min) / comps.slopey_to, pixel_x);
		sorted_pair((c_x[0] - comps.sample_x_min) / comps.slopex_to,
			(c_x[1] - comps.sample_x_min) / comps.slopex_to, pixel_y);
	}
	window[0] = clamp_coordinate(pixel_x[0], (double)comps.width);
	window[1] = clamp_coordinate(pixel_x[1], (double)comps.width);
	window[2] = clamp_coordinate(pixel_y[0], (double)comps.height);
	window[3] = clamp_coordinate(pixel_y[1], (double)comps.height);
}

static void	prepare_leaf_lookup(t_fractal *fractal)
{
	t_buddha_leaf	*leaves;
	size_t			index;
	uint32_t		cell;

	memset(fractal->sample_counts, 0,
		(size_t)fractal->size * sizeof(*fractal->sample_counts));
	leaves = fractal->buddha->proposal_leaves;
	index = 0;
	while (index < fractal->buddha->proposal_leaf_count)
	{
		cell = leaves[index].cell;
		if ((size_t)cell < (size_t)fractal->size
			&& fractal->sample_counts[cell] == 0)
			fractal->sample_counts[cell] = (uint32_t)index + 1;
		index++;
	}
}

static bool	prepare_proposal(t_fractal *fractal, t_buddha_proposal *proposal)
{
	double	available;
	size_t	budget;

	if (!fractal->buddha->proposal_mixture
		|| fractal->buddha->proposal_leaf_count == 0)
		return (false);
	memset(proposal, 0, sizeof(*proposal));
	proposal->comps = set_comps(fractal, false);
	proposal->channel = fractal->hist_num;
	proposal->leaf_total = fractal->buddha->proposal_leaf_total[
			proposal->channel];
	proposal_window(proposal->comps, proposal->window);
	proposal->window_area = (proposal->window[1] - proposal->window[0])
		* (proposal->window[3] - proposal->window[2]);
	proposal->global_mix = fractal->buddha->proposal_global;
	if (proposal->window_area > 0.0)
		proposal->window_mix = fractal->buddha->proposal_window;
	if (proposal->leaf_total > 0.0)
		proposal->leaf_mix = 1.0 - fractal->buddha->proposal_global
			- fractal->buddha->proposal_window;
	available = proposal->global_mix + proposal->window_mix
		+ proposal->leaf_mix;
	proposal->global_mix /= available;
	proposal->window_mix /= available;
	proposal->leaf_mix /= available;
	budget = buddha_sample_budget(fractal);
	proposal->global_samples = (size_t)llround(
			proposal->global_mix * (double)budget);
	if (proposal->global_samples == 0)
		proposal->global_samples = 1;
	proposal->window_samples = (size_t)llround(
			proposal->window_mix * (double)budget);
	if (proposal->global_samples + proposal->window_samples > budget)
		proposal->window_samples = budget - proposal->global_samples;
	proposal->leaf_samples = budget - proposal->global_samples
		- proposal->window_samples;
	proposal->global_mix = (double)proposal->global_samples / (double)budget;
	proposal->window_mix = (double)proposal->window_samples / (double)budget;
	proposal->leaf_mix = (double)proposal->leaf_samples / (double)budget;
	prepare_leaf_lookup(fractal);
	if (buddha_profile_enabled())
		printf("[buddha] mixture channel=%d global=%.2f%% window=%.2f%% "
			"guided=%.2f%% leaves=%zu window-area=%.8f%%\n",
			proposal->channel, 100.0 * proposal->global_mix,
			100.0 * proposal->window_mix, 100.0 * proposal->leaf_mix,
			fractal->buddha->proposal_leaf_count,
			100.0 * proposal->window_area / (double)fractal->size);
	return (true);
}

static size_t	select_leaf(t_buddha *buddha, int channel, double target)
{
	size_t	low;
	size_t	high;
	size_t	middle;

	low = 0;
	high = buddha->proposal_leaf_count;
	while (low < high)
	{
		middle = low + (high - low) / 2;
		if (buddha->proposal_leaves[middle].cumulative[channel] <= target)
			low = middle + 1;
		else
			high = middle;
	}
	if (low == buddha->proposal_leaf_count)
		low--;
	return (low);
}

static double	leaf_mass(t_buddha *buddha, size_t index, int channel)
{
	double	previous;

	previous = 0.0;
	if (index > 0)
		previous = buddha->proposal_leaves[index - 1].cumulative[channel];
	return (buddha->proposal_leaves[index].cumulative[channel] - previous);
}

static bool	inside(double value, double low, double high)
{
	return (value >= low && value < high);
}

static double	leaf_density_at(t_fractal *fractal,
		t_buddha_proposal *proposal, double pixel_x, double pixel_y)
{
	t_buddha_leaf	*leaf;
	double			density;
	double			mass;
	size_t			index;
	uint32_t		lookup;
	uint32_t		cell;

	if (proposal->leaf_mix == 0.0 || pixel_x < 0.0 || pixel_y < 0.0
		|| pixel_x >= (double)fractal->width
		|| pixel_y >= (double)fractal->height)
		return (0.0);
	cell = (uint32_t)pixel_y * (uint32_t)fractal->width + (uint32_t)pixel_x;
	lookup = fractal->sample_counts[cell];
	if (lookup == 0)
		return (0.0);
	index = (size_t)lookup - 1;
	density = 0.0;
	while (index < fractal->buddha->proposal_leaf_count
		&& fractal->buddha->proposal_leaves[index].cell == cell)
	{
		leaf = &fractal->buddha->proposal_leaves[index];
		if (inside(pixel_x, (double)(cell % (uint32_t)fractal->width)
				+ leaf->u0, (double)(cell % (uint32_t)fractal->width)
				+ leaf->u0 + leaf->size)
			&& inside(pixel_y, (double)(cell / (uint32_t)fractal->width)
				+ leaf->v0, (double)(cell / (uint32_t)fractal->width)
				+ leaf->v0 + leaf->size))
		{
			mass = leaf_mass(fractal->buddha, index, proposal->channel);
			density += mass / proposal->leaf_total
				/ ((double)leaf->size * (double)leaf->size);
		}
		index++;
	}
	return (density);
}

static double	proposal_density(t_fractal *fractal,
		t_buddha_proposal *proposal, double pixel_x, double pixel_y)
{
	double	density;

	density = proposal->global_mix / (double)fractal->size;
	if (proposal->window_mix > 0.0
		&& inside(pixel_x, proposal->window[0], proposal->window[1])
		&& inside(pixel_y, proposal->window[2], proposal->window[3]))
		density += proposal->window_mix / proposal->window_area;
	density += proposal->leaf_mix * leaf_density_at(fractal, proposal,
			pixel_x, pixel_y);
	return (density);
}

static void	draw_global_sample(t_fractal *fractal, Xoro128 *rng,
		size_t ordinal, size_t offset, double point[2])
{
	size_t	cell;

	cell = (ordinal + offset) % (size_t)fractal->size;
	point[0] = (double)(cell % (size_t)fractal->width) + xoro128d(rng);
	point[1] = (double)(cell / (size_t)fractal->width) + xoro128d(rng);
}

static void	draw_window_sample(t_buddha_proposal *proposal, Xoro128 *rng,
		double point[2])
{
	point[0] = proposal->window[0] + xoro128d(rng)
		* (proposal->window[1] - proposal->window[0]);
	point[1] = proposal->window[2] + xoro128d(rng)
		* (proposal->window[3] - proposal->window[2]);
}

static void	draw_leaf_sample(t_fractal *fractal,
		t_buddha_proposal *proposal, Xoro128 *rng, double point[2])
{
	t_buddha_leaf	*leaf;
	size_t			index;
	uint32_t		cell;

	index = select_leaf(fractal->buddha, proposal->channel,
			xoro128d(rng) * proposal->leaf_total);
	leaf = &fractal->buddha->proposal_leaves[index];
	cell = leaf->cell;
	point[0] = (double)(cell % (uint32_t)fractal->width) + leaf->u0
		+ xoro128d(rng) * leaf->size;
	point[1] = (double)(cell / (uint32_t)fractal->width) + leaf->v0
		+ xoro128d(rng) * leaf->size;
}

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

static void	*buddha_set_proposal(void *argument)
{
	t_piece				*piece;
	t_buddha_proposal	*proposal;
	t_comps				comps;
	t_complex			c;
	t_complex			orbit_cache[BUDDHA_ORBIT_CACHE_POINTS];
	t_complex			*cache;
	double				point[2];
	double				density;
	size_t				start;
	size_t				end;
	size_t				ordinal;
	size_t				offset;
	size_t				count;
	int					component;
	int					workers;

	piece = (t_piece *)argument;
	proposal = (t_buddha_proposal *)piece->proposal;
	comps = proposal->comps;
	comps.density_flat = piece->fractal->worker_histograms[piece->id];
	cache = NULL;
	if (piece->fractal->buddha->orbit_cache && comps.square_formula
		&& comps.b_max_i <= BUDDHA_ORBIT_CACHE_POINTS)
		cache = orbit_cache;
	workers = piece->fractal->worker_histogram_count;
	offset = (size_t)(0xFABDECAFULL + (uint64_t)piece->buffer
			* 0x9E3779B97F4A7C15ULL) % (size_t)piece->fractal->size;
	component = -1;
	while (++component < 3)
	{
		count = ((size_t [3]){proposal->global_samples,
				proposal->window_samples, proposal->leaf_samples})[component];
		start = (size_t)piece->id * count / (size_t)workers;
		end = (size_t)(piece->id + 1) * count / (size_t)workers;
		ordinal = start;
		while (ordinal < end)
		{
			if (component == 0)
				draw_global_sample(piece->fractal, &piece->rng,
					ordinal, offset, point);
			else if (component == 1)
				draw_window_sample(proposal, &piece->rng, point);
			else
				draw_leaf_sample(piece->fractal, proposal, &piece->rng, point);
			density = proposal_density(piece->fractal, proposal,
					point[0], point[1]);
			c = buddha_sample_point(comps, point[0], point[1]);
			buddha_iteration(c, 1.0 / density, comps, cache);
			ordinal++;
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

static void	start_proposal_piece(t_fractal *fractal, t_piece *piece,
		t_buddha_proposal *proposal, int worker, int buffer, uint64_t seed)
{
	memset(piece, 0, sizeof(*piece));
	piece->id = worker;
	piece->fractal = fractal;
	piece->proposal = proposal;
	piece->buffer = buffer;
	sxoro128(&piece->rng, seed + (uint64_t)worker * 142857ULL);
	if (pthread_create(&fractal->threads[worker], NULL,
			buddha_set_proposal, piece) != 0)
		thread_error(fractal, worker);
}

static void	run_proposal_buffer(t_fractal *fractal, int buffer,
		t_buddha_proposal *proposal)
{
	t_piece		pieces[fractal->worker_histogram_count];
	uint64_t	seed;
	int			worker;

	seed = 0xFABDECAF
		+ (uint64_t)buffer * 0x9E3779B97F4A7C15ULL;
	buddha_clear_worker_histograms(fractal);
	worker = -1;
	while (++worker < fractal->worker_histogram_count)
		start_proposal_piece(fractal, &pieces[worker], proposal,
			worker, buffer, seed);
	join_threads(fractal->threads, fractal->worker_histogram_count, 1);
	buddha_reduce_worker_histograms(fractal, buffer);
}

void	fast_buddha(t_fractal *fractal)
{
	t_buddha_proposal	proposal;
	bool				mixture;
	int					buffer;
	int					ends[fractal->worker_histogram_count];

	mixture = prepare_proposal(fractal, &proposal);
	if (!mixture)
		build_work_ends(fractal, ends);
	buffer = -1;
	while (++buffer < fractal->buffs)
	{
		if (mixture)
			run_proposal_buffer(fractal, buffer, &proposal);
		else
			run_fast_buffer(fractal, buffer, ends);
	}
}
