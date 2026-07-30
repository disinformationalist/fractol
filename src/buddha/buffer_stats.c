#include "fractol.h"

double	high_hit_count(int width, int height, double **density)
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

void	combine_buff_set_var(double ***densities, int histogram,
		int buffers, int width, int height)
{
	double	difference;
	double	mean;
	double	sum;
	int		buffer;
	int		x;
	int		y;

	y = -1;
	while (++y < height)
	{
		x = -1;
		while (++x < width)
		{
			sum = 0.0;
			buffer = -1;
			while (++buffer < buffers)
				sum += densities[histogram + buffer * 3][y][x];
			mean = sum / (double)buffers;
			if (buffers <= 1)
			{
				densities[histogram][y][x] = mean;
				continue ;
			}
			sum = 0.0;
			buffer = -1;
			while (++buffer < buffers)
			{
				difference = densities[histogram + buffer * 3][y][x] - mean;
				sum += difference * difference;
			}
			densities[histogram + 3][y][x]
				= sum / (double)(buffers - 1);
			densities[histogram][y][x] = mean;
		}
	}
}

void	buddha_set_channel_statistics(t_fractal *fractal, int histogram)
{
	double	high;
	double	white;

	high = high_hit_count(fractal->width, fractal->height,
			fractal->densities[histogram]);
	if (fractal->buddha->normalization == BUDDHA_NORM_CHANNEL_PERCENTILE)
		white = buddha_density_percentile(fractal->densities[histogram],
				fractal->width, fractal->height,
				fractal->buddha->white_percentile);
	else
		white = high;
	if (histogram == 0)
	{
		fractal->buddha->high_b = high;
		fractal->buddha->white_b = white;
	}
	else if (histogram == 1)
	{
		fractal->buddha->high_g = high;
		fractal->buddha->white_g = white;
	}
	else
	{
		fractal->buddha->high_r = high;
		fractal->buddha->white_r = white;
	}
}
