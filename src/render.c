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

void	render_id(t_fractal *fractal)
{
	if (fractal->id == 1)
		mandelbrot(fractal);
	else if (fractal->id == 2)
		julia(fractal);
	else if (fractal->id == 3)
	{
		if (fractal->cdf)
			free_matrices(fractal);
		init_matricies(fractal);
		render_buddha(fractal);
	}
	else if (fractal->id == 4)
		render_barnsleyfern(fractal);
}

void	setup_vals(t_fractal *fractal)
{
	//fractal->complex_f = cube_complex;
	/* fractal->sin_a = sin(fractal->rot_x * DEG_TO_RAD);
	fractal->sin_b = sin(fractal->rot_y * DEG_TO_RAD);
	fractal->cos_a = cos(fractal->rot_x * DEG_TO_RAD);
	fractal->cos_b = cos(fractal->rot_y * DEG_TO_RAD); */
	memset(fractal->img.pixels_ptr, 0, fractal->width_orig * fractal->height_orig * (fractal->img.bpp / 8));
	fractal->size = fractal->height * fractal->width;
}

void	render(t_fractal *fractal)
{
	setup_vals(fractal);
	if (fractal->supersample)
	{
		ft_putstr_color("SUPERSAMPLE IN PROGRESS...\n", BOLD_BRIGHT_GREEN);
		fractal->pixels_xl = malloc_ui_matrix(fractal->width, fractal->height);
		if (!fractal->pixels_xl)
			clear_all(fractal);
		zero_ui_matrix(fractal->pixels_xl, fractal->width, fractal->height);
	}
	render_id(fractal);
	if (fractal->supersample)
	{
		downsample_xl(fractal->width, fractal->height, &fractal->img, fractal->pixels_xl, fractal->s_kernel);
		free_ui_matrix(fractal->pixels_xl, fractal->height);
		ft_putstr_color("SUPERSAMPLE COMPLETE\n", BOLD_BRIGHT_BLUE);
	}
	if (fractal->toggle_color == 0)
		fractal->col_i = (fractal->col_i + 7) % fractal->num_colors;
	mlx_put_image_to_window(fractal->mlx_connect,
		fractal->mlx_win, fractal->img.img_ptr, 0, 0);
}
