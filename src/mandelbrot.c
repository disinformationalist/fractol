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

void	put_pixel(int x, int y, t_fractal *fractal, unsigned int color)
{
	if (!fractal->supersample)
		my_pixel_put(x, y, &fractal->img, color);
	else
		fractal->pixels_xl[y][x] = color;
}

int	get_color(double x, double y, t_fractal *fractal)
{
	int	angle;
	
	angle = (int)floor((atan2(y, x) * RAD_TO_DEG + 360) + 360) % 360;
	return ((fractal->w_colors[(angle + fractal->col_i) % fractal->num_colors]));
}

void	color_option(t_fractal *fractal, t_position p, double iter)
{
	double		color;
	t_complex	d;

	if (iter == fractal->max_i)
		color = 0;
	else if (fractal->color_spectrum)
	{
		d.x = map(p.x, -2, +2, fractal->width) - fractal->move_col_x;
		d.y = map(p.y, +2, -2, fractal->height) + fractal->move_col_y;
		color = iter_color(iter, fractal->max_i, get_color(d.x, d.y, fractal));
	}
	else
		color = iter_color(iter, fractal->max_i, fractal->w_colors[fractal->col_i]);
	put_pixel(p.x, p.y, fractal, color);
}

//SIMD for mandel_set() and mandel_iteration(), normal versions below for clarity and experimenting.

void mandel_iteration_simd(t_fractal *fractal, __m256d cx, __m256d cy, double *iters)
{
    __m256d zx = _mm256_setzero_pd();
    __m256d zy = _mm256_setzero_pd();
    __m256d bound = _mm256_set1_pd(fractal->bound);
    __m256d iterations = _mm256_setzero_pd();
    __m256d one = _mm256_set1_pd(1.0);

    for (int i = 0; i < fractal->max_i; ++i)
    {
        __m256d zx2 = _mm256_mul_pd(zx, zx);
        __m256d zy2 = _mm256_mul_pd(zy, zy);
        __m256d zxy = _mm256_mul_pd(zx, zy);

        __m256d magnitude = _mm256_add_pd(zx2, zy2);
        __m256d mask = _mm256_cmp_pd(magnitude, bound, _CMP_LT_OS);

        int mask_int = _mm256_movemask_pd(mask);
        if (mask_int == 0) break;

        zx = _mm256_add_pd(_mm256_sub_pd(zx2, zy2), cx);
        zy = _mm256_add_pd(_mm256_add_pd(zxy, zxy), cy);

        iterations = _mm256_add_pd(iterations, _mm256_and_pd(one, mask));
    }

    _mm256_storeu_pd(iters, iterations);
}

void *mandel_set(void *arg)
{
    t_piece *piece = (t_piece *)arg;
    t_fractal *fractal = piece->fractal;

    double width = fractal->width;
    double height = fractal->height;
	double aspect_ratio;

	aspect_ratio = (double)fractal->width / (double)fractal->height;

    double zoom = fractal->zoom * aspect_ratio;
    double zoom_orig = fractal->zoom;
	double move_x = fractal->move_x * aspect_ratio;
    double move_y = fractal->move_y;
    for (int y = piece->y_s; y < piece->y_e; y++)
    {
        for (int x = piece->x_s; x < piece->x_e; x += 4) // Process 4 pixels at a time
        {
            __m256d cx = _mm256_set_pd(
           (map(x + 3, -2, +2, width) * zoom + move_x),
            (map(x + 2, -2, +2, width) * zoom + move_x),
            (map(x + 1, -2, +2, width) * zoom + move_x),
            (map(x, -2, +2, width) * zoom + move_x)
            );
            __m256d cy = _mm256_set1_pd(map(y, +2, -2, height) * zoom_orig - move_y);

            double iters[4];
            mandel_iteration_simd(fractal, cx, cy, iters);

            for (int i = 0; i < 4; ++i)
            {
                t_position p = { x + i, y };
                color_option(fractal, p, iters[i]);
            }
        }
    }
    pthread_exit(NULL);
}

// NORMAL versions

double	mandel_iteration(t_fractal *fractal, t_complex c)
{
	t_complex	z;
	int			iterations;
	//f complex_f = fractal->complex_f; // pass in , doesnt work right now

	iterations = 0;
	z.x = 0;
	z.y = 0;
	while ((z.x * z.x) + (z.y * z.y) < fractal->bound
		&& iterations < fractal->max_i)
	{
		z = sum_complex(square_complex(z), c);
		//z = sum_complex(complex_f(z), c);
		iterations++;
	}
	if (iterations >= fractal->max_i)
		return (iterations);
	return ((double)iterations + 1 - log(log(sqrt(z.x * z.x + z.y * z.y))) / log(2.0));
}

/* void	*mandel_set(void *arg)
{
	t_piece		*piece;
	t_fractal	*fractal;
	t_complex	c;
	t_position	p;
	double		iter;

	piece = (t_piece *)arg;
	fractal = piece->fractal;

	double width = fractal->width;
	double height = fractal->height;
	double aspect_ratio;

	aspect_ratio = (double)fractal->width / (double)fractal->height;

	double zoom = fractal->zoom * aspect_ratio;
    double zoom_orig = fractal->zoom;
	double move_x = fractal->move_x * aspect_ratio;
    double move_y = fractal->move_y;

	p.y = piece->y_s - 1;
	while (++p.y < piece->y_e)
	{
		p.x = piece->x_s - 1;
		while (++p.x < piece->x_e)
		{
			c.x = (map(p.x, -2, +2, width) * zoom + move_x);
			c.y = (map(p.y, +2, -2, height) * zoom_orig - move_y);
			iter = mandel_iteration(fractal, c);
			color_option(fractal, p, iter);
		}
	}
	pthread_exit(NULL);
}  */