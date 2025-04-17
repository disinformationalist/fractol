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

unsigned int rand_r_custom(unsigned int *seed)
{
    return rand_r(seed);
}

double randf_r(unsigned int *seed) //for other methods.
{
    return (double)rand_r_custom(seed) / (double)RAND_MAX;
}

int	ft_round(double d)
{
	return (floor(d + 0.5));
}

//random double between 0 and 1
double randf()
{
	return ((double)rand() / (double)RAND_MAX);
}

void	orbit_tracker(t_fractal *fractal, t_complex c, double sample_prob, int k, double slope_x, double slope_y, f complex_f)
{
	t_complex	z;
	int			x;
	int			y;
	int			iterations;
	double		weight = 1.0 / sample_prob;
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
		y = ft_round(map_back_2((z.x + move_x) * zoom, -2, slope_x));
		x = ft_round(map_back_2((z.y + move_y) * zoom, 2, slope_y));
		if (x >= 0 && x < fractal->width && y >= 0 && y < fractal->height)
		{
			//THIS MTX EATS TIME, without it mthread 3x+ faster than non, with it mthread is SLOWER, very low prob of data race, insignificant
			//other solution is to keep separate arrays for each thread and sum after thread join. Very memory intensive though

			//pthread_mutex_lock(&fractal->mutex); 
			fractal->densities[k][y][x] += weight;
			//pthread_mutex_unlock(&fractal->mutex);
		}
	}
}


void	buddha_iteration(t_fractal *fractal, t_complex c, double sample_prob, int k, double slope_x, double slope_y, f complex_f)
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
		orbit_tracker(fractal, c, sample_prob, k, slope_x, slope_y, complex_f);
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

void	*buddha_set(void *arg)//old method much slower.
{
	t_fractal	*fractal;
	t_complex	c;
	int 		i;
	int 		index;
	double		offset_x;
	double		offset_y;
	double 		sample_prob;
	double		width;
	double		height;
	double		samples;

	fractal = (t_fractal *)arg;
	width = (double)fractal->width;
	height = (double)fractal->height;
	f 			complex_f = fractal->complex_f;
	double 		slope_x = ((double)fractal->width - 1.0) / 4.0;//slope = (new_max - new_min) / (old_max - old_min);
	double 		slope_y = ((double)fractal->height - 1.0) / -4.0;//for the map_back...
	double 		slope_x1 = 4.0 / width;
	double 		slope_y1 = -4.0 / height;
	int 		hist = fractal->hist_num;

	samples = (double)fractal->size * SQ(fractal->buddha->n);
	samples /= (double)fractal->num_rows * (double)fractal->num_cols;

	i = -1;
	while (++i < samples)
	{
		//-------------------normal rand samp here
		pthread_mutex_lock(&fractal->rand_mtx);
		c.x =  randf() * 4.0 - 2.0; //randf() * (new_max - new_min) + new_min 
		//c.y =  randf() * (-2.0) + 2.0;//half and copy.
		c.y =  randf() * (-4.0) + 2.0;//normal range
		pthread_mutex_unlock(&fractal->rand_mtx);
		sample_prob = 1;
		buddha_iteration(fractal, c, sample_prob, hist, slope_x, slope_y, complex_f);
	}
	pthread_exit(NULL);
}
