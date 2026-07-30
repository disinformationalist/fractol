#include "fractol.h"

typedef struct s_tone_channel
{
	double			power;
	double			edge0;
	double			edge1;
	double			reference;
	double			magnitude;
	double			scale;
	double			legacy_scale;
	bool			inverse;
	bool			smoother;
	bool			spill;
	unsigned int	shift;
}	t_tone_channel;

typedef struct s_hybrid_tone
{
	t_tone_channel	channel[3];
	double			high[3];
	double			reference[3];
	double			base_power[3];
	double			base_scale[3];
	double			edge0;
	double			edge1;
	double			spill_mix;
	unsigned int	background;
	unsigned int	foreground;
	bool			smoother;
}	t_hybrid_tone;

static inline void	set_pixel(t_fractal *fractal, int x, int y,
		unsigned int color)
{
	if (!fractal->supersample)
		my_pixel_put_plus(x, y, &fractal->img, color);
	else
		fractal->pixels_xl[y][x] |= color;
}

static bool	use_post_spill_smoother(t_buddha *buddha)
{
	return (buddha->negative_mode == BUDDHA_NEGATIVE_SPILL
		&& buddha->smootherstep
		&& (buddha->bpow < 0.0 || buddha->gpow < 0.0
			|| buddha->rpow < 0.0));
}

static bool	has_negative_power(t_buddha *buddha)
{
	return (buddha->bpow < 0.0 || buddha->gpow < 0.0
		|| buddha->rpow < 0.0);
}

static t_tone_channel	get_tone_channel(t_buddha *buddha, int channel)
{
	t_tone_channel	tone;
	double			exposure;

	if (channel == 0)
		tone = (t_tone_channel){buddha->bpow, buddha->edge0_b,
			buddha->edge1_b, buddha->white_b, 0.0, 0.0, 0.0, false,
			buddha->smootherstep,
			buddha->negative_mode == BUDDHA_NEGATIVE_SPILL, 0};
	else if (channel == 1)
		tone = (t_tone_channel){buddha->gpow, buddha->edge0_g,
			buddha->edge1_g, buddha->white_g, 0.0, 0.0, 0.0, false,
			buddha->smootherstep,
			buddha->negative_mode == BUDDHA_NEGATIVE_SPILL, 8};
	else
		tone = (t_tone_channel){buddha->rpow, buddha->edge0_r,
			buddha->edge1_r, buddha->white_r, 0.0, 0.0, 0.0, false,
			buddha->smootherstep,
			buddha->negative_mode == BUDDHA_NEGATIVE_SPILL, 16};
	tone.inverse = tone.power < 0.0;
	tone.magnitude = fabs(tone.power);
	if (use_post_spill_smoother(buddha))
		tone.smoother = false;
	exposure = ((double [3]){buddha->exposure_b,
			buddha->exposure_g, buddha->exposure_r})[channel];
	tone.scale = buddha->global_exposure * exposure;
	tone.legacy_scale = buddha->global_exposure;
	if (buddha->tone_default.exposure[channel] > 0.0)
		tone.legacy_scale *= exposure
			/ buddha->tone_default.exposure[channel];
	if (!tone.inverse && tone.reference > 0.0 && tone.magnitude > 0.0)
		tone.scale /= pow(tone.reference, tone.magnitude);
	return (tone);
}

static double	map_tone_value(double density, t_tone_channel tone)
{
	double	mapped;
	double	ratio;

	if (density <= 0.0 || tone.reference <= 0.0 || tone.magnitude <= 0.0)
		return (0.0);
	if (tone.inverse)
	{
		ratio = density / tone.reference;
		if (ratio > 1.0)
			ratio = 1.0;
		mapped = (1.0 - pow(ratio, tone.magnitude)) * tone.scale;
	}
	else
		mapped = pow(density, tone.magnitude) * tone.scale;
	if (tone.smoother)
		mapped = buddha_smootherstep_value(tone.edge0, tone.edge1, mapped);
	if (mapped < 0.0)
		return (0.0);
	if (mapped > 1.0)
		return (1.0);
	return (mapped);
}

