/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   events_mouse_super.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 14:08:10 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:12:53 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "chaos.h"

void	zoom_iters_mouse(int button, t_game *r)
{
	if (r->supersample)
	{
		if (button == 4)
			r->iters += 79000 * r->s_kernal * r->s_kernal;
		else if (button == 5)
			r->iters -= 79000 * r->s_kernal * r->s_kernal;
	}
	else
	{
		if (button == 4)
			r->iters += 79000;
		else if (button == 5)
			r->iters -= 79000;
	}
	if (button == 4)
		r->zoom *= 1.1;
	else if (button == 5)
		r->zoom /= 1.1;
}

static int	mouse_handler_2(int button, int x, int y, t_game *r)
{
	if (button == 4 || button == 5)
	{
		if (r->mouse_zoom)
		{
			r->move_x = (r->width / 2 - x) / r->zoom + r->move_x;
			r->move_y = (y - r->height / 2) / r->zoom + r->move_y;
		}
		zoom_iters_mouse(button, r);
	}
	return (0);
}

int	mouse_handler(int button, int x, int y, t_game *r)
{
	if (r->supersample)
	{
		x = x * r->s_kernal;
		y = y * r->s_kernal;
	}
	if (button == 1)
		r->rotate_l += M_PI / ((r->sides * 3) * r->zoom);
	else if (button == 3)
		r ->rotate_r += M_PI / ((r->sides * 3) * r->zoom);
	else
		mouse_handler_2(button, x, y, r);
	intermed(r);
	return (0);
}

static void	set_supersampler_off(t_game *r)
{
	r->iters = r->iters / (r->s_kernal * r->s_kernal);
	r->width = r->width / r->s_kernal;
	r->height = r->height / r->s_kernal;
	r->r = r->height / 2 - r->height / 10;
	r->max_distance = r->max_distance / r->s_kernal;
	r->move_x = r->move_x / r->s_kernal;
	r->move_y = r->move_y / r->s_kernal;
}

int	supersample_handler(int keysym, t_game *r)
{
	if (keysym == 32)
	{
		if (r->supersample)
			set_supersampler_off(r);
		else
		{
			r->width = r->width * r->s_kernal;
			r->height = r->height * r->s_kernal;
			r->iters = r->iters * r->s_kernal * r->s_kernal;
			r->r = r->height / 2 - r->height / 10;
			r->max_distance = r->max_distance * r->s_kernal;
			r->move_x = r->move_x * r->s_kernal;
			r->move_y = r->move_y * r->s_kernal;
		}
		r->supersample = !r->supersample;
	}
	else if (keysym == 65434)
		r->dist_ratio = 1 / PHI;
	else if (keysym == 65429)
		r->dist_ratio = .6667;
	else if (keysym == 65470)
		print_board(r);
	return (0);
}
