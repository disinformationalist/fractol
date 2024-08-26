/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:08:42 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:08:44 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

void	second_img_init(t_fractal *fractal)
{
	fractal->img_new.img_ptr = mlx_new_image(fractal->mlx_connect, \
	WIDTH, HEIGHT);
	if (fractal->img_new.img_ptr == NULL)
	{
		mlx_destroy_image(fractal->mlx_connect, fractal->img.img_ptr);
		mlx_destroy_window(fractal->mlx_connect, fractal->mlx_win);
		mlx_destroy_display(fractal->mlx_connect);
		free(fractal->mlx_connect);
		exit(EXIT_FAILURE);
	}
	fractal->img_new.pixels_ptr = mlx_get_data_addr(fractal->img_new.img_ptr,
			&fractal->img_new.bpp, &fractal->img_new.line_len, \
			&fractal->img_new.endian);
}

void	info_init(t_fractal *fractal)
{
	fractal->bound = 4;
	fractal->max_i = 80;
	fractal->move_x = 0.0;
	fractal->move_y = 0.0;
	fractal->zoom = 1.0;
	fractal->col_i = 0;
	fractal->toggle_color = 0;
	fractal->color_spectrum = 0;
	fractal->move_col_x = 0;
	fractal->move_col_y = 0;
	fractal->j_handle = 0;
	fractal->mouse_zoom = 0;
	fractal->zoom_iter = 1;
	fractal->co->saturation = .8;
	fractal->co->lightness = 0.5;
	fractal->co->base_hue = 165;
	fractal->b_max_i = 0;
	fractal->n = 1.0;
}

void	clear_all(t_fractal *fractal)
{
	if (fractal->density != NULL)
		free_density(fractal, WIDTH);
	if (fractal->co->colors)
		free(fractal->co->colors);
	if (fractal->co)
		free(fractal->co);
	if (fractal->img.img_ptr)
		mlx_destroy_image(fractal->mlx_connect, fractal->img.img_ptr);
	if (fractal->img_new.img_ptr)
		mlx_destroy_image(fractal->mlx_connect, fractal->img_new.img_ptr);
	if (fractal->mlx_win)
		mlx_destroy_window(fractal->mlx_connect, fractal->mlx_win);
	mlx_destroy_display(fractal->mlx_connect);
	free(fractal->mlx_connect);
	perror("Malloc or thread error");
	exit(EXIT_FAILURE);
}

static void	events_init(t_fractal *fractal)
{
	mlx_hook(fractal->mlx_win, KeyPress, \
	KeyPressMask, key_handler, fractal);
	if (ft_strncmp(fractal->name, "buddha", 6))
	{
		mlx_hook(fractal->mlx_win, ButtonPress, \
		ButtonPressMask, mouse_handler, fractal);
		mlx_hook(fractal->mlx_win, MotionNotify, \
		PointerMotionMask, julia_handle, fractal);
	}
	mlx_hook(fractal->mlx_win, DestroyNotify, \
	StructureNotifyMask, close_handler, fractal);
}

void	fractal_init(t_fractal *fractal)
{
	init_density(fractal);
	fractal->mlx_connect = mlx_init();
	if (fractal->mlx_connect == NULL)
	{
		free_density(fractal, WIDTH);
		exit(EXIT_FAILURE);
	}
	fractal->mlx_win = mlx_new_window(fractal->mlx_connect, \
	WIDTH, HEIGHT, fractal->name);
	if (fractal->mlx_win == NULL)
		clear_all(fractal);
	fractal->img.img_ptr = mlx_new_image(fractal->mlx_connect, WIDTH, HEIGHT);
	if (fractal->img.img_ptr == NULL)
		clear_all(fractal);
	fractal->img.pixels_ptr = mlx_get_data_addr(fractal->img.img_ptr,
			&fractal->img.bpp, &fractal->img.line_len, &fractal->img.endian);
	second_img_init(fractal);
	fractal->co = (t_colors *)malloc(sizeof(t_colors));
	if (!fractal->co)
		clear_all(fractal);
	info_init(fractal);
	get_color_wheel(360, fractal->co, fractal);
	if (pthread_mutex_init(&fractal->mutex, NULL) != 0)
		clear_all(fractal);
	events_init(fractal);
}
