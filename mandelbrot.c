/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mandelbrot.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:08:55 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:08:57 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	get_color(double x, double y, t_fractal *fractal)
{
	int	angle;

	angle = (int)floor((atan2(y, x) * 180.0 / M_PI + 360) + 360) % 360;
	return ((fractal->co->colors[(angle + fractal->col_i) % 360]));
}

void	color_option(t_fractal *fractal, t_position p, double iter)
{
	double		a;
	double		b;
	double		color;
	t_complex	d;

	if (iter == fractal->max_i)
		color = 0;
	else if (fractal->color_spectrum)
	{
		d.x = map(p.x, -2, +2, WIDTH) - fractal->move_col_x;
		d.y = map(p.y, +2, -2, HEIGHT) + fractal->move_col_y;
		color = iter_color(iter, fractal->max_i, get_color(d.x, d.y, fractal));
	}
	else
		color = iter_color(iter, fractal->max_i, \
		fractal->co->colors[fractal->col_i]);
	my_pixel_put(p.x, p.y, &fractal->img, color);
}

double	mandel_iteration(t_fractal *fractal, t_complex c)
{
	t_complex	z;
	int			iterations;

	iterations = 0;
	z.x = 0;
	z.y = 0;
	while ((z.x * z.x) + (z.y * z.y) < fractal->bound
		&& iterations < fractal->max_i)
	{
		z = sum_complex(square_complex(z), c);
		iterations++;
	}
	if (iterations >= fractal->max_i)
		return (iterations);
	return ((double)iterations + 1 - \
	log(log(sqrt(z.x * z.x + z.y * z.y))) / log(2.0));
}

void	*mandel_set(void *arg)
{
	t_piece		*piece;
	t_fractal	*fractal;
	t_complex	c;
	t_position	p;
	double		iter;

	piece = (t_piece *)arg;
	fractal = piece->fractal;
	p.y = piece->y_s - 1;
	while (++p.y < piece->y_e)
	{
		p.x = piece->x_s - 1;
		while (++p.x < piece->x_e)
		{
			c.x = map(p.x, -2, +2, WIDTH) * fractal->zoom + fractal->move_x;
			c.y = map(p.y, +2, -2, HEIGHT) * fractal->zoom - fractal->move_y;
			iter = mandel_iteration(fractal, c);
			color_option(fractal, p, iter);
		}
	}
	pthread_exit(NULL);
}
