#include "fractol.h"

static void	swap_double(double *a, double *b)
{
	double	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

static double	select_value(double *values, size_t count, size_t target)
{
	size_t	left;
	size_t	right;
	size_t	low;
	size_t	i;
	size_t	high;
	double	pivot;

	left = 0;
	right = count - 1;
	while (left < right)
	{
		pivot = values[left + (right - left) / 2];
		low = left;
		i = left;
		high = right;
		while (i <= high)
		{
			if (values[i] < pivot)
				swap_double(&values[i++], &values[low++]);
			else if (values[i] > pivot)
				swap_double(&values[i], &values[high--]);
			else
				i++;
		}
		if (target < low)
			right = low - 1;
		else if (target > high)
			left = high + 1;
		else
			return (values[target]);
	}
	return (values[left]);
}

static double	max_density(double **density, int width, int height)
{
	double	high;
	int		x;
	int		y;

	high = 0.0;
	y = -1;
	while (++y < height)
	{
		x = -1;
		while (++x < width)
			if (density[y][x] > high)
				high = density[y][x];
	}
	return (high);
}

static double	select_percentile(double *values, size_t count,
		double percentile)
{
	size_t	target;
	size_t	index;
	size_t	positive;
	double	result;

	target = (size_t)ceil(percentile * (double)count);
	if (target == 0)
		target = 1;
	if (target > count)
		target = count;
	result = select_value(values, count, target - 1);
	if (result > 0.0)
		return (result);
	positive = 0;
	index = 0;
	while (index < count)
	{
		if (values[index] > 0.0)
			values[positive++] = values[index];
		index++;
	}
	if (positive == 0)
		return (0.0);
	target = (size_t)ceil(percentile * (double)positive);
	if (target == 0)
		target = 1;
	if (target > positive)
		target = positive;
	return (select_value(values, positive, target - 1));
}

double	buddha_density_percentile(double **density, int width,
		int height, double percentile)
{
	double	*values;
	size_t	count;
	size_t	index;
	double	result;
	int		x;
	int		y;

	if (!density || width <= 0 || height <= 0)
		return (0.0);
	if (percentile <= 0.0)
		percentile = BUDDHA_WHITE_PERCENTILE;
	if (percentile > 1.0)
		percentile = 1.0;
	count = (size_t)width * (size_t)height;
	values = malloc(count * sizeof(*values));
	if (!values)
		return (max_density(density, width, height));
	index = 0;
	y = -1;
	while (++y < height)
	{
		x = -1;
		while (++x < width)
			values[index++] = density[y][x];
	}
	result = select_percentile(values, count, percentile);
	free(values);
	return (result);
}

double	buddha_density_percentile_rgb(double ***densities, int width,
		int height, double percentile)
{
	double	*values;
	double	value;
	double	result;
	size_t	count;
	size_t	index;
	int		channel;
	int		x;
	int		y;

	if (!densities || width <= 0 || height <= 0)
		return (0.0);
	if (percentile <= 0.0)
		percentile = BUDDHA_WHITE_PERCENTILE;
	if (percentile > 1.0)
		percentile = 1.0;
	count = (size_t)width * (size_t)height;
	values = malloc(count * sizeof(*values));
	if (!values)
	{
		result = 0.0;
		channel = -1;
		while (++channel < 3)
		{
			value = max_density(densities[channel], width, height);
			if (value > result)
				result = value;
		}
		return (result);
	}
	index = 0;
	y = -1;
	while (++y < height)
	{
		x = -1;
		while (++x < width)
		{
			value = densities[0][y][x];
			channel = 0;
			while (++channel < 3)
				if (densities[channel][y][x] > value)
					value = densities[channel][y][x];
			values[index++] = value;
		}
	}
	result = select_percentile(values, count, percentile);
	free(values);
	return (result);
}

double	buddha_smootherstep_value(double edge0, double edge1, double value)
{
	double	t;

	if (edge1 <= edge0)
		return (value >= edge1);
	t = (value - edge0) / (edge1 - edge0);
	if (t < 0.0)
		t = 0.0;
	else if (t > 1.0)
		t = 1.0;
	return (t * t * t * (t * (6.0 * t - 15.0) + 10.0));
}

double	buddha_tone_value(double density, double white_point,
		double power, double edge0, double edge1, double exposure)
{
	double	factor;

	if (density <= 0.0 || white_point <= 0.0
		|| power == 0.0 || edge1 <= 0.0 || exposure <= 0.0)
		return (0.0);
	factor = buddha_curve_value(density, white_point, power) * exposure;
	return (buddha_smootherstep_value(edge0, edge1, factor));
}

double	buddha_curve_value(double density, double reference, double power)
{
	double	ratio;

	if (density <= 0.0 || reference <= 0.0 || power == 0.0)
		return (0.0);
	if (power > 0.0)
		return (pow(density, power) / pow(reference, power));
	ratio = density / reference;
	if (ratio > 1.0)
		ratio = 1.0;
	if (ratio < 0.0)
		ratio = 0.0;
	return (1.0 - pow(ratio, -power));
}

double	buddha_adjust_power(double power, double direction, bool fine,
		bool extra_fine)
{
	double	step;
	double	next;

	if (direction == 0.0)
		return (power);
	direction = copysign(1.0, direction);
	step = 0.05;
	if (fine)
		step = 0.01;
	if (extra_fine)
		step = 0.001;
	next = power + direction * step;
	if ((power < 0.0 && next >= 0.0)
		|| (power > 0.0 && next <= 0.0) || power == 0.0)
		next = direction * step;
	if (next > 4.0)
		next = 4.0;
	if (next < -4.0)
		next = -4.0;
	return (next);
}

unsigned int	buddha_legacy_spill_color(double density, double reference,
		double power, double exposure, unsigned int shift)
{
	double	modulus;
	double	rounded;
	double	value;

	if (density <= 0.0 || reference <= 0.0 || power >= 0.0
		|| exposure <= 0.0 || shift > 16)
		return (0);
	value = 255.0 * pow(reference / density, -power) * exposure;
	if (!isfinite(value))
		return (0);
	rounded = floor(value + 0.5);
	modulus = ldexp(1.0, 24 - shift);
	value = fmod(rounded, modulus);
	if (value < 0.0)
		value += modulus;
	return (((unsigned int)value << shift) & 0x00FFFFFFu);
}

char	*buddha_normalization_name(t_buddha_normalization normalization)
{
	if (normalization == BUDDHA_NORM_LINKED_PERCENTILE)
		return ("linked percentile");
	if (normalization == BUDDHA_NORM_LEGACY_MAX)
		return ("legacy channel maxima");
	return ("channel percentiles");
}

char	*buddha_negative_mode_name(t_buddha_negative_mode mode)
{
	if (mode == BUDDHA_NEGATIVE_SPILL)
		return ("legacy channel spill");
	if (mode == BUDDHA_NEGATIVE_HYBRID)
		return ("hybrid duotone + spill");
	return ("safe inverse");
}

static t_buddha_tone_state	capture_tone_state(t_buddha *b)
{
	t_buddha_tone_state	state;

	state.power[0] = b->bpow;
	state.power[1] = b->gpow;
	state.power[2] = b->rpow;
	state.edge0[0] = b->edge0_b;
	state.edge0[1] = b->edge0_g;
	state.edge0[2] = b->edge0_r;
	state.edge1[0] = b->edge1_b;
	state.edge1[1] = b->edge1_g;
	state.edge1[2] = b->edge1_r;
	state.exposure[0] = b->exposure_b;
	state.exposure[1] = b->exposure_g;
	state.exposure[2] = b->exposure_r;
	state.global_exposure = b->global_exposure;
	state.white_percentile = b->white_percentile;
	state.normalization = b->normalization;
	state.negative_mode = b->negative_mode;
	state.hybrid_spill = b->hybrid_spill;
	state.hybrid_background = b->hybrid_background;
	state.hybrid_foreground = b->hybrid_foreground;
	state.smootherstep = b->smootherstep;
	return (state);
}

static void	apply_tone_state(t_buddha *b, t_buddha_tone_state state)
{
	b->bpow = state.power[0];
	b->gpow = state.power[1];
	b->rpow = state.power[2];
	b->edge0_b = state.edge0[0];
	b->edge0_g = state.edge0[1];
	b->edge0_r = state.edge0[2];
	b->edge1_b = state.edge1[0];
	b->edge1_g = state.edge1[1];
	b->edge1_r = state.edge1[2];
	b->exposure_b = state.exposure[0];
	b->exposure_g = state.exposure[1];
	b->exposure_r = state.exposure[2];
	b->global_exposure = state.global_exposure;
	b->white_percentile = state.white_percentile;
	b->normalization = state.normalization;
	b->negative_mode = state.negative_mode;
	b->hybrid_spill = state.hybrid_spill;
	b->hybrid_background = state.hybrid_background;
	b->hybrid_foreground = state.hybrid_foreground;
	b->smootherstep = state.smootherstep;
}

void	buddha_tone_store_defaults(t_buddha *b)
{
	b->tone_default = capture_tone_state(b);
	b->tone_alternate = b->tone_default;
}

void	buddha_tone_reset(t_buddha *b)
{
	apply_tone_state(b, b->tone_default);
}

void	buddha_tone_swap_alternate(t_buddha *b)
{
	t_buddha_tone_state	current;

	current = capture_tone_state(b);
	apply_tone_state(b, b->tone_alternate);
	b->tone_alternate = current;
}

void	buddha_update_white_points(t_fractal *fractal)
{
	double	linked;

	if (!fractal || !fractal->buddha || !fractal->densities)
		return ;
	buddha_set_channel_statistics(fractal, 0);
	buddha_set_channel_statistics(fractal, 1);
	buddha_set_channel_statistics(fractal, 2);
	if (fractal->buddha->normalization != BUDDHA_NORM_LINKED_PERCENTILE)
		return ;
	linked = buddha_density_percentile_rgb(fractal->densities,
			fractal->width, fractal->height,
			fractal->buddha->white_percentile);
	fractal->buddha->white_b = linked;
	fractal->buddha->white_g = linked;
	fractal->buddha->white_r = linked;
}
