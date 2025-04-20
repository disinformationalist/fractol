/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   buddhabrot.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:16:43 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:16:46 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	ft_round(double d)
{
	return (floor(d + 0.5));
}

//random double between 0 and 1
double randf()
{
	return ((double)rand() / (double)RAND_MAX);
}

void	orbit_tracker(t_fractal *fractal, t_complex c, double weight, int k, double slope_x, double slope_y, f complex_f)
{
	t_complex	z;
	int			x;
	int			y;
	int			iterations;
	double		move_x = fractal->move_x;
	double		move_y = fractal->move_y;
	double		zoom = fractal->zoom;
	int			bmax = fractal->b_max_i;

	iterations = 0;
	z.x = 0;
	z.y = 0;
	while (iterations < bmax)
	{
		z = sum_complex(complex_f(z), c);
		iterations++;
		y = ft_round(map_back_2((z.x - move_x) * zoom, -2, slope_x));
		x = ft_round(map_back_2((z.y + move_y) * zoom, 2, slope_y));
		/* y = ft_round(map_back_2(((z.x + move_x) * fractal->cos_a + (c.y + move_y) * fractal->sin_a) * zoom, -2, slope_x));
		x = ft_round(map_back_2(((z.y + move_y) * fractal->cos_b - (c.x + move_x) * fractal->sin_b) * zoom, 2, slope_y)); */
		if (x >= 0 && x < fractal->width && y >= 0 && y < fractal->height)
		{
			//THIS MTX EATS TIME, without it mthread 3x+ faster than non, with it mthread is  3x+ SLOWER, very low prob of data race, insignificant
			//other solution is to keep separate arrays for each thread and sum after thread join. Very memory intensive though

			//pthread_mutex_lock(&fractal->mutex); 
			fractal->densities[k][y][x] += weight;
			//pthread_mutex_unlock(&fractal->mutex);
		}
	}
}


void	buddha_iteration(t_fractal *fractal, t_complex c, double weight, int k, double slope_x, double slope_y, f complex_f)
{
	t_complex	z;
	int			iterations;
	int			max;

	max = fractal->b_max_i;
	iterations = 0;
	z.x = 0.0;
	z.y = 0.0;
	while ((z.x * z.x) + (z.y * z.y) < fractal->bound
		&& iterations < max)
	{
		z = sum_complex(complex_f(z), c);
		iterations++;
	}
	if (iterations < max && iterations > fractal->b_min_i)
		orbit_tracker(fractal, c, weight, k, slope_x, slope_y, complex_f);
}

int binary_search(double *cdf, int size, double value)
{
	int low = 0;
	int high = size - 1;
	int mid;

	while (low < high) 
	{
		mid = (high + low) >> 1;
		if (cdf[mid] < value)
			low = mid + 1;
		else 
			high = mid;
	}
	return (low);
}

void	importance_sample(t_fractal *fractal, int *s_x, int *s_y, int *index)
{
	double r = randf();    
	*index = binary_search(fractal->cdf, fractal->size, r);

    // Convert 1D index back to 2D coordinates
    *s_x = *index % fractal->width;
    *s_y = *index / fractal->width;
}

void	*buddha_set(void *arg)//normal rand method much slower.
{
	t_fractal	*fractal;
	t_piece		*piece;
	t_complex	c;
	int 		i;
	int 		index;
	double		move_x;
	double		move_y;
	double		samples;
	double		y_lim;
	Xoro128		*rng;
	double		inv_zoom;

	piece = (t_piece *)arg;
	fractal = piece->fractal;
	f 			complex_f = fractal->complex_f;
	double 		slope_x = ((double)fractal->width - 1.0) / 4.0;//slope = (new_max - new_min) / (old_max - old_min);
	double 		slope_y = ((double)fractal->height - 1.0) / -4.0;//for the map_back...
	int 		hist = fractal->hist_num;
	move_x = fractal->move_x;
	move_y = fractal->move_y;
	inv_zoom = 1.0f / fractal->zoom;
	if (!fractal->buddha->copy_half)
		y_lim = -4.0;
	else
		y_lim = -2.0;
	samples = (double)fractal->size * SQ(fractal->buddha->n);
	samples /= (double)fractal->num_rows * (double)fractal->num_cols;
	rng = &piece->rng;
	i = -1;
	while (++i < samples)
	{
		c.x = (xoro128d(rng) * 4.0 - 2.0) * inv_zoom + move_x; //randfloat * (new_max - new_min) + new_min 
		c.y = (xoro128d(rng) * y_lim + 2.0) * inv_zoom - move_y;//math opti these....
		buddha_iteration(fractal, c, 1.0, hist, slope_x, slope_y, complex_f);
	}
	pthread_exit(NULL);
}
