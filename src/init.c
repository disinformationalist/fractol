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

void	set_vals(t_fractal *fractal, int min_1, int min_2, int min_3, int max_1, int max_2, int max_3, f complex_f)
{
	fractal->buddha->min1 = min_1;
	fractal->buddha->min2 = min_2;
	fractal->buddha->min3 = min_3;
	fractal->buddha->max1 = max_1;
	fractal->buddha->max2 = max_2;
	fractal->buddha->max3 = max_3;
	fractal->complex_f = complex_f;
}

void	init_buddha(t_fractal *fractal)
{
	t_buddha	*b;
	t_btype		type;

	
	b = fractal->buddha;
	b->filter = false;
	b->fchan = 0;
	b->flevel = 11;

	fractal->move_x = .21;//-x coord
	fractal->move_y = 0;//0;//-y coord
	fractal->zoom = 1.2;//zoom lvl

	/* fractal->move_x = .34;//.158;
	fractal->move_y = -.7;//-1.033;
	fractal->zoom = 5;//200; */

	b->fast = true;
	b->copy_half = false;
	//n, map_n
	b->n = 4;//can use doubles on these as well
	b->map_n = b->n - 1;

	//b->type = 0;
	type = b->type;

	if (type == BUDDHA1)
	{
		b->bpow = 1;
		b->gpow = 1;
		b->rpow = 1;
		set_vals(fractal, 0, 0, 0, 50, 300, 2750, &square_complex);
	}
	else if (type == BUDDHA2)//stormcloud buddha
	{
		b->bpow = .5;	
		b->gpow = .65;
		b->rpow = .6;
		set_vals(fractal, 20, 55, 55, 350, 500, 3500, &square_complex);
	}
	else if (type == LOTUS)
	{
		b->bpow = .5;
		b->gpow = .65;
		b->rpow = .65;
		set_vals(fractal, 20, 55, 100, 150, 200, 3000, &square_complex_conj);
	}
	else// (type == PHEONIX)
	{
		set_vals(fractal, 12, 55, 55, 250, 600, 3000, &cube_ship);
		b->bpow = .45;
		b->gpow = .6;
		b->rpow = .5;
		fractal->move_x = 0;
	}

	//set_vals(fractal, 12, 100, 60, 2000, 350, 2500, &square_complex_conj);//maybe lotus sq_comp_conj, .5, .65, .65. .45 .6 .5
	//set_vals(fractal, 30, 55, 55, 350, 500, 3500, &square_complex);//stormcloud buddha
	//set_vals(fractal, 12, 100, 150, 2000, 2500, 3500, &square_complex);
	//set_vals(fractal, 25, 55, 100, 150, 200, 3000, &square_complex);//need to play with for a buddha version

	//set_vals(fractal,  12, 100, 150, 2000, 2500, 3500, &square_complex);//old like mill
}	

void	info_init(t_fractal *fractal)
{
	fractal->num_colors = 360;
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
	fractal->mouse_zoom = true;
	fractal->zoom_iter = 1;
	fractal->b_max_i = 0;
	fractal->supersample = 0;
	fractal->layer = 0;
	fractal->s_kernel = 3;
	fractal->histograms = 3;
	fractal->species = 0;
	fractal->aspect = (double)fractal->height_orig / (double)fractal->width_orig;
	fractal->cdf = NULL;
}

static void	events_init(t_fractal *fractal)
{
	mlx_hook(fractal->mlx_win, KeyPress, \
	KeyPressMask, key_handler, fractal);
	if (!(fractal->id == 3))
	{
		mlx_hook(fractal->mlx_win, MotionNotify, \
		PointerMotionMask, julia_handle, fractal);
	}
	mlx_hook(fractal->mlx_win, ButtonPress, \
	ButtonPressMask, mouse_handler, fractal);
	mlx_hook(fractal->mlx_win, DestroyNotify, \
	StructureNotifyMask, close_handler, fractal);
	fractal->species = 0;
}

void	fractal_init(t_fractal *fractal)
{
	fractal->mlx_connect = mlx_init();
	if (fractal->mlx_connect == NULL)
		exit(EXIT_FAILURE);
	fractal->mlx_win = mlx_new_window(fractal->mlx_connect, fractal->width, fractal->height, fractal->name);
	if (fractal->mlx_win == NULL)
		clear_all(fractal);
	if (new_img_init(fractal->mlx_connect, &fractal->img, fractal->width, fractal->height) == -1)
		clear_all(fractal);
	if (new_img_init(fractal->mlx_connect, &fractal->img_2, fractal->width, fractal->height) == -1)
		clear_all(fractal);
	info_init(fractal);
	fractal->w_colors = set_color_wheel(360, 1.0, 0.5, 202);//num colors, sat, lightness, base hue
	if (!fractal->w_colors)
		clear_all(fractal);
	if (pthread_mutex_init(&fractal->mutex, NULL) != 0)
		clear_all(fractal);
	if (pthread_mutex_init(&fractal->rand_mtx, NULL) != 0)
		clear_all(fractal);
	events_init(fractal);
	if (fractal->id == 3)
		init_buddha(fractal);
}
