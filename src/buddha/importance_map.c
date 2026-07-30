#include "fractol.h"

size_t	buddha_sample_budget(t_fractal *fractal)
{
	double	budget;
	double	buffers;

	buffers = (double)fractal->buffs;
	if (buffers < 1.0)
		buffers = 1.0;
	budget = (double)fractal->size * fractal->buddha->n
		* fractal->buddha->n / buffers;
	if (!isfinite(budget)
		|| budget >= (double)UINT32_MAX + 0.5)
		return (SIZE_MAX);
	if (budget < (double)fractal->size)
		return ((size_t)fractal->size);
	return ((size_t)llround(budget));
}

static size_t	uniform_extra_target(size_t index, size_t remaining,
		size_t size)
{
	return ((size_t)((long double)(index + 1)
		* (long double)remaining / (long double)size));
}

void	build_importance_map(t_fractal *fractal, double **density)
{
	long double	importance_sum;
	long double	cumulative_importance;
	size_t		budget;
	size_t		extra_target;
	size_t		previous_extra;
	size_t		remaining;
	size_t		index;
	double		importance;

	importance_sum = 0.0L;
	index = 0;
	while (index < (size_t)fractal->size)
	{
		importance = density[index / (size_t)fractal->width]
		[index % (size_t)fractal->width];
		if (isfinite(importance) && importance > 0.0)
			importance_sum += (long double)importance;
		index++;
	}
	budget = buddha_sample_budget(fractal);
	remaining = budget;
	cumulative_importance = 0.0L;
	previous_extra = 0;
	index = 0;
	while (index < (size_t)fractal->size)
	{
		importance = density[index / (size_t)fractal->width]
		[index % (size_t)fractal->width];
		if (isfinite(importance) && importance > 0.0)
			cumulative_importance += (long double)importance;
		if (importance_sum > 0.0L)
		{
			extra_target = (size_t)(cumulative_importance
					* (long double)remaining / importance_sum);
			if (cumulative_importance >= importance_sum)
				extra_target = remaining;
		}
		else
			extra_target = uniform_extra_target(index, remaining,
					(size_t)fractal->size);
		if (extra_target < previous_extra)
			extra_target = previous_extra;
		if (extra_target > remaining)
			extra_target = remaining;
		fractal->sample_counts[index]
			= (uint32_t)(extra_target - previous_extra);
		previous_extra = extra_target;
		index++;
	}
}

void	buddha_importance_begin(t_fractal *fractal)
{
	t_buddha	*b;
	size_t		pixels;

	b = fractal->buddha;
	b->importance_ready = false;
	if (!b->importance_enabled)
		return ;
	pixels = (size_t)fractal->width_orig * (size_t)fractal->height_orig;
	if (!b->importance_values)
		b->importance_values = malloc(pixels * BUDDHA_CHANNELS
				* sizeof(*b->importance_values));
	if (!b->render_pixels)
		b->render_pixels = malloc(pixels * sizeof(*b->render_pixels));
	if (!b->importance_values || !b->render_pixels)
	{
		free(b->importance_values);
		free(b->render_pixels);
		b->importance_values = NULL;
		b->render_pixels = NULL;
		b->importance_enabled = false;
		fprintf(stderr, "Importance-map capture disabled: allocation failed\n");
		return ;
	}
	memset(b->importance_values, 0, pixels * BUDDHA_CHANNELS
		* sizeof(*b->importance_values));
}

static double	map_block_value(t_fractal *fractal, double **density,
		int out_x, int out_y, double log_high)
{
	double	sum;
	double	value;
	int		scale_x;
	int		scale_y;
	int		x;
	int		y;

	scale_x = fractal->width / fractal->width_orig;
	scale_y = fractal->height / fractal->height_orig;
	if (scale_x < 1)
		scale_x = 1;
	if (scale_y < 1)
		scale_y = 1;
	sum = 0.0;
	y = out_y * scale_y - 1;
	while (++y < (out_y + 1) * scale_y && y < fractal->height)
	{
		x = out_x * scale_x - 1;
		while (++x < (out_x + 1) * scale_x && x < fractal->width)
		{
			value = density[y][x];
			if (isfinite(value) && value > 0.0)
				sum += log1p(value) / log_high;
		}
	}
	sum /= (double)(scale_x * scale_y);
	if (sum < 0.0)
		sum = 0.0;
	if (sum > 1.0)
		sum = 1.0;
	return (sum);
}

void	buddha_importance_capture(t_fractal *fractal, int histogram)
{
	t_buddha		*b;
	double			high;
	double			log_high;
	double			normalized;
	size_t			index;
	int				x;
	int				y;

	b = fractal->buddha;
	if (!b->importance_enabled || !b->importance_values)
		return ;
	high = high_hit_count(fractal->width, fractal->height,
			fractal->densities[histogram]);
	if (high <= 0.0)
	{
		if (histogram == 2)
			b->importance_ready = true;
		return ;
	}
	log_high = log1p(high);
	y = -1;
	while (++y < fractal->height_orig)
	{
		x = -1;
		while (++x < fractal->width_orig)
		{
			index = (size_t)y * (size_t)fractal->width_orig + (size_t)x;
			normalized = map_block_value(fractal,
					fractal->densities[histogram],
					x, y, log_high);
			b->importance_values[index * BUDDHA_CHANNELS
				+ (size_t)histogram]
				= (uint16_t)floor(normalized * 65535.0 + 0.5);
		}
	}
	if (histogram == 2)
		b->importance_ready = true;
}

