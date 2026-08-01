/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:08:42 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:08:44 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

static double	get_env_double(char *name, double default_value,
		double minimum, double maximum);

void	set_vals(t_fractal *fractal, int min_1, int min_2, int min_3, int max_1, int max_2, int max_3, f complex_f)
{
	fractal->buddha->min1 = min_1;
	fractal->buddha->min2 = min_2;
	fractal->buddha->min3 = min_3;
	fractal->buddha->max1 = max_1;
	fractal->buddha->max2 = max_2;
	fractal->buddha->max3 = max_3;
	fractal->complex_f = complex_f;
}

void	set_edge_vals(t_buddha *b, double e0b, double e0g, double e0r, double e1b, double e1g, double e1r)
{
	b->edge0_b = e0b;
	b->edge0_g = e0g;
	b->edge0_r = e0r;
	b->edge1_b = e1b;
	b->edge1_g = e1g;
	b->edge1_r = e1r;
}

void	set_powers(t_buddha *bud, double b, double g, double r)
{
	bud->bpow = b;
	bud->gpow = g;
	bud->rpow = r;
}

static double	get_white_percentile(void)
{
	char	*value;
	char	*end;
	double	percentile;

	value = getenv("FRACTOL_BUDDHA_WHITE_PERCENTILE");
	if (!value || !value[0])
		return (BUDDHA_WHITE_PERCENTILE);
	percentile = strtod(value, &end);
	if (percentile > 1.0 && percentile <= 100.0)
		percentile /= 100.0;
	if (end == value || *end != '\0'
		|| percentile <= 0.0 || percentile > 1.0)
	{
		fprintf(stderr, "Invalid Buddha white percentile '%s'; using %.4f\n",
			value, BUDDHA_WHITE_PERCENTILE);
		return (BUDDHA_WHITE_PERCENTILE);
	}
	return (percentile);
}

static bool	get_nlm_enabled(int buffs)
{
	char	*value;

	value = getenv("FRACTOL_BUDDHA_NLM");
	if (value && (!strcmp(value, "0") || !strcmp(value, "false")
			|| !strcmp(value, "off")))
		return (false);
	return (buffs > 1);
}

static bool	get_importance_enabled(bool cli_enabled)
{
	char	*value;

	if (cli_enabled)
		return (true);
	value = getenv("FRACTOL_BUDDHA_IMPORTANCE");
	if (!value || !value[0] || !strcmp(value, "0")
		|| !strcmp(value, "false") || !strcmp(value, "off"))
		return (false);
	return (true);
}

static bool	get_feature_enabled(char *name, bool default_value)
{
	char	*value;

	value = getenv(name);
	if (!value || !value[0])
		return (default_value);
	if (!strcmp(value, "0") || !strcmp(value, "false")
		|| !strcmp(value, "off"))
		return (false);
	if (!strcmp(value, "1") || !strcmp(value, "true")
		|| !strcmp(value, "on"))
		return (true);
	fprintf(stderr, "Invalid %s value '%s'; using %s\n", name, value,
		(char *[2]){"off", "on"}[default_value]);
	return (default_value);
}

static void	get_importance_refinement(t_buddha *b)
{
	char	*value;

	b->importance_refinement = true;
	b->importance_refinement_force = false;
	value = getenv("FRACTOL_BUDDHA_MAP_REFINEMENT");
	if (!value || !value[0] || !strcmp(value, "auto"))
		return ;
	if (!strcmp(value, "0") || !strcmp(value, "false")
		|| !strcmp(value, "off"))
		b->importance_refinement = false;
	else if (!strcmp(value, "1") || !strcmp(value, "true")
		|| !strcmp(value, "on") || !strcmp(value, "force"))
		b->importance_refinement_force = true;
	else
		fprintf(stderr, "Invalid FRACTOL_BUDDHA_MAP_REFINEMENT '%s'; "
			"using auto\n", value);
}

