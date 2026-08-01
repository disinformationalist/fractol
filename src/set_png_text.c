#include "fractol.h"

#define FRACTOL_METADATA_CAPACITY 32
#define FRACTOL_METADATA_VERSION "fractol-render-v5"

static char	*format_text(const char *format, ...)
{
	va_list	copy;
	va_list	args;
	char	*text;
	int		length;

	va_start(args, format);
	va_copy(copy, args);
	length = vsnprintf(NULL, 0, format, copy);
	va_end(copy);
	if (length < 0)
		return (va_end(args), NULL);
	text = malloc((size_t)length + 1);
	if (!text)
		return (va_end(args), NULL);
	vsnprintf(text, (size_t)length + 1, format, args);
	va_end(args);
	return (text);
}

static void	free_text_entries(png_text *text, int count)
{
	while (--count >= 0)
	{
		free(text[count].key);
		free(text[count].text);
	}
	free(text);
}

static int	add_text(png_text *text, int *count, const char *key,
		const char *format, ...)
{
	va_list	copy;
	va_list	args;
	char	*value;
	int		length;

	if (*count >= FRACTOL_METADATA_CAPACITY - 1)
		return (-1);
	va_start(args, format);
	va_copy(copy, args);
	length = vsnprintf(NULL, 0, format, copy);
	va_end(copy);
	if (length < 0)
		return (va_end(args), -1);
	value = malloc((size_t)length + 1);
	if (!value)
		return (va_end(args), -1);
	vsnprintf(value, (size_t)length + 1, format, args);
	va_end(args);
	text[*count].key = ft_strdup(key);
	if (!text[*count].key)
		return (free(value), -1);
	text[*count].text = value;
	text[*count].compression = PNG_TEXT_COMPRESSION_NONE;
	(*count)++;
	return (0);
}

static const char	*fractal_name(t_fractal *fractal)
{
	if (fractal->id == 1)
		return ("Mandelbrot");
	if (fractal->id == 2)
		return ("Julia");
	if (fractal->id == 3)
		return ("Buddhabrot");
	if (fractal->id == 4)
		return ("Barnsley Fern");
	return ("Unknown");
}

static const char	*buddha_type_name(t_btype type)
{
	if (type == BUDDHA1)
		return ("standard");
	if (type == BUDDHA2)
		return ("custom");
	if (type == LOTUS)
		return ("space-lotus");
	return ("phoenix");
}

static const char	*formula_name(f formula)
{
	if (formula == &square_complex)
		return ("square");
	if (formula == &square_complex_conj)
		return ("square-conjugate");
	if (formula == &cube_ship)
		return ("cube-absolute");
	if (formula == &cube_complex)
		return ("cube");
	if (formula == &ship)
		return ("burning-ship");
	if (formula == &celtic)
		return ("celtic");
	return ("custom-function-pointer");
}

static char	*rebuild_command(t_fractal *fractal)
{
	if (fractal->id == 1)
		return (format_text("./fractol mandelbrot %d %d",
				fractal->width_orig, fractal->height_orig));
	if (fractal->id == 2)
		return (format_text("./fractol julia %a %a %d %d",
				fractal->julia_x, fractal->julia_y,
				fractal->width_orig, fractal->height_orig));
	if (fractal->id == 3)
		return (format_text("./fractol buddha %d %d %d "
				"--center %a %a --zoom %a%s",
				fractal->width_orig, fractal->height_orig,
				fractal->buddha->type, fractal->move_x,
				-fractal->move_y, fractal->zoom,
				(char *[2]){"", " --importance-map"}
				[fractal->buddha->importance_enabled]));
	if (fractal->id == 4)
		return (format_text("./fractol fern %d %d",
				fractal->width_orig, fractal->height_orig));
	return (format_text("./fractol"));
}

static const char	*png_precision_source(t_fractal *f)
{
	if ((f->id == 1 || f->id == 2) && f->w_colors)
		return ("orbit-recompute-double");
	if (f->id == 3 && f->buddha && f->buddha->importance_view
		&& f->buddha->importance_ready && f->buddha->importance_values)
		return ("importance-density-double-tone");
	if (f->id == 3 && f->buddha && f->densities
		&& !f->buddha->importance_view)
	{
		if (f->buddha->filter && f->buddha->ftype == ADJUST)
			return ("density-double-tone-adjust");
		if (f->buddha->filter && f->buddha->ftype == GAUSS)
			return ("density-double-tone-gaussian");
		return ("density-double-tone");
	}
	if (f->id == 4 && f->fdensity)
		return ("density-integer-double-palette");
	return ("preview-8bit-expanded");
}

