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

void	set_edge_vals(t_buddha *b, double e0b, double e0g, double e0r, double e1b, double e1g, double e1r)
{
	b->edge0_b = e0b;
	b->edge0_g = e0g;
	b->edge0_r = e0r;
	b->edge1_b = e1b;
	b->edge1_g = e1g;
	b->edge1_r = e1r;
}

void	set_powers(t_buddha *bud, double b, double g, double r)
{
	bud->bpow = b;
	bud->gpow = g;
	bud->rpow = r;
}

void	init_buddha(t_fractal *fractal)
{
	t_buddha	*b;
	t_btype		type;

	b = fractal->buddha;
	b->filter = false;
	b->fchan = 0;
	b->flevel = 5;
	b->ftype = MEAN;
	b->mlx_win_map = NULL;
	b->change = .05;

	/* fractal->move_x = -0.0425;//.21//-x coord 
	fractal->move_y = 0.9862;//-y coord
	fractal->zoom = 420;//420;//10;//1.2;//420;//zoom lvl,  */

	fractal->move_x = -.21;//.34;//-.158;
	fractal->move_y = 0;//-.7;//-1.033;
	fractal->zoom = 1.2;//5;//200;

	/* fractal->move_x = -0.042865;
	fractal->move_y = 0.989751;
	fractal->zoom = 55150;//55150 */

	b->fast = true;
	b->copy_half = 0;//true;
	b->n = 6;//can use doubles on these as well
	b->map_n = b->n - 1;
	type = b->type;
	b->smootherstep = false;
//.86,.93., .83   -.05, .7,.5,.55
	if (type == BUDDHA1)
	{
		fractal->buffs = 3;
		b->smootherstep = true;
		if (b->smootherstep)
			set_powers(b, .41, .39, .34);// bitter?
			//set_powers(b, 1, 1, 1);
			//set_powers(b, .43, .38, .35);
		else
			set_powers(b, 1, 1, 1);
		set_edge_vals(b, .31, .29, .29, .59, .61, .61);
		//set_edge_vals(b, .35, .35, .35, .65, .65, .65);

		set_vals(fractal, 0, 0, 0, 50, 500, 5000, &square_complex);
			//set_vals(fractal, 0, 0, 0, 50, 300, 2750, &square_complex);
	}
	else if (type == BUDDHA2)
	{
		fractal->buffs = 3;
		b->smootherstep = true;
		if (b->smootherstep)
		{
			set_powers(b, .48, .58, .53);
			set_vals(fractal, 0, 55, 55, 350, 500, 3500, &square_complex);
		}
		else
		{
			set_powers(b, .5, .65, .6);
			set_vals(fractal, 20, 55, 55, 350, 500, 3500, &square_complex);
		}
		set_edge_vals(b, .15, .0, .0, .55, .50, .6);		
	}
	else if (type == LOTUS)
	{
		fractal->buffs = 1;
		fractal->name = "Lotus";
		set_powers(b, .5, .65, .65);
		set_edge_vals(b, .05, .05, .05, .6, .6, .6);
		set_vals(fractal, 20, 55, 100, 150, 200, 3000, &square_complex_conj);
	}
	else// (type == PHEONIX)
	{
		fractal->buffs = 1;
		fractal->name = "Pheonix";
		set_powers(b, .45, .6, .5);
		set_edge_vals(b, .05, .05, .05, .6, .6, .6);
		set_vals(fractal, 12, 55, 55, 250, 600, 3000, &cube_ship);
		fractal->move_x = 0;
		b->copy_half = false;
	}
	fractal->histograms = fractal->buffs * 3;
}	

void	info_init(t_fractal *fractal)
{
	fractal->bound = 4;
	fractal->num_colors = 360;
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
	fractal->species = 0;
	fractal->aspect = (double)fractal->height_orig / (double)fractal->width_orig;
	fractal->cdf = NULL;
	fractal->fdensity = NULL;
	fractal->pixels_xl = NULL;
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
	//fractal->mlx_win = mlx_new_window(fractal->mlx_connect, 20, 20, "bud");
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