char	*buddha_importance_mode_name(t_buddha_importance_mode mode)
{
	if (mode == BUDDHA_IMPORTANCE_BLUE)
		return ("blue");
	if (mode == BUDDHA_IMPORTANCE_GREEN)
		return ("green");
	if (mode == BUDDHA_IMPORTANCE_RED)
		return ("red");
	return ("RGB");
}

void	buddha_importance_pixel_rgb(t_fractal *fractal, int x, int y,
		double rgb[3])
{
	t_buddha	*b;
	uint16_t	*value;
	size_t		index;

	memset(rgb, 0, sizeof(double) * BUDDHA_CHANNELS);
	b = fractal->buddha;
	if (!b || !b->importance_values || x < 0 || y < 0
		|| x >= fractal->width_orig || y >= fractal->height_orig)
		return ;
	index = (size_t)y * (size_t)fractal->width_orig + (size_t)x;
	value = b->importance_values + index * BUDDHA_CHANNELS;
	if (b->importance_mode == BUDDHA_IMPORTANCE_RGB
		|| b->importance_mode == BUDDHA_IMPORTANCE_RED)
		rgb[0] = (double)value[2] / 65535.0;
	if (b->importance_mode == BUDDHA_IMPORTANCE_RGB
		|| b->importance_mode == BUDDHA_IMPORTANCE_GREEN)
		rgb[1] = (double)value[1] / 65535.0;
	if (b->importance_mode == BUDDHA_IMPORTANCE_RGB
		|| b->importance_mode == BUDDHA_IMPORTANCE_BLUE)
		rgb[2] = (double)value[0] / 65535.0;
}

void	buddha_importance_draw(t_fractal *fractal)
{
	double	rgb[3];
	unsigned int	color;
	int		x;
	int		y;

	if (!fractal->buddha->importance_ready)
		return ;
	y = -1;
	while (++y < fractal->height_orig)
	{
		x = -1;
		while (++x < fractal->width_orig)
		{
			buddha_importance_pixel_rgb(fractal, x, y, rgb);
			color = (unsigned int)ft_round(rgb[0] * 255.0) << 16;
			color |= (unsigned int)ft_round(rgb[1] * 255.0) << 8;
			color |= (unsigned int)ft_round(rgb[2] * 255.0);
			my_pixel_put(x, y, &fractal->img, color);
		}
	}
}

void	buddha_importance_store_render(t_fractal *fractal)
{
	t_buddha	*b;
	size_t		index;
	int			x;
	int			y;

	b = fractal->buddha;
	if (!b->importance_enabled || !b->render_pixels)
		return ;
	index = 0;
	y = -1;
	while (++y < fractal->height_orig)
	{
		x = -1;
		while (++x < fractal->width_orig)
			b->render_pixels[index++]
				= pixel_color_get(x, y, &fractal->img);
	}
}

static void	draw_saved_render(t_fractal *fractal)
{
	size_t	index;
	int		x;
	int		y;

	index = 0;
	y = -1;
	while (++y < fractal->height_orig)
	{
		x = -1;
		while (++x < fractal->width_orig)
			my_pixel_put(x, y, &fractal->img,
				fractal->buddha->render_pixels[index++]);
	}
}

bool	buddha_importance_toggle(t_fractal *fractal)
{
	t_buddha	*b;

	b = fractal->buddha;
	if (!b->importance_enabled || !b->importance_ready)
	{
		printf("Importance map was not captured. Start with "
			"--importance-map or FRACTOL_BUDDHA_IMPORTANCE=1.\n");
		return (false);
	}
	b->importance_view = !b->importance_view;
	if (b->importance_view)
	{
		buddha_importance_draw(fractal);
		printf("Viewing %s importance map%s (log-scaled per channel).\n",
			buddha_importance_mode_name(b->importance_mode),
			(char *[2]){"", "s"}
			[b->importance_mode == BUDDHA_IMPORTANCE_RGB]);
	}
	else
	{
		draw_saved_render(fractal);
		printf("Viewing Buddhabrot render.\n");
	}
	display_fractal_image(fractal);
	return (true);
}

bool	buddha_importance_cycle(t_fractal *fractal)
{
	t_buddha	*b;

	b = fractal->buddha;
	if (!b->importance_enabled || !b->importance_ready)
	{
		printf("Importance map was not captured. Start with "
			"--importance-map or FRACTOL_BUDDHA_IMPORTANCE=1.\n");
		return (false);
	}
	if (!b->importance_view)
	{
		printf("Press M to display the importance maps before cycling.\n");
		return (false);
	}
	b->importance_mode = (b->importance_mode + 1)
		% BUDDHA_IMPORTANCE_MODE_COUNT;
	buddha_importance_draw(fractal);
	display_fractal_image(fractal);
	printf("Viewing %s importance map%s (log-scaled per channel).\n",
		buddha_importance_mode_name(b->importance_mode),
		(char *[2]){"", "s"}[b->importance_mode == BUDDHA_IMPORTANCE_RGB]);
	return (true);
}
