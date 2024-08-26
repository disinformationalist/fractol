/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 14:58:47 by jhotchki          #+#    #+#             */
/*   Updated: 2024/01/22 17:47:33 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "chaos.h"

	/*-------RULES---------*/
	/*---1 ===== NO DOUBLES
	-----2 ===== NO LEFT NEIGHBOR OF PREVIOUS
	-----3 ===== NO RIGHT NEIGHBOR OF PREVIOUS
    -----4 ===== IF SAME TWICE(DOUBLE), NO NEIGHBOR 1 AWAY
    -----5 ===== NOT SAME AS 1 BEFORE PREVIOUS
    -----6 ===== NO 2 AWAY LEFT
    -----7 ===== NO 2 AWAY RIGHT
    -----8 ===== NO TRIPLES
    -----9 ===== NO 3 AWAY LEFT
    -----10 ==== NO 3 AWAY RIGHT
    -----11 ==== NO 4 AWAY LEFT
    -----12 ==== NO 4 AWAY RIGHT
    -----13 ==== IF DOUBLE NO 2 AWAY
    -----14 ==== IF DOUBLE NO 3 AWAY
    -----15 ==== NO LEFT NEIGHBOR OF ONE BEFORE PREV
    -----16 ==== NO RIGHT NEIGHBOR OF ONE BEFORE PREV
    -----17 ==== IF DOUBLE NO 4 AWAY
    -----18 ==== NO 2 AWAY OF 1 BEFORE PREV, LEFT
    -----19 ==== NO 2 AWAY OF 1 BEFORE PREV, RIGHT
    -----20 ==== NO 3 AWAY OF 1 BEFORE PREV, LEFT
    -----21 ==== NO 3 AWAY OF 1 BEFORE PREV, RIGHT
    -----22 ==== NO 4 AWAY OF 1 BEFORE PREV, LEFT
    -----23 ==== NO 4 AWAY OF 1 BEFORE PREV, RIGHT
    -----24 ==== IF PREV IS NEIGH OF 1 BEFORE PREV, NO NEIGH OF 1 BEFORE PREV
    -----25 ==== IF PREV IS NEIGH OF 1 BERORE PREV, NO 2 AWAY OF 1 BEFORE PREV
    -----26 ==== TURNS ON NO SAME AS 1 BEFORE PREV FOR 24 AND 25, 27, 28
	-----27 ==== IF PREV IS A NEIGH OF 1 BEFORE PREV, NO NEIGH OF 2 BEFORE PREV
	-----28 ==== IF PREV IS A NEIGH OF 1 BEFORE PREV, NO 2 AWAY FROM PREV
    -----DISINFO_1 ==== DISINFORM RECENT VERTS 1 BACK
    -----DISINFO_2 ==== DISINFORM RECENT VERTS 2 BACK
    --RULE INFINITY === MAKE DOPE ASS PROGRAMS */

void	events_init(t_game *r)
{
	mlx_hook(r->mlx_win, KeyPress, KeyPressMask, key_handler, r);
	mlx_hook(r->mlx_win, DestroyNotify, StructureNotifyMask, close_screen, r);
	mlx_hook(r->mlx_win, ButtonPress, ButtonPressMask, mouse_handler, r);
	r->colors.color_1 = 0x00FFFF;
	r->colors.color_2 = 0xFF00FF;
	r->colors.color_3 = 0xFFFF00;
	r->colors.color_4 = 0x0000FF;
	r->mouse_zoom = 0;
}

void	info_init(t_game *r)
{
	r->move_x = 0.0;
	r->move_y = 0.0;
	r->zoom = 1.0;
	r->supersample = 0;
	if (!r->supersample)
		r->iters = 540000;
	r->sides = 1;
	r->r = r->height / 2 - r->height / 10;
	r->rotate_l = 0;
	r->rotate_r = 0;
	r->dist_ratio = .5;
	r->max_distance = r->width / 2;
	r->rv[0] = 0;
	r->rv[1] = 1;
	r->rv[2] = 2;
}

void	r_init_2(t_game *r)
{
	r->rule_15 = 0;
	r->rule_16 = 0;
	r->rule_17 = 0;
	r->rule_18 = 0;
	r->rule_19 = 0;
	r->rule_20 = 0;
	r->rule_21 = 0;
	r->rule_22 = 0;
	r->rule_23 = 0;
	r->rule_24 = 0;
	r->rule_25 = 0;
	r->rule_26 = 0;
	r->rule_27 = 0;
	r->rule_28 = 0;
	r->rule_29 = 0;
	r->disinfo_1 = 0;
	r->disinfo_2 = 0;
}

void	r_init(t_game *r)
{
	r->width = 1920;
	r->height = 960;
	r->rule_1 = 0;
	r->rule_2 = 0;
	r->rule_3 = 0;
	r->rule_4 = 0;
	r->rule_5 = 0;
	r->rule_6 = 0;
	r->rule_7 = 0;
	r->rule_8 = 0;
	r->rule_9 = 0;
	r->rule_10 = 0;
	r->rule_11 = 0;
	r->rule_12 = 0;
	r->rule_13 = 0;
	r->rule_14 = 0;
	r_init_2(r);
}

void	game_init(t_game *r)
{
	r->mlx_connect = mlx_init();
	if (r->mlx_connect == NULL)
		exit(EXIT_FAILURE);
	r->mlx_win = mlx_new_window(r->mlx_connect, r->width, \
		r->height, "******* THE  CHAOS  GAME *******");
	if (r->mlx_win == NULL)
	{
		mlx_destroy_display(r->mlx_connect);
		free(r->mlx_connect);
		exit(EXIT_FAILURE);
	}
	r->img.img_ptr = mlx_new_image(r->mlx_connect, r->width, r->height);
	if (r->img.img_ptr == NULL)
	{
		mlx_destroy_window(r->mlx_connect, r->mlx_win);
		mlx_destroy_display(r->mlx_connect);
		free(r->mlx_connect);
		exit(EXIT_FAILURE);
	}
	r->img.pixels_ptr = mlx_get_data_addr(r->img.img_ptr,
			&r->img.bpp, &r->img.line_len, &r->img.endian);
	second_img_init(r);
	events_init(r);
	info_init(r);
}
