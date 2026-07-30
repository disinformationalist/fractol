#include "fractol.h"

static t_buddha_channel	channel_plan(t_buddha *b, int histogram)
{
	if (histogram == 0)
		return ((t_buddha_channel){0, b->min1, b->max1});
	if (histogram == 1)
		return ((t_buddha_channel){1, b->min2, b->max2});
	return ((t_buddha_channel){2, b->min3, b->max3});
}

static void	apply_channel(t_fractal *fractal, t_buddha_channel channel)
{
	fractal->hist_num = channel.histogram;
	fractal->b_min_i = channel.min_iter;
	fractal->b_max_i = channel.max_iter;
}

static void	print_channel_time(char *label, long start)
{
	printf(MAGENTA"%s: %f seconds\n"RESET, label,
		(double)(get_time() - start) / 1000.0);
}

static void	build_channel_importance(t_fractal *fractal,
		t_buddha_channel channel, bool map_ready)
{
	if (!fractal->buddha->fast)
		return ;
	if (!map_ready)
	{
		zero_matrix(fractal->densities[channel.histogram],
			fractal->width, fractal->height);
		printf(MAGENTA"Mapping channel: "BLUE"%d ...\n"RESET,
			channel.histogram);
		buddha_map(fractal);
		buddha_importance_capture(fractal, channel.histogram);
	}
	build_importance_map(fractal, fractal->densities[channel.histogram]);
}

static void	sample_channel(t_fractal *fractal, t_buddha_channel channel)
{
	zero_matrix(fractal->densities[channel.histogram],
		fractal->width, fractal->height);
	if (fractal->buddha->fast)
		fast_buddha(fractal);
	else
		buddha(fractal);
}

static void	finalize_channel_buffers(t_fractal *fractal,
		t_buddha_channel channel)
{
	if (fractal->buffs > 1)
		combine_buff_set_var(fractal->densities, channel.histogram,
			fractal->buffs, fractal->width, fractal->height);
}

static void	render_channel(t_fractal *fractal, t_buddha_channel channel,
		bool map_ready)
{
	long	start;
	long	phase_start;

	if (channel.max_iter <= 0)
		return ;
	start = get_time();
	apply_channel(fractal, channel);
	phase_start = get_time();
	build_channel_importance(fractal, channel, map_ready);
	if (fractal->buddha->fast && !map_ready)
		print_channel_time("Map channel time  ", phase_start);
	phase_start = get_time();
	printf(MAGENTA"Running channel: "BLUE"%d ...\n"RESET,
		channel.histogram);
	sample_channel(fractal, channel);
	print_channel_time("Run channel time  ", phase_start);
	phase_start = get_time();
	finalize_channel_buffers(fractal, channel);
	buddha_profile_phase("buffer statistics", phase_start);
	if (fractal->buffs <= 1)
	{
		phase_start = get_time();
		buddha_set_channel_statistics(fractal, channel.histogram);
		buddha_profile_density(fractal, channel.histogram);
		buddha_profile_phase("tone statistics", phase_start);
	}
	printf(MAGENTA"Channel: "BLUE"%d Complete\n"RESET, channel.histogram);
	print_channel_time("Total channel time", start);
	printf("\n");
}

void	buddha_render_channel(t_fractal *fractal, int min_iter,
		int max_iter, int histogram)
{
	if (histogram == 0)
		buddha_nlm_reset(fractal);
	render_channel(fractal,
		(t_buddha_channel){histogram, min_iter, max_iter}, false);
}

static void	build_all_importance_maps(t_fractal *fractal)
{
	long	start;
	int		channel;

	if (!fractal->buddha->fast)
		return ;
	start = get_time();
	printf(MAGENTA"Mapping all channels "
		"(scale "BLUE"%d"RESET MAGENTA") ...\n"RESET,
		buddha_map_scale(fractal));
	channel = -1;
	while (++channel < 3)
		zero_matrix(fractal->densities[channel],
			fractal->width, fractal->height);
	buddha_map_all(fractal);
	channel = -1;
	while (++channel < 3)
		buddha_importance_capture(fractal, channel);
	print_channel_time("All-channel map time", start);
}

static void	finalize_multibuffer(t_fractal *fractal)
{
	long	start;
	int		histogram;

	start = get_time();
	if (fractal->buddha->nlm_enabled && buddha_nlm(fractal) != 0)
		fprintf(stderr,
			"Buddha NLM skipped: workspace or temporary allocation failed\n");
	buddha_profile_phase("nlm", start);
	histogram = -1;
	while (++histogram < 3)
	{
		buddha_set_channel_statistics(fractal, histogram);
		buddha_profile_density(fractal, histogram);
	}
}

void	render_buddha(t_fractal *fractal)
{
	long	start;
	int		histogram;

	start = get_time();
	if (buddha_profile_enabled())
		printf("[buddha] orbit=%s cache=%s<=%d interior=%s map=%s "
			"configured=%d effective=%d metric=%s buffers=%d "
			"samples-per-cell=%.3f allocation=flat-u32:%.2fMiB\n",
			(char *[2]){"generic", "square-specialized"}
			[fractal->buddha->square_specialized
				&& fractal->complex_f == &square_complex],
			(char *[2]){"off", "short-channel"}
			[fractal->buddha->orbit_cache],
			BUDDHA_ORBIT_CACHE_POINTS,
			(char *[2]){"off", "analytic"}
			[fractal->buddha->interior_rejection],
			(char *[2]){"fixed", "adaptive"}
			[fractal->buddha->map_adaptive],
			ft_round(fractal->buddha->map_n), buddha_map_scale(fractal),
			(char *[2]){"mean", "rms"}[fractal->buddha->importance_rms],
			fractal->buffs, fractal->buddha->n * fractal->buddha->n,
			(double)fractal->size * sizeof(*fractal->sample_counts)
			/ (1024.0 * 1024.0));
	buddha_nlm_reset(fractal);
	buddha_importance_begin(fractal);
	build_all_importance_maps(fractal);
	histogram = -1;
	while (++histogram < 3)
		render_channel(fractal,
			channel_plan(fractal->buddha, histogram),
			fractal->buddha->fast);
	if (fractal->buffs > 1)
		finalize_multibuffer(fractal);
	if (fractal->buddha->normalization == BUDDHA_NORM_LINKED_PERCENTILE)
		buddha_update_white_points(fractal);
	color_buddha(fractal);
	print_times(start, get_time(), "RENDER COMPLETE\n",
		"Total render time : "GREEN"%f"RESET" seconds\n", BOLD_BLUE);
}
