/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   julia.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:09:19 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:09:22 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

void	julia_iteration(t_fractal *fractal, t_complex c, t_position p)
{
	t_complex	z;
	int			iterations;
	double		iter;
	double		aspect_ratio;

	aspect_ratio = (double)fractal->width / (double)fractal->height;

	double zoom = fractal->zoom * aspect_ratio;
    double zoom_orig = fractal->zoom;
	double move_x = fractal->move_x * aspect_ratio;
	double move_y = fractal->move_y;

	iterations = 0;
	z.x = (map(p.x, -2, 2, fractal->width) * zoom + move_x);
	z.y = (map(p.y, 2, -2, fractal->height) * zoom_orig - move_y);
	while ((z.x * z.x) + (z.y * z.y) < fractal->bound
		&& iterations < fractal->max_i)
	{
		z = sum_complex(square_complex(z), c);
		iterations++;
	}
	if (iterations >= fractal->max_i)
		iter = iterations;
	else
		iter = (double)iterations + 1 - \
			log(log(sqrt(z.x * z.x + z.y * z.y))) / log(2.0);
	color_option(fractal, p, iter);
}

void	*julia_set(void *arg)
{
	t_piece		*piece;
	t_fractal	*fractal;
	t_complex	c;
	t_position	p;

	piece = (t_piece *)arg;
	fractal = piece->fractal;
	p.y = piece->y_s - 1;
	while (++p.y < piece->y_e)
	{
		p.x = piece->x_s - 1;
		while (++p.x < piece->x_e)
		{
			c.x = fractal->julia_x;
			c.y = fractal->julia_y;
			julia_iteration(fractal, c, p);
		}
	}
	pthread_exit(NULL);
}