static void	color_channel(t_fractal *fractal, t_piece *piece, int channel)
{
	t_tone_channel	tone;
	double			mapped;
	double			high;
	unsigned int	color;
	int				x;
	int				y;

	tone = get_tone_channel(fractal->buddha, channel);
	high = ((double [3]){fractal->buddha->high_b,
			fractal->buddha->high_g, fractal->buddha->high_r})[channel];
	y = piece->y_s - 1;
	while (++y < piece->y_e)
	{
		x = piece->x_s - 1;
		while (++x < piece->x_e)
		{
			if (tone.spill && tone.inverse)
				color = buddha_legacy_spill_color(
						fractal->densities[channel][y][x], high,
						tone.power, tone.legacy_scale, tone.shift);
			else
			{
				mapped = map_tone_value(
						fractal->densities[channel][y][x], tone);
				color = (unsigned int)ft_round(mapped * 255.0)
					<< tone.shift;
			}
			set_pixel(fractal, x, y, color);
		}
	}
}

static unsigned int	post_smoother_channel(unsigned int color,
		unsigned int shift, double edge0, double edge1)
{
	double	value;

	value = (double)((color >> shift) & 0xFFu) / 255.0;
	value = buddha_smootherstep_value(edge0, edge1, value);
	return ((unsigned int)ft_round(value * 255.0) << shift);
}

static unsigned int	post_smoother_color(unsigned int color, t_buddha *b)
{
	unsigned int	mapped;

	mapped = post_smoother_channel(color, 0, b->edge0_b, b->edge1_b);
	mapped |= post_smoother_channel(color, 8,
			b->edge0_g, b->edge1_g);
	mapped |= post_smoother_channel(color, 16,
			b->edge0_r, b->edge1_r);
	return (mapped);
}

static void	post_smooth_spill(t_fractal *fractal, t_piece *piece)
{
	t_buddha		*b;
	unsigned int	color;
	unsigned int	mapped;
	int				x;
	int				y;

	b = fractal->buddha;
	y = piece->y_s - 1;
	while (++y < piece->y_e)
	{
		x = piece->x_s - 1;
		while (++x < piece->x_e)
		{
			if (fractal->supersample)
				color = fractal->pixels_xl[y][x];
			else
				color = pixel_color_get(x, y, &fractal->img);
			mapped = post_smoother_color(color, b);
			if (fractal->supersample)
				fractal->pixels_xl[y][x] = mapped;
			else
				my_pixel_put(x, y, &fractal->img, mapped);
		}
	}
}

static double	clamp_unit(double value)
{
	if (value < 0.0)
		return (0.0);
	if (value > 1.0)
		return (1.0);
	return (value);
}

static t_hybrid_tone	get_hybrid_tone(t_buddha *b)
{
	t_hybrid_tone	tone;
	double		exposure;
	int			channel;

	tone.reference[0] = b->white_b;
	tone.reference[1] = b->white_g;
	tone.reference[2] = b->white_r;
	tone.high[0] = b->high_b;
	tone.high[1] = b->high_g;
	tone.high[2] = b->high_r;
	channel = -1;
	while (++channel < 3)
	{
		tone.channel[channel] = get_tone_channel(b, channel);
		tone.channel[channel].smoother = false;
		tone.base_power[channel] = fabs(b->tone_default.power[channel]);
		if (tone.base_power[channel] < 0.05)
			tone.base_power[channel] = 0.05;
		exposure = ((double [3]){b->exposure_b,
				b->exposure_g, b->exposure_r})[channel];
		if (b->tone_default.exposure[channel] > 0.0)
			exposure /= b->tone_default.exposure[channel];
		tone.base_scale[channel] = exposure * b->global_exposure;
	}
	tone.edge0 = (b->edge0_b + b->edge0_g + b->edge0_r) / 3.0;
	tone.edge1 = (b->edge1_b + b->edge1_g + b->edge1_r) / 3.0;
	tone.spill_mix = b->hybrid_spill;
	tone.background = b->hybrid_background;
	tone.foreground = b->hybrid_foreground;
	tone.smoother = b->smootherstep;
	return (tone);
}

static double	hybrid_channel_value(t_fractal *fractal,
		t_hybrid_tone *tone, int channel, int x, int y)
{
	double	density;

	density = fractal->densities[channel][y][x];
	if (density <= 0.0 || tone->reference[channel] <= 0.0)
		return (0.0);
	return (clamp_unit(pow(clamp_unit(
					density / tone->reference[channel]),
				tone->base_power[channel]) * tone->base_scale[channel]));
}