static int	add_common_text(png_text *text, int *count, t_fractal *f)
{
	char	*command;
	int		image_color;
	int		failed;

	command = rebuild_command(f);
	if (!command)
		return (-1);
	image_color = f->col_i;
	if (!f->toggle_color && f->num_colors > 0)
		image_color = (f->col_i - 7 + f->num_colors) % f->num_colors;
	failed = 0;
	failed |= add_text(text, count, "Metadata Version", "%s",
			FRACTOL_METADATA_VERSION);
	failed |= add_text(text, count, "Generator", "fractol");
	failed |= add_text(text, count, "Fractal", "%s; id=%d",
			fractal_name(f), f->id);
	failed |= add_text(text, count, "CLI Baseline", "%s", command);
	failed |= add_text(text, count, "Output Size", "width=%d;height=%d",
			f->width_orig, f->height_orig);
	failed |= add_text(text, count, "PNG Encoding",
			"depth=16;color=RGB;preview_depth=8;writer=streaming-rows;"
			"source=%s", png_precision_source(f));
	failed |= add_text(text, count, "Render State",
			"width=%d;height=%d;aspect=%a", f->width, f->height, f->aspect);
	failed |= add_text(text, count, "View",
			"zoom=%a;center_real=%a;center_imaginary=%a;"
			"move_x=%a;move_y=%a;bound=%a",
			f->zoom, f->move_x, -f->move_y,
			f->move_x, f->move_y, f->bound);
	failed |= add_text(text, count, "Iterations",
			"max=%d;zoom_iter=%d", f->max_i, f->zoom_iter);
	failed |= add_text(text, count, "Color",
			"image_index=%d;state_index=%d;count=%d;auto_cycle=%d;"
			"spectrum=%d;source_x=%a;source_y=%a",
			image_color, f->col_i, f->num_colors, !f->toggle_color,
			f->color_spectrum, f->move_col_x, f->move_col_y);
	failed |= add_text(text, count, "Supersampling",
			"enabled=%d;kernel=%d", f->supersample, f->s_kernel);
	failed |= add_text(text, count, "Workers", "rows=%d;columns=%d",
			f->num_rows, f->num_cols);
	free(command);
	return ((failed != 0) * -1);
}

static int	add_julia_text(png_text *text, int *count, t_fractal *fractal)
{
	return (add_text(text, count, "Julia Constant", "real=%a;imaginary=%a",
			fractal->julia_x, fractal->julia_y));
}

static int	add_fern_text(png_text *text, int *count, t_fractal *fractal)
{
	int	failed;

	failed = 0;
	failed |= add_text(text, count, "Fern",
			"species=%d;sample_multiplier=7;density_coloring=log",
			fractal->species);
	failed |= add_text(text, count, "Fern Palette",
			"color1=%08X;color2=%08X;color3=%08X;color4=%08X",
			fractal->colors.color_1, fractal->colors.color_2,
			fractal->colors.color_3, fractal->colors.color_4);
	failed |= add_text(text, count, "RNG",
			"xoroshiro128plus;splitmix64;seed=0xFABDECAF;"
			"worker_step=142857");
	return ((failed != 0) * -1);
}

static int	add_buddha_tone_text(png_text *text, int *count, t_buddha *b)
{
	int	failed;

	failed = 0;
	failed |= add_text(text, count, "Buddha Tone Powers",
			"blue=%a;green=%a;red=%a", b->bpow, b->gpow, b->rpow);
	failed |= add_text(text, count, "Buddha Tone Low Edges",
			"blue=%a;green=%a;red=%a",
			b->edge0_b, b->edge0_g, b->edge0_r);
	failed |= add_text(text, count, "Buddha Tone High Edges",
			"blue=%a;green=%a;red=%a",
			b->edge1_b, b->edge1_g, b->edge1_r);
	failed |= add_text(text, count, "Buddha Exposure",
			"global=%a;blue=%a;green=%a;red=%a",
			b->global_exposure, b->exposure_b,
			b->exposure_g, b->exposure_r);
	failed |= add_text(text, count, "Buddha Density High",
			"blue=%a;green=%a;red=%a", b->high_b, b->high_g, b->high_r);
	failed |= add_text(text, count, "Buddha White Points",
			"blue=%a;green=%a;red=%a;percentile=%a",
			b->white_b, b->white_g, b->white_r, b->white_percentile);
	return ((failed != 0) * -1);
}