static void	get_importance_proposal(t_buddha *b)
{
	char	*value;

	b->proposal_mixture = true;
	value = getenv("FRACTOL_BUDDHA_PROPOSAL");
	if (value && (!strcmp(value, "0") || !strcmp(value, "false")
			|| !strcmp(value, "off") || !strcmp(value, "legacy")))
		b->proposal_mixture = false;
	else if (value && strcmp(value, "1") && strcmp(value, "true")
		&& strcmp(value, "on") && strcmp(value, "mixture"))
		fprintf(stderr, "Invalid FRACTOL_BUDDHA_PROPOSAL '%s'; "
			"using mixture\n", value);
	b->proposal_global = get_env_double(
			"FRACTOL_BUDDHA_PROPOSAL_GLOBAL", 0.15, 0.01, 0.90);
	b->proposal_window = get_env_double(
			"FRACTOL_BUDDHA_PROPOSAL_WINDOW", 0.55, 0.0, 0.90);
	if (b->proposal_global + b->proposal_window >= 1.0)
	{
		fprintf(stderr, "Buddha proposal global + window must be below 1; "
			"using 0.15 + 0.55\n");
		b->proposal_global = 0.15;
		b->proposal_window = 0.55;
	}
}

static double	get_env_double(char *name, double default_value,
		double minimum, double maximum)
{
	char	*value;
	char	*end;
	double	number;

	value = getenv(name);
	if (!value || !value[0])
		return (default_value);
	number = strtod(value, &end);
	if (end == value || *end != '\0' || number < minimum || number > maximum)
	{
		fprintf(stderr, "Invalid %s value '%s'; using %.3f\n",
			name, value, default_value);
		return (default_value);
	}
	return (number);
}

static int	get_buffer_count(int default_value)
{
	char	*value;
	char	*end;
	long	count;

	value = getenv("FRACTOL_BUDDHA_BUFFERS");
	if (!value || !value[0])
		return (default_value);
	count = strtol(value, &end, 10);
	if (end == value || *end != '\0' || count < 1 || count > 3)
	{
		fprintf(stderr, "Invalid FRACTOL_BUDDHA_BUFFERS '%s'; using %d\n",
			value, default_value);
		return (default_value);
	}
	return ((int)count);
}

static t_buddha_normalization	get_buddha_normalization(void)
{
	char	*value;

	value = getenv("FRACTOL_BUDDHA_NORMALIZATION");
	if (!value || !value[0] || !strcmp(value, "channel")
		|| !strcmp(value, "percentile"))
		return (BUDDHA_NORM_CHANNEL_PERCENTILE);
	if (!strcmp(value, "linked"))
		return (BUDDHA_NORM_LINKED_PERCENTILE);
	if (!strcmp(value, "max") || !strcmp(value, "legacy"))
		return (BUDDHA_NORM_LEGACY_MAX);
	fprintf(stderr, "Invalid FRACTOL_BUDDHA_NORMALIZATION '%s'; "
		"using channel percentiles\n", value);
	return (BUDDHA_NORM_CHANNEL_PERCENTILE);
}

static t_buddha_negative_mode	get_buddha_negative_mode(void)
{
	char	*value;

	value = getenv("FRACTOL_BUDDHA_NEGATIVE_MODE");
	if (!value || !value[0] || !strcmp(value, "inverse"))
		return (BUDDHA_NEGATIVE_INVERSE);
	if (!strcmp(value, "spill"))
		return (BUDDHA_NEGATIVE_SPILL);
	if (!strcmp(value, "hybrid"))
		return (BUDDHA_NEGATIVE_HYBRID);
	fprintf(stderr, "Invalid FRACTOL_BUDDHA_NEGATIVE_MODE '%s'; "
		"using inverse\n", value);
	return (BUDDHA_NEGATIVE_INVERSE);
}

