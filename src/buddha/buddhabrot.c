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

t_complex invert_complex(t_complex z)
{
	t_complex	inv;
	double		denom = z.x * z.x + z.y * z.y;

	inv.x =  z.x / denom;
	inv.y = -z.y / denom;
	return inv;
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

void	orbit_tracker(t_fractal *fractal, t_complex c, double weight, t_comps comps)
{
	t_complex	z;
	int			x;
	int			y;
	int			iterations;

	iterations = 0;
	z.x = 0;
	z.y = 0;
	while (iterations < comps.b_max_i)
	{
		z = sum_complex(comps.complex_f(z), c);
		iterations++;
		//z = invert_complex(z);
		y = ft_round(map_back_2((z.x - comps.move_x) * comps.zoom, comps.x_cmin, comps.slopex_back));
		x = ft_round(map_back_2((z.y + comps.move_y) * comps.zoom, comps.y_cmin, comps.slopey_back));
		/* x = comps.width - x - 1;
		y = comps.height - y - 1; */
		
		
		
	/* y = ft_round((z.x - comps.move_x) / (comps.span / comps.zoom)) * comps.width;
		x = ft_round((z.y + comps.move_y) / (comps.span / comps.zoom)) * comps.height; */
		
		
/* 
		y = ft_round(((z.x - move_x) * zoom + 2.0) / 4.0 * fractal->width);
		x = ft_round(((z.y + move_y) * zoom - 2.0) / -4.0 * fractal->height); */
		/* y = ft_round(map_back_2(((z.x + move_x) * fractal->cos_a + (c.y + move_y) * fractal->sin_a) * zoom, -2, slope_x));
		x = ft_round(map_back_2(((z.y + move_y) * fractal->cos_b - (c.x + move_x) * fractal->sin_b) * zoom, 2, slope_y)); */
	/* 	if (x < 0 || x >= fractal->width || y < 0 || y >= fractal->height) {
			printf("Out of bounds at zoom=%.10f, z=(%.20f, %.20f)\n", zoom, z.x, z.y);//TRIM OUT BOUNDS BEFOR E CONVERT TO XY?	
		} */
		
		if (x >= 0 && x < comps.width && y >= 0 && y < comps.height)
		{
			//THIS MTX EATS TIME, without it mthread 3x+ faster than non, with it mthread is  3x+ SLOWER, very low prob of data race, insignificant
			//other solution is to keep separate arrays for each thread and sum after thread join. Very memory intensive though
			//pthread_mutex_lock(&fractal->mutex); 
			comps.density[y][x] += weight;// * (1.0 - (double)iterations / (double)bmax);
			//pthread_mutex_unlock(&fractal->mutex);
		}
	}
}

void	buddha_iteration(t_fractal *fractal, t_complex c, double weight, t_comps comps)
{
	t_complex	z;
	int			iterations;
	double		bound;

	bound = fractal->bound;
	//bound = SQ(comps.span);
	iterations = 0;
	z.x = 0.0;
	z.y = 0.0;
	while ((z.x * z.x) + (z.y * z.y) < bound && iterations < comps.b_max_i)
	{
		z = sum_complex(comps.complex_f(z), c);
		iterations++;
	}
	if (iterations < comps.b_max_i && iterations > fractal->b_min_i)
		orbit_tracker(fractal, c, weight, comps);
}

//check for escaping, pass to next is so, use brents for perodicity check(gauranteed not escaping, early exit)

/* void	buddha_iteration(t_fractal *fractal, t_complex c, double weight, int k, double slope_x, double slope_y, f complex_f)
{
	t_complex	z;
	int			iterations;
	int			max;

	t_complex	tort;
	t_complex	hare;
	int			power = 1;
	int			lam = 1;
	
	max = fractal->b_max_i;
	iterations = 0;
	z.x = 0.0;
	z.y = 0.0;
	tort = z;
	while ((z.x * z.x) + (z.y * z.y) < fractal->bound
		&& iterations < max)
	{
		hare = sum_complex(complex_f(z), c);
		if (fabs(hare.x - tort.x) < 1e-14 && fabs(hare.y - tort.y) < 1e-14)
		{
			iterations = max;
			break; // cycle detected
		}
		if (lam == power)
		{
			tort = hare;
			power *= 2;
			lam = 0;
		}
		iterations++;
		z = hare;
		lam++;
	}
	if (iterations < max && iterations > fractal->b_min_i)
		orbit_tracker(fractal, c, weight, k, slope_x, slope_y, complex_f);
} */


void	*buddha_set(void *arg)//normal rand method much slower.
{
	t_fractal	*fractal;
	t_piece		*piece;
	t_complex	c;
	t_comps		comps;
	int 		i;
	int 		index;
	double		samples;
	double		y_lim;
	Xoro128		*rng;
	
	piece = (t_piece *)arg;
	fractal = piece->fractal;
	comps = set_comps(fractal, false);
	/* if (!fractal->buddha->copy_half)
		y_lim = -comps.span;
	else
		y_lim = -comps.span / 2; */
	samples = comps.samples;
	samples /= (double)fractal->num_rows * (double)fractal->num_cols;
	rng = &piece->rng;
	i = -1;
	while (++i < samples)
	{
		/* c.x = (xoro128d(rng) * comps.span + comps.cmin) * comps.inv_zoom + comps.move_x; //randfloat * (new_max - new_min) + new_min 
		c.y = (xoro128d(rng) * y_lim - comps.cmin) * comps.inv_zoom - comps.move_y;//math opti these.... */
		c.x = (xoro128d(rng) * comps.x_span - comps.y_span / 2.0) + comps.move_x;//works... but doesnet us zoom at all
		c.y = (xoro128d(rng) * -comps.y_span + comps.y_span / 2.0) - comps.move_y;
		/* c.x = (map(p.x, -2, +2, (double)fractal->width) * inv_zoom + move_x);
		c.y = (map(p.y, +2, -2, (double)fractal->height) * inv_zoom - move_y); */
		buddha_iteration(fractal, c, 1.0, comps);
	}
	pthread_exit(NULL);
}
	/* 	if (iterations % 256)//for 256 period check, try floyds instead, but this is an improvement
		{
			if (pz.x == z.x && pz.y == z.y) 
			{
				iterations = bmax; 
				break; 
			}
		} 
		else 
		{
			pz.x = z.x;
			pz.y = z.y;
		} */
//old no perodicity checks
/* void	buddha_iteration(t_fractal *fractal, t_complex c, double weight, int k, double slope_x, double slope_y, f complex_f)
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
} */