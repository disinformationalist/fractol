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

void	orbit_tracker(t_fractal *fractal, t_complex c)
{
	t_complex	z;
	int			x;
	int			y;
	int			iterations;

	iterations = 0;
	z.x = 0;
	z.y = 0;
	while ((z.x * z.x) + (z.y * z.y) < fractal->bound
		&& iterations < fractal->b_max_i)
	{
		z = sum_complex(square_complex(z), c);
		iterations++;
		y = map_back(z.x, WIDTH - 1, -2, 2);
		x = map_back(z.y, HEIGHT - 1, 2, -2);
		if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
		{
			pthread_mutex_lock(&fractal->mutex);
			fractal->density[x][y] += 1;
			pthread_mutex_unlock(&fractal->mutex);
		}
	}
}

void	buddha_iteration(t_fractal *fractal, t_complex c)
{
	t_complex	z;
	int			iterations;

	iterations = 0;
	z.x = 0;
	z.y = 0;
	while ((z.x * z.x) + (z.y * z.y) < fractal->bound
		&& iterations < fractal->b_max_i)
	{
		z = sum_complex(square_complex(z), c);
		iterations++;
	}
	if (iterations < fractal->b_max_i)
		orbit_tracker(fractal, c);
}

void	*buddha_set(void *arg)
{
	t_piece		*piece;
	t_fractal	*fractal;
	t_complex	c;
	int			x;
	int			y;

	piece = (t_piece *)arg;
	fractal = piece->fractal;
	y = piece->y_s - 1;
	while (++y < piece->y_e)
	{
		x = piece->x_s - 1;
		while (++x < piece->x_e)
		{
			c.x = map(x, -2, +2, WIDTH * fractal->n);
			c.y = map(y, +2, -2, HEIGHT * fractal->n);
			buddha_iteration(fractal, c);
		}
	}
	pthread_exit(NULL);
}