static void	init_buddha_nlm(t_fractal *fractal)
{
	t_buddha	*b;
	double		default_kc;
	int			default_patch;

	b = fractal->buddha;
	default_patch = 2;
	default_kc = 1.0;
	if (b->type == BUDDHA1)
	{
		default_patch = 2;
		default_kc = 0.75;
	}
	b->nlm_enabled = get_nlm_enabled(fractal->buffs);
	b->nlm_smooth_var = true;
	b->nlm_patch_radius = (int)get_env_double(
			"FRACTOL_BUDDHA_NLM_PATCH", default_patch, 0.0, 8.0);
	b->nlm_search_radius = (int)get_env_double(
			"FRACTOL_BUDDHA_NLM_SEARCH", 15.0, 0.0, 64.0);
	b->nlm_kc = get_env_double(
			"FRACTOL_BUDDHA_NLM_KC", default_kc, 0.05, 8.0);
	b->nlm_noise_scale = 1.0 / (double)fractal->buffs;
	b->nlm_noise_floor = 1.0 / (255.0 * 255.0);
	b->nlm_relative_variance_cap = get_env_double(
			"FRACTOL_BUDDHA_NLM_VARIANCE_CAP", 0.02, 0.0, 1.0);
	b->nlm_firefly_factor = get_env_double(
			"FRACTOL_BUDDHA_NLM_FIREFLY", 2.0, 0.0, 1024.0);
	if (b->nlm_firefly_factor > 0.0 && b->nlm_firefly_factor < 1.0)
	{
		fprintf(stderr, "FRACTOL_BUDDHA_NLM_FIREFLY is a neighborhood "
			"ratio; clamping %.6g to 1\n", b->nlm_firefly_factor);
		b->nlm_firefly_factor = 1.0;
	}
	b->nlm_r_weight = 1.0;
	b->nlm_g_weight = 1.0;
	b->nlm_b_weight = 1.0;
	b->nlm_variance_ready = false;
	b->nlm_filtered_ready = false;
	b->nlm_show_filtered = false;
}

static void	init_buddha_importance(t_buddha *b)
{
	char	*metric;
	char	*score;

	b->importance_enabled
		= get_importance_enabled(b->importance_enabled);
	b->importance_ready = false;
	b->importance_view = b->importance_enabled;
	b->importance_mode = BUDDHA_IMPORTANCE_RGB;
	b->importance_values = NULL;
	b->render_pixels = NULL;
	b->map_adaptive = get_feature_enabled(
			"FRACTOL_BUDDHA_MAP_ADAPTIVE", true);
	get_importance_refinement(b);
	get_importance_proposal(b);
	b->importance_rms = true;
	metric = getenv("FRACTOL_BUDDHA_IMPORTANCE_METRIC");
	if (metric && !strcmp(metric, "mean"))
		b->importance_rms = false;
	else if (metric && strcmp(metric, "rms"))
		fprintf(stderr, "Invalid FRACTOL_BUDDHA_IMPORTANCE_METRIC '%s'; "
			"using rms\n", metric);
	b->importance_recurrence = false;
	score = getenv("FRACTOL_BUDDHA_IMPORTANCE_SCORE");
	if (score && !strcmp(score, "footprint"))
		b->importance_recurrence = true;
	else if (score && strcmp(score, "hits"))
		fprintf(stderr, "Invalid FRACTOL_BUDDHA_IMPORTANCE_SCORE '%s'; "
			"using hits\n", score);
}

static void	init_buddha_one(t_fractal *fractal)
{
	t_buddha	*b;

	b = fractal->buddha;
	fractal->buffs = 3;
	b->smootherstep = true;
	set_powers(b, .41, .39, .34);
	set_edge_vals(b, .31, .29, .34, .59, .61, .61);
	set_vals(fractal, 0, 0, 0, 50, 500, 5000, &square_complex);
}

