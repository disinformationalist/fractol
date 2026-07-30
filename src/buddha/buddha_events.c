#include "fractol.h"

void	filter(t_fractal *fractal, t_buddha *b)
{
	if (b->filter)
	{
		img_cpy(&fractal->img_2, &fractal->img,
			fractal->width, fractal->height);
		if (b->ftype == ADJUST)
			adjust_pixels_rgb(&fractal->img, fractal->width,
				fractal->height,
				(double)((b->flevel - 1) >> 1) * .5 + 1, 0, RGB1);
		else
			gaussian_convo_filter(fractal->mlx_connect, &fractal->img,
				fractal->width, fractal->height, 5,
				((b->flevel - 1) >> 1) * .5);
	}
	else
		img_cpy(&fractal->img, &fractal->img_2,
			fractal->width, fractal->height);
}

void	change_flevel(int keysym, t_buddha *b)
{
	if (keysym == Z)
	{
		b->flevel += 2;
		if (b->ftype == GAUSS && b->flevel > 7)
			b->flevel = 7;
		else if (b->flevel > 33)
			b->flevel = 33;
	}
	else
	{
		b->flevel -= 2;
		if (b->flevel < 3)
			b->flevel = 3;
	}
}

static bool	change_powers(int keysym, t_buddha *b, bool fine)
{
	if (keysym == Q)
		b->bpow = buddha_adjust_power(b->bpow, 1.0, fine,
				b->tone_extra_fine);
	else if (keysym == A)
		b->bpow = buddha_adjust_power(b->bpow, -1.0, fine,
				b->tone_extra_fine);
	else if (keysym == W)
		b->gpow = buddha_adjust_power(b->gpow, 1.0, fine,
				b->tone_extra_fine);
	else if (keysym == S)
		b->gpow = buddha_adjust_power(b->gpow, -1.0, fine,
				b->tone_extra_fine);
	else if (keysym == E)
		b->rpow = buddha_adjust_power(b->rpow, 1.0, fine,
				b->tone_extra_fine);
	else if (keysym == D)
		b->rpow = buddha_adjust_power(b->rpow, -1.0, fine,
				b->tone_extra_fine);
	else
		return (false);
	return (true);
}

static double	bounded_gain(double gain, double change)
{
	gain += change;
	if (gain < 0.05)
		return (0.05);
	if (gain > 4.0)
		return (4.0);
	return (gain);
}

static bool	change_channel_gains(int keysym, t_buddha *b, double change)
{
	if (keysym == Q)
		b->exposure_b = bounded_gain(b->exposure_b, change);
	else if (keysym == A)
		b->exposure_b = bounded_gain(b->exposure_b, -change);
	else if (keysym == W)
		b->exposure_g = bounded_gain(b->exposure_g, change);
	else if (keysym == S)
		b->exposure_g = bounded_gain(b->exposure_g, -change);
	else if (keysym == E)
		b->exposure_r = bounded_gain(b->exposure_r, change);
	else if (keysym == D)
		b->exposure_r = bounded_gain(b->exposure_r, -change);
	else
		return (false);
	return (true);
}

static double	bounded_edge0(double edge0, double edge1, double change)
{
	edge0 += change;
	if (edge0 < 0.0)
		edge0 = 0.0;
	if (edge0 > edge1 - 0.01)
		edge0 = edge1 - 0.01;
	return (edge0);
}

static double	bounded_edge1(double edge1, double edge0, double change)
{
	edge1 += change;
	if (edge1 < edge0 + 0.01)
		edge1 = edge0 + 0.01;
	if (edge1 > 4.0)
		edge1 = 4.0;
	return (edge1);
}