static int	add_buddha_text(png_text *text, int *count, t_fractal *f)
{
	t_buddha	*b;
	int			failed;

	b = f->buddha;
	failed = 0;
	failed |= add_text(text, count, "Buddha Type", "%s;id=%d",
			buddha_type_name(b->type), b->type);
	failed |= add_text(text, count, "Buddha Sampling",
			"n=%a;buffers=%d;fast=%d;allocation=flat-u32",
			b->n, f->buffs, b->fast);
	failed |= add_text(text, count, "Buddha Importance",
			"map_n=%a;effective_scale=%d;adaptive=%d;pilot=%s;"
			"pilot_seed=%llX;refinement=%s;"
			"score=%s;aggregate=%s;proposal=%s;global=%a;window=%a;"
			"guided=%a", b->map_n, buddha_map_scale(f),
			b->map_adaptive,
			(char *[2]){"centered-grid", "stratified-hash-jitter-v1"}
			[b->importance_pilot_jitter],
			(unsigned long long)BUDDHA_PILOT_JITTER_SEED,
			(char *[3]){"off", "beam-quadtree-v1-auto",
				"beam-quadtree-v1-force"}
			[b->importance_refinement + b->importance_refinement_force],
			(char *[2]){"visible-hits", "viewport-tile-l2"}
			[b->importance_recurrence],
			(char *[2]){"mean", "rms"}
			[b->importance_rms],
			(char *[2]){"legacy-cell", "defensive-mixture-v1"}
			[b->proposal_mixture], b->proposal_global, b->proposal_window,
			1.0 - b->proposal_global - b->proposal_window);
	failed |= add_text(text, count, "Buddha Channel Iterations",
			"blue=%d:%d;green=%d:%d;red=%d:%d",
			b->min1, b->max1, b->min2, b->max2, b->min3, b->max3);
	failed |= add_text(text, count, "Buddha Formula",
			"name=%s;square_specialized=%d;orbit_cache=%d;"
			"orbit_cache_points=%d;interior_rejection=%d",
			formula_name(f->complex_f), b->square_specialized,
			b->orbit_cache, BUDDHA_ORBIT_CACHE_POINTS,
			b->interior_rejection);
	failed |= add_buddha_tone_text(text, count, b);
	failed |= add_text(text, count, "Buddha Tone Mode",
			"smootherstep=%d;normalization=%s;normalization_id=%d;"
			"negative_mode=%s;negative_mode_id=%d;"
			"spill_smootherstep=post-rgb;extra_fine=%d;"
			"zero_density=black",
			b->smootherstep, buddha_normalization_name(b->normalization),
			b->normalization, buddha_negative_mode_name(b->negative_mode),
			b->negative_mode, b->tone_extra_fine);
	failed |= add_text(text, count, "Buddha Hybrid Tone",
			"spill=%a;background=%06X;foreground=%06X;"
			"base=percentile-rms-default-powers",
			b->hybrid_spill, b->hybrid_background, b->hybrid_foreground);
	failed |= add_text(text, count, "Buddha NLM",
			"enabled=%d;smooth_variance=%d;patch=%d;search=%d;kc=%a;"
			"filtered_ready=%d;display=%s;variance_scratch=two-row-double;"
			"distance_variance=mixture-reference-plus-min;legacy=sum",
			b->nlm_enabled, b->nlm_smooth_var, b->nlm_patch_radius,
			b->nlm_search_radius, b->nlm_kc, b->nlm_filtered_ready,
			(char *[2]){"unfiltered-mean", "filtered"}
			[b->nlm_show_filtered]);
	failed |= add_text(text, count, "Buddha NLM Noise",
			"scale=%a;floor=%a;mixture_relative_variance_cap=%a;"
			"mixture_firefly_factor=%a;firefly_white_threshold=0;"
			"firefly_support=per-channel-5x5-median;"
			"blue_weight=%a;green_weight=%a;red_weight=%a",
			b->nlm_noise_scale, b->nlm_noise_floor,
			b->nlm_relative_variance_cap,
			b->nlm_firefly_factor,
			b->nlm_b_weight, b->nlm_g_weight, b->nlm_r_weight);
	failed |= add_text(text, count, "Buddha Filter",
			"enabled=%d;type=%d;level=%d",
			b->filter, b->ftype, b->flevel);
	failed |= add_text(text, count, "Buddha Importance View",
			"enabled=%d;ready=%d;viewing=%d;mode=%s;"
			"capture=double-log1p-block-average-to-u16",
			b->importance_enabled, b->importance_ready, b->importance_view,
			buddha_importance_mode_name(b->importance_mode));
	failed |= add_text(text, count, "RNG",
			"xoroshiro128plus;splitmix64;base=0xFABDECAF;"
			"buffer_step=0x9E3779B97F4A7C15;worker_step=142857");
	return ((failed != 0) * -1);
}

png_text	*build_fractal_text(t_fractal *fractal)
{
	png_text	*text;
	int			count;
	int			failed;

	text = calloc(FRACTOL_METADATA_CAPACITY, sizeof(*text));
	if (!text)
		return (NULL);
	count = 0;
	failed = add_common_text(text, &count, fractal);
	if (fractal->id == 2)
		failed |= add_julia_text(text, &count, fractal);
	else if (fractal->id == 3 && fractal->buddha)
		failed |= add_buddha_text(text, &count, fractal);
	else if (fractal->id == 4)
		failed |= add_fern_text(text, &count, fractal);
	if (failed)
		return (free_text_entries(text, count), NULL);
	return (text);
}