static void	init_buddha_two(t_fractal *fractal)
{
	t_buddha	*b;

	b = fractal->buddha;
	fractal->buffs = 3;
	b->smootherstep = true;
	set_powers(b, .48, .58, .53);
	set_edge_vals(b, .15, .0, .05, .55, .50, .6);
	set_vals(fractal, 0, 55, 55, 350, 500, 3500, &square_complex);
}

static void	init_lotus(t_fractal *fractal)
{
	t_buddha	*b;

	b = fractal->buddha;
	fractal->buffs = 1;
	fractal->name = "Lotus";
	set_powers(b, .5, .65, .65);
	set_edge_vals(b, .05, .05, .05, .6, .6, .6);
	set_vals(fractal, 20, 55, 100, 150, 200, 3000,
		&square_complex_conj);
}

static void	init_phoenix(t_fractal *fractal)
{
	t_buddha	*b;

	b = fractal->buddha;
	fractal->buffs = 1;
	fractal->name = "Pheonix";
	fractal->move_x = 0;
	set_powers(b, .45, .6, .5);
	set_edge_vals(b, .05, .05, .05, .6, .6, .6);
	set_vals(fractal, 12, 55, 55, 250, 600, 3000, &cube_ship);
}

static void	init_buddha_type(t_fractal *fractal)
{
	t_btype	type;

	type = fractal->buddha->type;
	fractal->buddha->smootherstep = false;
	if (type == BUDDHA1)
		init_buddha_one(fractal);
	else if (type == BUDDHA2)
		init_buddha_two(fractal);
	else if (type == LOTUS)
		init_lotus(fractal);
	else
		init_phoenix(fractal);
	if (type == BUDDHA1 || type == BUDDHA2)
		fractal->buffs = get_buffer_count(fractal->buffs);
}

void	init_buddha(t_fractal *fractal)
{
	t_buddha	*b;

	b = fractal->buddha;
	b->filter = false;
	b->flevel = 5;
	b->ftype = ADJUST;
	b->change = .05;
	b->tone_gain_mode = false;
	b->tone_extra_fine = false;
	init_buddha_importance(b);
	b->square_specialized = !get_feature_enabled(
			"FRACTOL_BUDDHA_GENERIC_ORBIT", false);
	b->orbit_cache = get_feature_enabled(
			"FRACTOL_BUDDHA_ORBIT_CACHE", true);
	b->interior_rejection = get_feature_enabled(
			"FRACTOL_BUDDHA_INTERIOR_REJECTION", true);
	b->white_percentile = get_white_percentile();
	b->normalization = get_buddha_normalization();
	b->global_exposure = get_env_double(
			"FRACTOL_BUDDHA_EXPOSURE", 1.0, 0.05, 20.0);
	b->negative_mode = get_buddha_negative_mode();
	b->hybrid_spill = get_env_double(
			"FRACTOL_BUDDHA_HYBRID_SPILL",
			BUDDHA_HYBRID_SPILL, 0.0, 1.0);
	b->hybrid_background = BUDDHA_HYBRID_BACKGROUND;
	b->hybrid_foreground = BUDDHA_HYBRID_FOREGROUND;
	b->white_b = 0.0;
	b->white_g = 0.0;
	b->white_r = 0.0;

	fractal->move_x = -.21;
	fractal->move_y = 0;
	fractal->zoom = 1.2;

	b->fast = true;
	b->n = 15;
	b->map_n = b->n - 1;
	init_buddha_type(fractal);
	buddha_apply_start_view(fractal);
	b->exposure_b = b->edge1_b;
	b->exposure_g = b->edge1_g;
	b->exposure_r = b->edge1_r;
	buddha_tone_store_defaults(b);
	init_buddha_nlm(fractal);
	fractal->histograms = fractal->buffs * BUDDHA_CHANNELS;
	if (b->nlm_enabled && fractal->histograms < BUDDHA_NLM_MATRICES)
		fractal->histograms = BUDDHA_NLM_MATRICES;
}

