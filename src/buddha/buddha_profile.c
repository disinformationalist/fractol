#include "fractol.h"

bool	buddha_profile_enabled(void)
{
	char	*value;

	value = getenv("FRACTOL_BUDDHA_PROFILE");
	return (value != NULL && value[0] != '\0'
		&& !(value[0] == '0' && value[1] == '\0'));
}

static int	compare_u32(const void *left, const void *right)
{
	uint32_t	a;
	uint32_t	b;

	a = *(const uint32_t *)left;
	b = *(const uint32_t *)right;
	if (a < b)
		return (-1);
	return (a > b);
}

static void	importance_window_bounds(t_comps comps, double bounds[4])
{
	double	first;
	double	second;

	first = comps.move_x + comps.x_cmin * comps.inv_zoom;
	second = comps.move_x + comps.x_cmax * comps.inv_zoom;
	bounds[0] = fmin(first, second);
	bounds[1] = fmax(first, second);
	first = -comps.move_y + comps.y_cmin * comps.inv_zoom;
	second = -comps.move_y + comps.y_cmax * comps.inv_zoom;
	bounds[2] = fmin(first, second);
	bounds[3] = fmax(first, second);
}

static bool	importance_cell_in_window(t_comps comps, size_t index,
		double bounds[4])
{
	t_complex	c;
	int			x;
	int			y;

	x = (int)(index % (size_t)comps.width);
	y = (int)(index / (size_t)comps.width);
	c = buddha_sample_point(comps, (double)x + 0.5, (double)y + 0.5);
	return (c.x >= bounds[0] && c.x <= bounds[1]
		&& c.y >= bounds[2] && c.y <= bounds[3]);
}

static void	importance_top_shares(t_fractal *fractal, size_t budget,
		double shares[2])
{
	uint32_t	*sorted;
	long double	top_one;
	long double	top_ten;
	size_t		one_count;
	size_t		ten_count;
	size_t		index;

	shares[0] = -1.0;
	shares[1] = -1.0;
	sorted = malloc((size_t)fractal->size * sizeof(*sorted));
	if (!sorted || budget == 0)
		return (free(sorted));
	memcpy(sorted, fractal->sample_counts,
		(size_t)fractal->size * sizeof(*sorted));
	qsort(sorted, (size_t)fractal->size, sizeof(*sorted), compare_u32);
	one_count = ((size_t)fractal->size + 99) / 100;
	ten_count = ((size_t)fractal->size + 9) / 10;
	top_one = 0.0L;
	top_ten = 0.0L;
	index = 0;
	while (index < ten_count)
	{
		top_ten += sorted[(size_t)fractal->size - 1 - index];
		if (index < one_count)
			top_one += sorted[(size_t)fractal->size - 1 - index];
		index++;
	}
	shares[0] = (double)(top_one / (long double)budget);
	shares[1] = (double)(top_ten / (long double)budget);
	free(sorted);
}

static void	print_pilot_profile(t_fractal *fractal, int channel)
{
	t_buddha_pilot_stats	*stats;
	double					eligible;
	double					useful;
	double					hits_per_useful;
	double					score_per_useful;
	double					score_per_hit;

	stats = &fractal->buddha->pilot_stats;
	eligible = 0.0;
	useful = 0.0;
	hits_per_useful = 0.0;
	score_per_useful = 0.0;
	score_per_hit = 0.0;
	if (stats->samples[channel] > 0)
	{
		eligible = (double)stats->eligible[channel]
			/ (double)stats->samples[channel];
		useful = (double)stats->useful[channel]
			/ (double)stats->samples[channel];
	}
	if (stats->useful[channel] > 0)
	{
		hits_per_useful = (double)stats->visible_hits[channel]
			/ (double)stats->useful[channel];
		score_per_useful = (double)(stats->score_sum[channel]
			/ (long double)stats->useful[channel]);
	}
	if (stats->visible_hits[channel] > 0)
		score_per_hit = (double)(stats->score_sum[channel]
			/ (long double)stats->visible_hits[channel]);
	printf("[buddha] pilot channel=%d samples=%llu eligible=%.6f%% "
		"useful=%.6f%% visible-hits/useful=%.3f\n", channel,
		(unsigned long long)stats->samples[channel],
		100.0 * eligible, 100.0 * useful, hits_per_useful);
	printf("[buddha] pilot channel=%d score=%s score/useful=%.3f "
		"score/hit=%.6f\n", channel,
		(char *[2]){"visible-hits", "viewport-tile-l2"}
		[fractal->buddha->importance_recurrence],
		score_per_useful, score_per_hit);
	if (stats->adaptive_samples[channel] > 0)
	{
		useful = (double)stats->adaptive_useful[channel]
			/ (double)stats->adaptive_samples[channel];
		hits_per_useful = 0.0;
		score_per_hit = 0.0;
		if (stats->adaptive_useful[channel] > 0)
			hits_per_useful = (double)stats->adaptive_visible_hits[channel]
				/ (double)stats->adaptive_useful[channel];
		if (stats->adaptive_visible_hits[channel] > 0)
			score_per_hit = (double)(stats->adaptive_score_sum[channel]
				/ (long double)stats->adaptive_visible_hits[channel]);
		printf("[buddha] refine channel=%d cells=%llu samples=%llu "
			"useful=%.6f%% visible-hits/useful=%.3f score/hit=%.6f\n",
			channel, (unsigned long long)stats->refined_cells,
			(unsigned long long)stats->adaptive_samples[channel],
			100.0 * useful, hits_per_useful, score_per_hit);
	}
}

