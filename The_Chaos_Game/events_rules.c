/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   events_rules.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 13:57:07 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:12:01 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "chaos.h"

void	zoom_iters_key(int keysym, t_game *r)
{
	int	value;

	value = 835000 * r->s_kernal * r->s_kernal;
	if (r->supersample)
	{
		if (keysym == MULT)
			r->iters += value;
		else if (keysym == DIV && r->iters > value)
			r->iters -= value;
	}
	else
	{
		if (keysym == MULT)
			r->iters += 835000;
		else if (keysym == DIV && r->iters > 835000)
			r->iters -= 835000;
	}
	if (keysym == MULT)
		r->zoom *= 3;
	else if (keysym == DIV)
		r->zoom /= 3;
}

static int	key_handler_r_3(int keysym, t_game *r)
{
	if (keysym == 115)
		r->rule_22 = !r->rule_22;
	else if (keysym == 100)
		r->rule_23 = !r->rule_23;
	else if (keysym == 102)
		r->rule_24 = !r->rule_24;
	else if (keysym == 103)
		r->rule_25 = !r->rule_25;
	else if (keysym == 104)
		r->rule_26 = !r->rule_26;
	else if (keysym == 59)
		r->rule_27 = !r->rule_27;
	else if (keysym == 39)
		r->rule_28 = !r->rule_28;
	else if (keysym == Ü)
		r->rule_29 = !r->rule_29;
	else if (keysym == 106)
		r->disinfo_1 = !r->disinfo_1;
	else if (keysym == 107)
		r->disinfo_2 = !r->disinfo_2;
	else if (keysym == RGHT_STRG)
		r->mouse_zoom = !r->mouse_zoom;
	else
		key_handler_other(keysym, r);
	return (0);
}

static int	key_handler_r_2(int keysym, t_game *r)
{
	if (keysym == 113)
		r->rule_11 = !r->rule_11;
	else if (keysym == 119)
		r->rule_12 = !r->rule_12;
	else if (keysym == 101)
		r->rule_13 = !r->rule_13;
	else if (keysym == 114)
		r->rule_14 = !r->rule_14;
	else if (keysym == 116)
		r->rule_15 = !r->rule_15;
	else if (keysym == 121)
		r->rule_16 = !r->rule_16;
	else if (keysym == 117)
		r->rule_17 = !r->rule_17;
	else if (keysym == 105)
		r->rule_18 = !r->rule_18;
	else if (keysym == 111)
		r->rule_19 = !r->rule_19;
	else if (keysym == 112)
		r->rule_20 = !r->rule_20;
	else if (keysym == 97)
		r->rule_21 = !r->rule_21;
	else
		key_handler_r_3(keysym, r);
	return (0);
}

int	key_handler_r(int keysym, t_game *r)
{
	if (keysym == 49)
		r->rule_1 = !r->rule_1;
	else if (keysym == 50)
		r->rule_2 = !r->rule_2;
	else if (keysym == 51)
		r->rule_3 = !r->rule_3;
	else if (keysym == 52)
		r->rule_4 = !r->rule_4;
	else if (keysym == 53)
		r->rule_5 = !r->rule_5;
	else if (keysym == 54)
		r->rule_6 = !r->rule_6;
	else if (keysym == 55)
		r->rule_7 = !r->rule_7;
	else if (keysym == 56)
		r->rule_8 = !r->rule_8;
	else if (keysym == 57)
		r->rule_9 = !r->rule_9;
	else if (keysym == 48)
		r->rule_10 = !r->rule_10;
	else
		key_handler_r_2(keysym, r);
	return (0);
}
