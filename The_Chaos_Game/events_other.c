/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   events_other.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:14:30 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:14:32 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "chaos.h"

static int	key_handler_other_5(int keysym, t_game *r)
{
	if (keysym == 110)
	{
		r->colors.color_1 = 0x00FFFF;
		r->colors.color_2 = 0xFF00FF;
		r->colors.color_3 = 0xFFFF00;
		r->colors.color_4 = 0x0000FF;
	}
	else if (keysym == 120)
	{
		r->colors.color_1 = DEEP_PURPLE;
		r->colors.color_2 = ICY_BLUE;
		r->colors.color_3 = LIGHT_CYAN;
		r->colors.color_4 = SILVER;
	}
	else if (keysym == 122)
	{
		r->colors.color_1 = DEEP_RED;
		r->colors.color_2 = ORANGE;
		r->colors.color_3 = YELLOW;
		r->colors.color_4 = INTENSE_PINK;
	}
	else
		supersample_handler(keysym, r);
	return (0);
}

static int	key_handler_other_4(int keysym, t_game *r)
{
	if (keysym == 99)
	{
		r->colors.color_1 = BUBBLEGUM_PINK;
		r->colors.color_2 = LEMON_YELLOW;
		r->colors.color_3 = MINT_GREEN;
		r->colors.color_4 = SKY_BLUE;
	}
	else if (keysym == 118)
	{
		r->colors.color_1 = TOXIC_GREEN;
		r->colors.color_2 = RADIOACTIVE_YELLOW;
		r->colors.color_3 = ALIEN_BLUE;
		r->colors.color_4 = SLIMY_PURPLE;
	}
	else if (keysym == 98)
	{
		r->colors.color_1 = MYSTIC_PURPLE;
		r->colors.color_2 = PURPLE_BLUE;
		r->colors.color_3 = ERIE_BLUE;
		r->colors.color_4 = SINISTER_RED;
	}
	else
		key_handler_other_5(keysym, r);
	return (0);
}

static int	key_handler_other_3(int keysym, t_game *r)
{
	if (keysym == ALT)
	{
		r->rotate_l = 0;
		r->rotate_r = 0;
	}
	else if (keysym == 44)
	{
		if (r->supersample)
			r->max_distance += 100 * r->s_kernal;
		else
			r->max_distance += 100;
	}
	else if (keysym == 46)
	{
		if (r->supersample)
			r->max_distance -= 100 * r->s_kernal;
		else
			r->max_distance -= 100;
	}
	else if (keysym == 65435)
		r->dist_ratio = 2;
	else
		key_handler_other_4(keysym, r);
	return (0);
}

static int	key_handler_other_2(int keysym, t_game *r)
{
	if (keysym == 65473)
		r->sides = 4;
	else if (keysym == 65474)
		r->sides = 5;
	else if (keysym == 65475)
		r->sides = 6;
	else if (keysym == 65476)
		r->sides = 7;
	else if (keysym == 65481)
		r->sides = 12;
	else if (keysym == STRG)
	{
		r_init(r);
		info_init(r);
	}
	else
		key_handler_other_3(keysym, r);
	return (0);
}

int	key_handler_other(int keysym, t_game *r)
{
	if (keysym == MULT || keysym == DIV)
		zoom_iters_key(keysym, r);
	else if (keysym == PAD_ZERO)
	{
		if (r->supersample)
			r->iters = 540000 * r->s_kernal * r->s_kernal;
		else
			r->iters = 540000;
		r->zoom = 1.0;
		r->move_x = 0.0;
		r->move_y = 0.0;
		r->rotate_l = 0;
		r->rotate_r = 0;
	}
	else
		key_handler_other_2(keysym, r);
	return (0);
}
