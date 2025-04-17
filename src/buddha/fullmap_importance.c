# include "fractol.h"

//This version not in use, kept for experimentation


//filling out densities for all channels in one pass.

void	increase_counts(t_fractal *fractal, int k, int j, int i, double count_hits)
{
	if (k == 0)
	{
		pthread_mutex_lock(&fractal->mutex);
			fractal->densities[0][j][i] += count_hits;
			fractal->densities[1][j][i] += count_hits;
			fractal->densities[2][j][i] += count_hits;
		pthread_mutex_unlock(&fractal->mutex);
	}
	else if (k == 1)
	{
		pthread_mutex_lock(&fractal->mutex);
			fractal->densities[1][j][i] += count_hits;
			fractal->densities[2][j][i] += count_hits;
		pthread_mutex_unlock(&fractal->mutex);
	}
	else if (k == 2)
	{
		pthread_mutex_lock(&fractal->mutex);
			fractal->densities[2][j][i] += count_hits;
		pthread_mutex_unlock(&fractal->mutex);	
	}
}

void	track_importance_full(t_fractal *fractal, t_complex c, int k, int b_max, double slope_x, double slope_y, f complex_f)
{
	t_complex	z;
	int			x;
	int			y;
	int			iterations;
	double			count_hits;
	int			i;
	int			j;
	double		move_x = fractal->move_x;
	double		move_y = fractal->move_y;
	double		zoom = fractal->zoom;

	iterations = 0;
	count_hits = 0;
	z.x = 0;
	z.y = 0;
	while (iterations < b_max)
	{
		z = sum_complex(complex_f(z), c);
		iterations++;
		
		/* x = ft_round(map_back_2(((z.x + move_x) * fractal->cos_a + (c.y + move_y) * fractal->sin_a) * zoom, -2, slope_x));//rotations, pitch yaw
		y = ft_round(map_back_2(((z.y + move_y) * fractal->cos_b - (c.x + move_x) * fractal->sin_b) * zoom, 2, slope_y)); */

		x = ft_round(map_back_2((z.x + move_x) * zoom, -2, slope_x));
		y = ft_round(map_back_2((z.y + move_y) * zoom, 2, slope_y));
		
		//x = ft_round(map_back((z.x + move_x) * zoom, fractal->width - 1, -2, 2));
		//y = ft_round(map_back((z.y + move_y) * zoom, fractal->height - 1, 2, -2));
		if (x >= 0 && x < fractal->width && y >= 0 && y < fractal->height)
			count_hits++;
	}
	i = ft_round(map_back_2(c.x, -2, slope_x));//norm
	j = ft_round(map_back_2(c.y, 2, slope_y));
	
	increase_counts(fractal, k, j, i, count_hits);
}

void	buddha_iter_bound(t_fractal *fractal, t_complex *z, t_complex c, int b_max, int *iters, f complex_f)
{
	int bound = fractal->bound;
	
	while ((z->x * z->x) + (z->y * z->y) < bound && *iters < b_max)
	{
		*z = sum_complex(complex_f(*z), c);
		(*iters)++;
	}
}

//full map optimizes mapping but reduces flexibility, map time is relatively short though.
//Not currently in use.

void	buddha_iter_fullmap(t_fractal *fractal, t_complex c, double slope_x, double slope_y, f complex_f)
{
	t_complex	z;
	int			iterations;
	
	z.x = 0.0;
	z.y = 0.0;
	iterations = 0;
	buddha_iter_bound(fractal, &z, c, fractal->buddha->max1, &iterations, complex_f);
	if (iterations < fractal->buddha->max1 && iterations > fractal->buddha->min1)
		track_importance_full(fractal, c, 0, fractal->buddha->max1, slope_x, slope_y, complex_f);
	else
	{
		buddha_iter_bound(fractal, &z, c, fractal->buddha->max2, &iterations, complex_f);
		if (iterations < fractal->buddha->max2 && iterations > fractal->buddha->min2)
			track_importance_full(fractal, c, 1, fractal->buddha->max2, slope_x, slope_y, complex_f);
		else
		{
			buddha_iter_bound(fractal, &z, c, fractal->buddha->max3, &iterations, complex_f);
			if (iterations < fractal->buddha->max3 && iterations > fractal->buddha->min3)
				track_importance_full(fractal, c, 2, fractal->buddha->max3, slope_x, slope_y, complex_f);
		}
	}
}