void	buddha_profile_importance(t_fractal *fractal, double **importance,
		int channel)
{
	t_comps		comps;
	long double	count_squares;
	size_t		budget;
	size_t		window_budget;
	size_t		map_support;
	size_t		allocation_support;
	size_t		index;
	uint32_t	count;
	uint32_t	minimum;
	uint32_t	maximum;
	double		bounds[4];
	double		shares[2];
	double		effective_cells;
	double		max_relative_weight;
	long		start;

	if (!buddha_profile_enabled())
		return ;
	start = get_time();
	comps = set_comps(fractal, false);
	importance_window_bounds(comps, bounds);
	budget = 0;
	window_budget = 0;
	map_support = 0;
	allocation_support = 0;
	count_squares = 0.0L;
	minimum = UINT32_MAX;
	maximum = 0;
	index = 0;
	while (index < (size_t)fractal->size)
	{
		if (isfinite(importance[index / (size_t)fractal->width]
				[index % (size_t)fractal->width])
			&& importance[index / (size_t)fractal->width]
				[index % (size_t)fractal->width] > 0.0)
			map_support++;
		count = fractal->sample_counts[index];
		budget += (size_t)count;
		count_squares += (long double)count * (long double)count;
		if (count > 0)
		{
			allocation_support++;
			if (count < minimum)
				minimum = count;
			if (count > maximum)
				maximum = count;
			if (importance_cell_in_window(comps, index, bounds))
				window_budget += (size_t)count;
		}
		index++;
	}
	if (minimum == UINT32_MAX)
		minimum = 0;
	effective_cells = 0.0;
	max_relative_weight = 0.0;
	if (count_squares > 0.0L)
		effective_cells = (double)((long double)budget
				* (long double)budget / count_squares);
	if (minimum > 0 && fractal->size > 0)
		max_relative_weight = (double)budget
			/ ((double)minimum * (double)fractal->size);
	importance_top_shares(fractal, budget, shares);
	print_pilot_profile(fractal, channel);
	printf("[buddha] proposal channel=%d map-support=%.4f%% "
		"allocated=%.4f%% window-mass=%.4f%% top1=%.4f%% top10=%.4f%%\n",
		channel, 100.0 * (double)map_support / (double)fractal->size,
		100.0 * (double)allocation_support / (double)fractal->size,
		100.0 * (double)window_budget / (double)budget,
		100.0 * shares[0], 100.0 * shares[1]);
	printf("[buddha] proposal channel=%d effective-cells=%.1f (%.4f%%) "
		"count=[%u,%u] max-relative-weight=%.3f\n", channel,
		effective_cells, 100.0 * effective_cells / (double)fractal->size,
		minimum, maximum, max_relative_weight);
	buddha_profile_phase("importance metrics", start);
}

void	buddha_profile_phase(char *name, long start)
{
	if (buddha_profile_enabled())
		printf("[buddha] %-18s %8.3f s\n", name,
			(double)(get_time() - start) / 1000.0);
}

static void	get_tone_params(t_buddha *b, int histogram,
		double values[6])
{
	if (histogram == 0)
	{
		values[0] = b->white_b;
		values[1] = b->high_b;
		values[2] = b->bpow;
		values[3] = b->edge0_b;
		values[4] = b->edge1_b;
		values[5] = b->exposure_b;
	}
	else if (histogram == 1)
	{
		values[0] = b->white_g;
		values[1] = b->high_g;
		values[2] = b->gpow;
		values[3] = b->edge0_g;
		values[4] = b->edge1_g;
		values[5] = b->exposure_g;
	}
	else
	{
		values[0] = b->white_r;
		values[1] = b->high_r;
		values[2] = b->rpow;
		values[3] = b->edge0_r;
		values[4] = b->edge1_r;
		values[5] = b->exposure_r;
	}
}