void	info_init(t_fractal *fractal)
{
	fractal->bound = 4;
	fractal->num_colors = 360;
	fractal->max_i = 80;
	fractal->move_x = 0.0;
	fractal->move_y = 0.0;
	fractal->zoom = 1.0;
	fractal->col_i = 0;
	fractal->toggle_color = 0;
	fractal->color_spectrum = 0;
	fractal->move_col_x = 0;
	fractal->move_col_y = 0;
	fractal->j_handle = 0;
	fractal->mouse_zoom = true;
	fractal->zoom_iter = 1;
	fractal->b_max_i = 0;
	fractal->supersample = 0;
	fractal->layer = 0;
	fractal->s_kernel = 3;
	fractal->species = 0;
	fractal->aspect = (double)fractal->height_orig / (double)fractal->width_orig;
	fractal->fdensity = NULL;
	fractal->pixels_xl = NULL;
	fractal->worker_histograms = NULL;
	fractal->worker_histogram_count = 0;
	fractal->densities = NULL;
	fractal->sample_counts = NULL;
	fractal->buddha_workspace_width = 0;
	fractal->buddha_workspace_height = 0;
	fractal->buddha_workspace_histograms = 0;
	fractal->buddha_workspace_workers = 0;
}

static void	events_init(t_fractal *fractal)
{
	mlx_hook(fractal->mlx_win, KeyPress, \
	KeyPressMask, key_handler, fractal);
	if (!(fractal->id == 3))
	{
		mlx_hook(fractal->mlx_win, MotionNotify, \
		PointerMotionMask, julia_handle, fractal);
	}
	mlx_hook(fractal->mlx_win, ButtonPress, \
	ButtonPressMask, mouse_handler, fractal);
	mlx_hook(fractal->mlx_win, DestroyNotify, \
	StructureNotifyMask, close_handler, fractal);
	fractal->species = 0;
}

static void	display_geometry_init(t_fractal *fractal)
{
	int	screen_width;
	int	screen_height;

	fractal->display_width = fractal->width;
	fractal->display_height = fractal->height;
	fractal->display_rotated = false;
	memset(&fractal->display_img, 0, sizeof(fractal->display_img));
	mlx_get_screen_size(fractal->mlx_connect, &screen_width, &screen_height);
	(void)screen_width;
	if (fractal->id == 3 && buddha_should_rotate_display(fractal->width,
			fractal->height, screen_height))
	{
		fractal->display_rotated = true;
		fractal->display_width = fractal->height;
		fractal->display_height = fractal->width;
		printf("Buddha display rotated clockwise to fit screen height; "
			"export remains %dx%d.\n", fractal->width, fractal->height);
	}
}

void	fractal_init(t_fractal *fractal)
{
	fractal->mlx_connect = mlx_init();
	if (fractal->mlx_connect == NULL)
		exit(EXIT_FAILURE);
	display_geometry_init(fractal);
	fractal->mlx_win = mlx_new_window(fractal->mlx_connect,
			fractal->display_width, fractal->display_height, fractal->name);
	if (fractal->mlx_win == NULL)
		clear_all(fractal);
	if (new_img_init(fractal->mlx_connect, &fractal->img, fractal->width, fractal->height) == -1)
		clear_all(fractal);
	if (new_img_init(fractal->mlx_connect, &fractal->img_2, fractal->width, fractal->height) == -1)
		clear_all(fractal);
	if (fractal->display_rotated && new_img_init(fractal->mlx_connect,
			&fractal->display_img, fractal->display_width,
			fractal->display_height) == -1)
		clear_all(fractal);
	info_init(fractal);
	fractal->w_colors = set_color_wheel(360, 1.0, 0.5, 202);//num colors, sat, lightness, base hue
	if (!fractal->w_colors)
		clear_all(fractal);
	events_init(fractal);
	if (fractal->id == 3)
		init_buddha(fractal);
}
