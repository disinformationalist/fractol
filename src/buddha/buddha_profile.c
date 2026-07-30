#include "fractol.h"

bool	buddha_profile_enabled(void)
{
	char	*value;

	value = getenv("FRACTOL_BUDDHA_PROFILE");
	return (value != NULL && value[0] != '\0'
		&& !(value[0] == '0' && value[1] == '\0'));
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