static bool	change_edges(int keysym, t_buddha *b, double change, bool high)
{
	if (keysym == U)
	{
		if (high)
			b->edge1_b = bounded_edge1(b->edge1_b, b->edge0_b, change);
		else
			b->edge0_b = bounded_edge0(b->edge0_b, b->edge1_b, change);
	}
	else if (keysym == J)
	{
		if (high)
			b->edge1_b = bounded_edge1(b->edge1_b, b->edge0_b, -change);
		else
			b->edge0_b = bounded_edge0(b->edge0_b, b->edge1_b, -change);
	}
	else if (keysym == I)
	{
		if (high)
			b->edge1_g = bounded_edge1(b->edge1_g, b->edge0_g, change);
		else
			b->edge0_g = bounded_edge0(b->edge0_g, b->edge1_g, change);
	}
	else if (keysym == K)
	{
		if (high)
			b->edge1_g = bounded_edge1(b->edge1_g, b->edge0_g, -change);
		else
			b->edge0_g = bounded_edge0(b->edge0_g, b->edge1_g, -change);
	}
	else if (keysym == O)
	{
		if (high)
			b->edge1_r = bounded_edge1(b->edge1_r, b->edge0_r, change);
		else
			b->edge0_r = bounded_edge0(b->edge0_r, b->edge1_r, change);
	}
	else if (keysym == L)
	{
		if (high)
			b->edge1_r = bounded_edge1(b->edge1_r, b->edge0_r, -change);
		else
			b->edge0_r = bounded_edge0(b->edge0_r, b->edge1_r, -change);
	}
	else
		return (false);
	return (true);
}

void	change_filter(t_buddha *b)
{
	b->ftype++;
	if (b->ftype > GAUSS)
		b->ftype = ADJUST;
}

static bool	change_white_percentile(int keysym, t_buddha *b)
{
	if (keysym == COMMA)
		b->white_percentile -= 0.0001;
	else if (keysym == PERIOD)
		b->white_percentile += 0.0001;
	else
		return (false);
	if (b->white_percentile < 0.5)
		b->white_percentile = 0.5;
	if (b->white_percentile > 1.0)
		b->white_percentile = 1.0;
	printf("Buddha white percentile: %.4f (%.2f%%)\n",
		b->white_percentile, b->white_percentile * 100.0);
	return (true);
}

static void	print_tone_summary(t_buddha *b)
{
	printf("Buddha tone: norm=%s negative=%s hybrid-spill=%.2f "
		"global=%.2f smoother=%s edit=%s step=%s "
		"powers=[B %.3f, G %.3f, R %.3f] gains=[%.2f, %.2f, %.2f]\n",
		buddha_normalization_name(b->normalization),
		buddha_negative_mode_name(b->negative_mode), b->hybrid_spill,
		b->global_exposure,
		(char *[2]){"off", "on"}[b->smootherstep],
		(char *[2]){"power", "channel gain"}[b->tone_gain_mode],
		(char *[2]){"layer coarse/fine", "extra-fine .001"}
			[b->tone_extra_fine],
		b->bpow, b->gpow, b->rpow,
		b->exposure_b, b->exposure_g, b->exposure_r);
}

static void	change_hybrid_spill(t_buddha *b, double change)
{
	b->hybrid_spill += change;
	if (b->hybrid_spill < 0.0)
		b->hybrid_spill = 0.0;
	if (b->hybrid_spill > 1.0)
		b->hybrid_spill = 1.0;
}

static void	cycle_normalization(t_buddha *b)
{
	b->normalization++;
	if (b->normalization > BUDDHA_NORM_LEGACY_MAX)
		b->normalization = BUDDHA_NORM_CHANNEL_PERCENTILE;
}

static bool	change_global_exposure(int keysym, t_buddha *b)
{
	if (keysym == PLUS || keysym == PAD_PLUS
		|| keysym == XK_plus || keysym == XK_equal)
		b->global_exposure += 0.05;
	else if (keysym == MINUS || keysym == PAD_MINUS || keysym == XK_minus)
		b->global_exposure -= 0.05;
	else
		return (false);
	if (b->global_exposure < 0.05)
		b->global_exposure = 0.05;
	if (b->global_exposure > 20.0)
		b->global_exposure = 20.0;
	return (true);
}

