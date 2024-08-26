/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   events.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 14:03:29 by jhotchki          #+#    #+#             */
/*   Updated: 2024/01/22 14:07:36 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "chaos.h"

int	close_screen(t_game *r)
{
	mlx_destroy_image(r->mlx_connect, r->img.img_ptr);
	mlx_destroy_image(r->mlx_connect, r->img_new.img_ptr);
	mlx_destroy_window(r->mlx_connect, r->mlx_win);
	mlx_destroy_display(r->mlx_connect);
	free(r->mlx_connect);
	exit(EXIT_SUCCESS);
}

static int	key_handler_3(int keysym, t_game *r)
{
	if (keysym == XK_Down)
	{
		if (r->supersample)
			r->move_y += (72 * r->s_kernal) / r->zoom;
		else
			r->move_y += 72 / r->zoom;
	}
	else if (keysym == 93)
	{
		if (r->supersample)
			r->iters += 540000 * r->s_kernal * r->s_kernal;
		else
			r->iters += 540000;
	}
	else if (keysym == 47)
	{
		if (r->supersample)
			r->iters -= 540000 * r->s_kernal * r->s_kernal;
		else
			r->iters -= 540000;
	}
	else
		key_handler_r(keysym, r);
	return (0);
}

static int	key_handler_2(int keysym, t_game *r)
{
	if (keysym == XK_Left)
	{
		if (r->supersample)
			r->move_x += (72 * r->s_kernal) / r->zoom;
		else
			r->move_x += 72 / r->zoom;
	}
	else if (keysym == XK_Right)
	{
		if (r->supersample)
			r->move_x -= (72 * r->s_kernal) / r->zoom;
		else
			r->move_x -= 72 / r->zoom;
	}
	else if (keysym == XK_Up)
	{
		if (r->supersample)
			r->move_y -= (72 * r->s_kernal) / r->zoom;
		else
			r->move_y -= 72 / r->zoom;
	}
	else
		key_handler_3(keysym, r);
	return (0);
}

int	key_handler(int keysym, t_game *r)
{
	//printf("%d\n", keysym);
	if (keysym == XK_Escape)
		close_screen(r);
	else if (keysym == 65431)
		r->dist_ratio += .05;
	else if (keysym == 65433)
		r->dist_ratio -= .05;
	else if (keysym == 65436)
		r->dist_ratio = 1.5;
	else if (keysym == 65437)
		r->dist_ratio = .5;
	else if (keysym == PAD_PLUS)
		r->sides += 1;
	else if (keysym == PAD_MINUS)
	{
		if (r->sides > 1)
			r->sides -= 1;
	}
	else
		key_handler_2(keysym, r);
	intermed(r);
	return (0);
}
