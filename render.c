/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:08:05 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:08:09 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

void	my_pixel_put_plus(int x, int y, t_img *img, int color)
{
	int				offset;
	unsigned int	current_color;

	offset = (y * img->line_len) + (x * (img->bpp / 8));
	current_color = *(unsigned int *)(img->pixels_ptr + offset);
	*(unsigned int *)(img->pixels_ptr + offset) = current_color | color;
}

int	high_hit_count(int **density)
{
	int	i;
	int	j;
	int	high;

	i = -1;
	high = 0;
	while (++i < WIDTH)
	{
		j = -1;
		while (++j < HEIGHT)
		{
			if (high < density[i][j])
				high = density[i][j];
		}
	}
	return (high);
}

void	set_color(t_fractal *fractal, double num, char channel)
{
	int		color;
	int		i;
	int		j;
	double	n;
	double	factor;

	i = -1;
	n = 255.0 / num;
	while (++i < WIDTH)
	{
		j = -1;
		while (++j < HEIGHT)
		{
			factor = (double)fractal->density[i][j] * n;
			if (channel == 'r')
				color = (int)(factor) << 16;
			else if (channel == 'g')
				color = (int)(factor) << 8;
			else if (channel == 'b')
				color = (int)(factor);
			my_pixel_put_plus(i, j, &fractal->img, color);
		}
	}
}

void	run_and_reset(t_fractal *fractal, int buddha_iters, char channel)
{
	double	num;

	fractal->b_max_i = buddha_iters;
	buddha(fractal);
	num = high_hit_count(fractal->density);
	set_color(fractal, num, channel);
	zero_density(fractal);
}

void	render(t_fractal *fractal)
{
	if (!ft_strncmp(fractal->name, "mandelbrot", 10))
		mandelbrot(fractal);
	else if (!ft_strncmp(fractal->name, "julia", 5))
		julia(fractal);
	else if (!ft_strncmp(fractal->name, "buddha", 6))
	{
		fractal->n = 8;
		run_and_reset(fractal, 10000, 'r');
		run_and_reset(fractal, 1000, 'g');
		run_and_reset(fractal, 100, 'b');
		adjust_pixels_rgb(&fractal->img, 1.2, -20);
		mean_convo_filter(&fractal->img, &fractal->img_new, 3);
		adjust_pixels_rgb(&fractal->img_new, 1.1, 15);
		adjust_pixels_rgb(&fractal->img_new, 2, 0);
		mlx_put_image_to_window(fractal->mlx_connect,
			fractal->mlx_win, fractal->img_new.img_ptr, 0, 0);
	}
}