static bool	handle_tone_control(int keysym, t_fractal *fractal)
{
	t_buddha	*b;
	double		gain_step;
	bool		refresh_white;
	bool		handled;

	b = fractal->buddha;
	gain_step = 0.05;
	if (fractal->layer)
		gain_step = 0.01;
	if (b->tone_extra_fine)
		gain_step = 0.001;
	refresh_white = false;
	if (b->tone_gain_mode)
		handled = change_channel_gains(keysym, b, gain_step);
	else
		handled = change_powers(keysym, b, fractal->layer);
	if (!handled)
		handled = change_edges(keysym, b, b->change, fractal->layer);
	if (!handled)
		handled = change_global_exposure(keysym, b);
	if (!handled && change_white_percentile(keysym, b))
	{
		handled = true;
		refresh_white = true;
	}
	if (keysym == N_1)
	{
		change_hybrid_spill(b, -0.05);
		handled = true;
	}
	else if (keysym == N_2)
	{
		change_hybrid_spill(b, 0.05);
		handled = true;
	}
	else if (keysym == N_3)
	{
		b->negative_mode++;
		if (b->negative_mode > BUDDHA_NEGATIVE_HYBRID)
			b->negative_mode = BUDDHA_NEGATIVE_INVERSE;
		handled = true;
	}
	else if (keysym == N_4)
	{
		b->tone_gain_mode = !b->tone_gain_mode;
		handled = true;
	}
	else if (keysym == N_5)
	{
		buddha_tone_swap_alternate(b);
		handled = true;
		refresh_white = true;
	}
	else if (keysym == N_6)
	{
		buddha_tone_reset(b);
		handled = true;
		refresh_white = true;
	}
	else if (keysym == N_7)
	{
		cycle_normalization(b);
		handled = true;
		refresh_white = true;
	}
	else if (keysym == N_8)
	{
		b->bpow = -b->bpow;
		b->gpow = -b->gpow;
		b->rpow = -b->rpow;
		handled = true;
	}
	else if (keysym == N_9)
	{
		b->smootherstep = !b->smootherstep;
		handled = true;
	}
	if (!handled)
		return (false);
	if (refresh_white)
		buddha_update_white_points(fractal);
	print_tone_summary(b);
	return (true);
}

static void	redraw_buddha(t_fractal *fractal)
{
	t_buddha	*b;

	b = fractal->buddha;
	ft_memset(fractal->img.pixels_ptr, 0, fractal->width_orig
		* fractal->height_orig * (fractal->img.bpp / 8));
	color_buddha(fractal);
	if (b->filter)
		filter(fractal, b);
	buddha_importance_store_render(fractal);
	if (b->importance_view)
		buddha_importance_draw(fractal);
	display_fractal_image(fractal);
}

static bool	nlm_control_available(t_fractal *fractal)
{
	if (fractal->supersample)
		return (printf("NLM controls unavailable during supersampling\n"), false);
	if (fractal->buffs < 2 || !fractal->buddha->nlm_filtered_ready)
		return (printf("NLM controls unavailable: no filtered render retained\n"),
			false);
	return (true);
}

static void	adjust_nlm_setting(int keysym, t_buddha *b)
{
	if (keysym == F5)
		b->nlm_kc -= 0.05;
	else if (keysym == F6)
		b->nlm_kc += 0.05;
	else if (keysym == F7)
		b->nlm_patch_radius--;
	else if (keysym == F8)
		b->nlm_patch_radius++;
	else if (keysym == F9)
		b->nlm_search_radius--;
	else if (keysym == F10)
		b->nlm_search_radius++;
	if (b->nlm_kc < 0.05)
		b->nlm_kc = 0.05;
	if (b->nlm_kc > 8.0)
		b->nlm_kc = 8.0;
	if (b->nlm_patch_radius < 0)
		b->nlm_patch_radius = 0;
	if (b->nlm_patch_radius > 8)
		b->nlm_patch_radius = 8;
	if (b->nlm_search_radius < 0)
		b->nlm_search_radius = 0;
	if (b->nlm_search_radius > 64)
		b->nlm_search_radius = 64;
}