static double	hybrid_structure(t_fractal *fractal,
		t_hybrid_tone *tone, int x, int y)
{
	double		value[3];
	double		structure;

	value[0] = hybrid_channel_value(fractal, tone, 0, x, y);
	value[1] = hybrid_channel_value(fractal, tone, 1, x, y);
	value[2] = hybrid_channel_value(fractal, tone, 2, x, y);
	structure = sqrt((value[0] * value[0] + value[1] * value[1]
				+ value[2] * value[2]) / 3.0);
	if (!tone->smoother)
		return (structure);
	return (buddha_smootherstep_value(
			tone->edge0, tone->edge1, structure));
}

static unsigned int	interpolate_rgb(unsigned int first,
		unsigned int second, double amount)
{
	unsigned int	color;
	double			value;
	int				channel;
	int				shift;

	color = 0;
	channel = -1;
	while (++channel < 3)
	{
		shift = channel * 8;
		value = (double)((first >> shift) & 0xFFu) * (1.0 - amount)
			+ (double)((second >> shift) & 0xFFu) * amount;
		color |= (unsigned int)ft_round(value) << shift;
	}
	return (color);
}

static unsigned int	hybrid_spill_color(t_fractal *fractal,
		t_hybrid_tone *hybrid, int x, int y)
{
	t_buddha		*b;
	t_tone_channel	tone;
	unsigned int	color;
	double			mapped;
	int				channel;

	b = fractal->buddha;
	color = 0;
	channel = -1;
	while (++channel < 3)
	{
		tone = hybrid->channel[channel];
		if (tone.inverse)
			color |= buddha_legacy_spill_color(
					fractal->densities[channel][y][x],
					hybrid->high[channel],
					tone.power, tone.legacy_scale, tone.shift);
		else
		{
			mapped = map_tone_value(
					fractal->densities[channel][y][x], tone);
			color |= (unsigned int)ft_round(mapped * 255.0) << tone.shift;
		}
	}
	if (b->smootherstep)
		color = post_smoother_color(color, b);
	return (color);
}

static double	packed_channel(unsigned int color, int channel)
{
	return ((double)((color >> (channel * 8)) & 0xFFu) / 255.0);
}

static void	hybrid_source_rgb(t_fractal *fractal, int x, int y,
		double rgb[3])
{
	t_hybrid_tone	tone;
	unsigned int	spill;
	double			duotone;
	double			structure;
	double			amount;
	int				channel;
	int				shift;

	tone = get_hybrid_tone(fractal->buddha);
	structure = hybrid_structure(fractal, &tone, x, y);
	spill = hybrid_spill_color(fractal, &tone, x, y);
	amount = tone.spill_mix * structure;
	channel = -1;
	while (++channel < 3)
	{
		shift = channel * 8;
		duotone = (double)((tone.background >> shift) & 0xFFu) / 255.0
			* (1.0 - structure)
			+ (double)((tone.foreground >> shift) & 0xFFu) / 255.0
			* structure;
		rgb[2 - channel] = duotone * (1.0 - amount)
			+ packed_channel(spill, channel) * amount;
	}
}

static unsigned int	standard_spill_pixel(t_fractal *fractal, int x, int y)
{
	t_tone_channel	tone;
	unsigned int	color;
	double			high;
	double			mapped;
	int				channel;

	color = 0;
	channel = -1;
	while (++channel < 3)
	{
		tone = get_tone_channel(fractal->buddha, channel);
		high = ((double [3]){fractal->buddha->high_b,
				fractal->buddha->high_g,
				fractal->buddha->high_r})[channel];
		if (tone.spill && tone.inverse)
			color |= buddha_legacy_spill_color(
					fractal->densities[channel][y][x], high,
					tone.power, tone.legacy_scale, tone.shift);
		else
		{
			mapped = map_tone_value(
					fractal->densities[channel][y][x], tone);
			color |= (unsigned int)ft_round(mapped * 255.0) << tone.shift;
		}
	}
	if (use_post_spill_smoother(fractal->buddha))
		color = post_smoother_color(color, fractal->buddha);
	return (color);
}

static void	standard_source_rgb(t_fractal *fractal, int x, int y,
		double rgb[3])
{
	t_tone_channel	tone;
	unsigned int	color;
	int				channel;

	if (fractal->buddha->negative_mode == BUDDHA_NEGATIVE_SPILL
		&& has_negative_power(fractal->buddha))
	{
		color = standard_spill_pixel(fractal, x, y);
		rgb[0] = packed_channel(color, 2);
		rgb[1] = packed_channel(color, 1);
		rgb[2] = packed_channel(color, 0);
		return ;
	}
	channel = -1;
	while (++channel < 3)
	{
		tone = get_tone_channel(fractal->buddha, channel);
		rgb[2 - channel] = map_tone_value(
				fractal->densities[channel][y][x], tone);
	}
}