static double	profile_tone_value(t_fractal *fractal, int histogram,
		double density, double values[6])
{
	double	mapped;
	double	exposure;
	unsigned int	color;

	if (fractal->buddha->negative_mode == BUDDHA_NEGATIVE_HYBRID)
	{
		if (density <= 0.0 || values[0] <= 0.0)
			return (0.0);
		mapped = pow(fmin(1.0, density / values[0]),
				fabs(fractal->buddha->tone_default.power[histogram]));
		exposure = fractal->buddha->global_exposure;
		if (fractal->buddha->tone_default.exposure[histogram] > 0.0)
			exposure *= values[5]
				/ fractal->buddha->tone_default.exposure[histogram];
		mapped = fmin(1.0, mapped * exposure);
		if (fractal->buddha->smootherstep)
			mapped = buddha_smootherstep_value(
					values[3], values[4], mapped);
		return (mapped);
	}
	if (fractal->buddha->negative_mode == BUDDHA_NEGATIVE_SPILL
		&& values[2] < 0.0)
	{
		exposure = fractal->buddha->global_exposure;
		if (fractal->buddha->tone_default.exposure[histogram] > 0.0)
			exposure *= values[5]
				/ fractal->buddha->tone_default.exposure[histogram];
		color = buddha_legacy_spill_color(density, values[1], values[2],
				exposure, (unsigned int)histogram * 8);
		mapped = (double)((color >> (histogram * 8)) & 0xFFu) / 255.0;
		if (fractal->buddha->smootherstep)
			mapped = buddha_smootherstep_value(
					values[3], values[4], mapped);
		return (mapped);
	}
	if (fractal->buddha->smootherstep)
		return (buddha_tone_value(density, values[0], values[2],
				values[3], values[4],
				values[5] * fractal->buddha->global_exposure));
	mapped = buddha_curve_value(density, values[0], values[2])
		* values[5] * fractal->buddha->global_exposure;
	if (mapped < 0.0)
		return (0.0);
	if (mapped > 1.0)
		return (1.0);
	return (mapped);
}

void	buddha_profile_density(t_fractal *fractal, int histogram)
{
	double		values[6];
	double		density;
	double		mapped;
	double		sum;
	double		high;
	long double	checksum;
	size_t		counts[4];
	int			x;
	int			y;

	if (!buddha_profile_enabled())
		return ;
	get_tone_params(fractal->buddha, histogram, values);
	sum = 0.0;
	high = 0.0;
	checksum = 0.0L;
	memset(counts, 0, sizeof(counts));
	y = -1;
	while (++y < fractal->height)
	{
		x = -1;
		while (++x < fractal->width)
		{
			density = fractal->densities[histogram][y][x];
			sum += density;
			if (density > high)
				high = density;
			if (density != 0.0)
				counts[0]++;
			mapped = profile_tone_value(fractal, histogram, density, values);
			if (mapped < 0.5 / 255.0)
				counts[1]++;
			if (mapped >= 229.5 / 255.0)
				counts[2]++;
			if (mapped >= 254.5 / 255.0)
				counts[3]++;
			checksum += (long double)density
				* (long double)(1 + x + y * fractal->width);
		}
	}
	printf("[buddha] histogram=%d sum=%.17g high=%.17g "
		"checksum=%.21Lg nonzero=%zu\n", histogram, sum, high,
		checksum, counts[0]);
	printf("[buddha] white=%.17g percentile=%.5f norm=%s negative=%s power=%.3f "
		"edges=[%.3f,%.3f] exposure=%.3f global=%.3f "
		"black=%.3f%% bright=%.3f%% saturated=%.3f%%\n",
		values[0], fractal->buddha->white_percentile,
		buddha_normalization_name(fractal->buddha->normalization),
		buddha_negative_mode_name(fractal->buddha->negative_mode), values[2],
		values[3], values[4], values[5],
		fractal->buddha->global_exposure,
		100.0 * (double)counts[1] / (double)fractal->size,
		100.0 * (double)counts[2] / (double)fractal->size,
		100.0 * (double)counts[3] / (double)fractal->size);
}