static bool	handle_nlm_control(int keysym, t_fractal *fractal)
{
	t_buddha	*b;
	long		start;
	double		old_kc;
	int			old_patch;
	int			old_search;

	if (keysym < F4 || keysym > F10)
		return (false);
	if (!nlm_control_available(fractal))
		return (true);
	b = fractal->buddha;
	if (keysym == F4)
	{
		buddha_nlm_toggle_output(fractal);
		buddha_update_white_points(fractal);
		printf("NLM view: %s\n",
			(char *[2]){"unfiltered mean", "filtered"}[b->nlm_show_filtered]);
		redraw_buddha(fractal);
		return (true);
	}
	old_kc = b->nlm_kc;
	old_patch = b->nlm_patch_radius;
	old_search = b->nlm_search_radius;
	adjust_nlm_setting(keysym, b);
	if (old_kc == b->nlm_kc && old_patch == b->nlm_patch_radius
		&& old_search == b->nlm_search_radius)
		return (printf("NLM setting already at its limit\n"), true);
	printf("NLM refilter in progress...\n");
	fflush(stdout);
	start = get_time();
	if (buddha_nlm(fractal) != 0)
		return (fprintf(stderr, "NLM refilter failed\n"), true);
	buddha_update_white_points(fractal);
	printf("NLM refilter: patch=%d search=%d kc=%.2f (%.3f seconds)\n",
		b->nlm_patch_radius, b->nlm_search_radius, b->nlm_kc,
		(double)(get_time() - start) / 1000.0);
	redraw_buddha(fractal);
	return (true);
}

static void	print_filter_summary(t_buddha *b)
{
	if (b->ftype == ADJUST)
		printf("Buddha filter: %s adjust multiplier=%.2f\n",
			(char *[2]){"off", "on"}[b->filter],
			(double)((b->flevel - 1) >> 1) * 0.5 + 1.0);
	else
		printf("Buddha filter: %s gaussian sigma=%.2f\n",
			(char *[2]){"off", "on"}[b->filter],
			(double)((b->flevel - 1) >> 1) * 0.5);
}

static bool	handle_filter_control(int keysym, t_fractal *fractal)
{
	t_buddha	*b;

	b = fractal->buddha;
	if (keysym == F)
	{
		change_filter(b);
		if (b->ftype == GAUSS && b->flevel > 7)
			b->flevel = 7;
	}
	else if (keysym == RGHT_STRG)
	{
		b->filter = !b->filter;
		if (fractal->supersample)
			filter(fractal, b);
	}
	else if (keysym == Z || keysym == H)
		change_flevel(keysym, b);
	else
		return (false);
	print_filter_summary(b);
	if (!fractal->supersample)
		redraw_buddha(fractal);
	return (true);
}

void	buddha_handler(int keysym, t_fractal *fractal)
{
	t_buddha	*b;

	b = fractal->buddha;
	if (keysym == M)
	{
		buddha_importance_toggle(fractal);
		return ;
	}
	if (keysym == N)
	{
		buddha_importance_cycle(fractal);
		return ;
	}
	if (handle_nlm_control(keysym, fractal))
		return ;
	if (keysym == X)
	{
		b->tone_extra_fine = !b->tone_extra_fine;
		printf("Buddha power/gain step: %s\n",
			(char *[2]){"normal layer step", "extra-fine 0.001"}
			[b->tone_extra_fine]);
		return ;
	}
	if (keysym == ALT)
	{
		if (b->change == .05)
			b->change = .01;
		else
			b->change = .05;
		printf("Buddha edge adjustment step: %.2f\n", b->change);
	}
	else if (keysym == SPACE)
	{
		supersample_handle(keysym, fractal);
		render(fractal);
		return ;
	}
	else if (keysym == N_0)
	{
		print_buddha_vals(b, fractal);
	}
	else if (keysym == F3)
		export(keysym, fractal);
	else if (keysym == F2)
		print_buddha_guide();
	else if (keysym == XK_Tab)
		printf("Buddha tone layer: %s; power step %.3f%s\n",
			(char *[2]){"coarse / low edges",
				"fine / high edges"}[fractal->layer],
			(double [2]){0.05, 0.01}[fractal->layer],
			(char *[2]){"", " (overridden by X: 0.001)"}
			[b->tone_extra_fine]);
	else if (handle_filter_control(keysym, fractal))
		return ;
	else if (handle_tone_control(keysym, fractal))
	{
		if (!fractal->supersample)
			redraw_buddha(fractal);
	}
	else
		return ;
}