static void	buddha_source_rgb(t_fractal *fractal, int x, int y,
		double rgb[3])
{
	if (fractal->buddha->negative_mode == BUDDHA_NEGATIVE_HYBRID
		&& has_negative_power(fractal->buddha))
		hybrid_source_rgb(fractal, x, y, rgb);
	else
		standard_source_rgb(fractal, x, y, rgb);
}

void	buddha_pixel_rgb(t_fractal *fractal, int x, int y, double rgb[3])
{
	double	sample[3];
	int		scale;
	int		sx;
	int		sy;
	int		channel;

	memset(rgb, 0, sizeof(double) * 3);
	scale = 1;
	if (fractal->supersample)
		scale = fractal->s_kernel;
	sy = y * scale - 1;
	while (++sy < (y + 1) * scale)
	{
		sx = x * scale - 1;
		while (++sx < (x + 1) * scale)
		{
			buddha_source_rgb(fractal, sx, sy, sample);
			channel = -1;
			while (++channel < 3)
				rgb[channel] += sample[channel];
		}
	}
	channel = -1;
	while (++channel < 3)
		rgb[channel] /= (double)(scale * scale);
}

static unsigned int	hybrid_pixel(t_fractal *fractal,
		t_hybrid_tone *tone, int x, int y)
{
	unsigned int	duotone;
	unsigned int	spill;
	double			structure;

	structure = hybrid_structure(fractal, tone, x, y);
	duotone = interpolate_rgb(tone->background,
			tone->foreground, structure);
	spill = hybrid_spill_color(fractal, tone, x, y);
	return (interpolate_rgb(duotone, spill,
			tone->spill_mix * structure));
}

static void	color_hybrid(t_fractal *fractal, t_piece *piece)
{
	t_hybrid_tone	tone;
	unsigned int	color;
	int				x;
	int				y;

	tone = get_hybrid_tone(fractal->buddha);
	y = piece->y_s - 1;
	while (++y < piece->y_e)
	{
		x = piece->x_s - 1;
		while (++x < piece->x_e)
		{
			color = hybrid_pixel(fractal, &tone, x, y);
			if (fractal->supersample)
				fractal->pixels_xl[y][x] = color;
			else
				my_pixel_put(x, y, &fractal->img, color);
		}
	}
}

static void	*set_colors(void *arg)
{
	t_piece		*piece;
	t_fractal	*fractal;

	piece = (t_piece *)arg;
	fractal = piece->fractal;
	if (fractal->buddha->negative_mode == BUDDHA_NEGATIVE_HYBRID
		&& has_negative_power(fractal->buddha))
	{
		color_hybrid(fractal, piece);
		return (NULL);
	}
	color_channel(fractal, piece, 0);
	color_channel(fractal, piece, 1);
	color_channel(fractal, piece, 2);
	if (use_post_spill_smoother(fractal->buddha))
		post_smooth_spill(fractal, piece);
	return (NULL);
}

static void	set_color_piece(t_fractal *fractal,
		t_piece piece[][fractal->num_cols], int x, int y)
{
	piece[y][x].x_s = x * (fractal->width / fractal->num_cols);
	piece[y][x].x_e = (x + 1) * (fractal->width / fractal->num_cols);
	piece[y][x].y_s = y * (fractal->height / fractal->num_rows);
	piece[y][x].y_e = (y + 1) * (fractal->height / fractal->num_rows);
	if (x + 1 == fractal->num_cols)
		piece[y][x].x_e = fractal->width;
	if (y + 1 == fractal->num_rows)
		piece[y][x].y_e = fractal->height;
	piece[y][x].fractal = fractal;
}

void	color_buddha(t_fractal *fractal)
{
	t_piece	piece[fractal->num_rows][fractal->num_cols];
	int		x;
	int		y;

	y = -1;
	while (++y < fractal->num_rows)
	{
		x = -1;
		while (++x < fractal->num_cols)
		{
			set_color_piece(fractal, piece, x, y);
			if (pthread_create(
					&fractal->threads[y * fractal->num_cols + x], NULL,
					set_colors, (void *)&piece[y][x]) != 0)
				thread_error(fractal, y * fractal->num_cols + x);
		}
	}
	join_threads(fractal->threads, fractal->num_rows, fractal->num_cols);
}
