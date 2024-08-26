/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   rules.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 14:22:48 by jhotchki          #+#    #+#             */
/*   Updated: 2024/01/22 14:57:15 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "chaos.h"

static int	ft_r_2(int rv[], int v, int i, t_game *r)
{
	if ((v == rv[(i - 1) % 3]) && (v == rv[(i - 2) % 3]) && r->rule_8)
		return (1);
	if (v == (rv[(i - 1) % 3] - 3 + r->sides) % r->sides && r->rule_9)
		return (1);
	if (v == (rv[(i - 1) % 3] + 3) % r->sides && r->rule_10)
		return (1);
	if (v == (rv[(i - 1) % 3] - 4 + r->sides) % r->sides && r->rule_11)
		return (1);
	if (v == (rv[(i - 1) % 3] + 4) % r->sides && r->rule_12)
		return (1);
	if (rv[(i - 2) % 3] == rv[(i - 1) % 3] && r->rule_13)
	{
		if (v == (rv[(i - 1) % 3] - 2 + r->sides) % r->sides)
			return (1);
		if (v == (rv[(i - 1) % 3] + 2) % r->sides)
			return (1);
	}
	if (rv[(i - 2) % 3] == rv[(i - 1) % 3] && r->rule_14)
	{
		if (v == (rv[(i - 1) % 3] - 3 + r->sides) % r->sides)
			return (1);
		if (v == (rv[(i - 1) % 3] + 3) % r->sides)
			return (1);
	}
	return (0);
}

static int	ft_r_3(int rv[], int v, int i, t_game *r)
{
	if (v == (rv[(i - 2) % 3] - 1 + r->sides) % r->sides && r->rule_15)
		return (1);
	if (v == (rv[(i - 2) % 3] + 1) % r->sides && r->rule_16)
		return (1);
	if (rv[(i - 2) % 3] == rv[(i - 1) % 3] && r->rule_17)
	{
		if (v == (rv[(i - 1) % 3] - 4 + r->sides) % r->sides)
			return (1);
		if (v == (rv[(i - 1) % 3] + 4) % r->sides)
			return (1);
	}
	if (v == (rv[(i - 2) % 3] - 2 + r->sides) % r->sides && r->rule_18)
		return (1);
	if (v == (rv[(i - 2) % 3] + 2) % r->sides && r->rule_19)
		return (1);
	if (v == (rv[(i - 2) % 3] - 3 + r->sides) % r->sides && r->rule_20)
		return (1);
	if (v == (rv[(i - 2) % 3] + 3) % r->sides && r->rule_21)
		return (1);
	if (v == (rv[(i - 2) % 3] - 4 + r->sides) % r->sides && r->rule_22)
		return (1);
	if (v == (rv[(i - 2) % 3] + 4) % r->sides && r->rule_23)
		return (1);
	return (0);
}

static int	ft_r_4(int rv[], int v, int i, t_game *r)
{
	if ((rv[(i - 1) % 3] == (rv[(i - 2) % 3] + 1) % r->sides
			|| rv[(i - 1) % 3] == (rv[(i - 2) % 3] - 1 + r->sides)
			% r->sides) && r->rule_24)
	{
		if (v == (rv[(i - 2) % 3] - 1 + r->sides) % r->sides)
			return (1);
		if (v == (rv[(i - 2) % 3] + 1) % r->sides)
			return (1);
		if (v == rv[(i - 2) % 3] && r->rule_26)
			return (1);
	}
	if ((rv[(i - 1) % 3] == (rv[(i - 2) % 3] + 1) % r->sides
			|| rv[(i - 1) % 3] == (rv[(i - 2) % 3] - 1 + r->sides)
			% r->sides) && r->rule_25)
	{
		if (v == (rv[(i - 2) % 3] - 2 + r->sides) % r->sides)
			return (1);
		if (v == (rv[(i - 2) % 3] + 2) % r->sides)
			return (1);
		if (v == rv[(i - 2) % 3] && r->rule_26)
			return (1);
	}
	return (0);
}

static int	ft_r_5(int rv[], int v, int i, t_game *r)
{
	if ((rv[(i - 1) % 3] == (rv[(i - 2) % 3] + 1) % r->sides
			|| rv[(i - 1) % 3] == (rv[(i - 2) % 3] - 1 + r->sides)
			% r->sides) && r->rule_27)
	{
		if (v == (rv[(i - 3) % 3] - 1 + r->sides) % r->sides)
			return (1);
		if (v == (rv[(i - 3) % 3] + 1) % r->sides)
			return (1);
		if (v == rv[(i - 3) % 3] && r->rule_26)
			return (1);
	}
	if ((rv[(i - 1) % 3] == (rv[(i - 2) % 3] + 1) % r->sides
			|| rv[(i - 1) % 3] == (rv[(i - 2) % 3] - 1 + r->sides)
			% r->sides) && r->rule_28)
	{
		if (v == (rv[(i - 1) % 3] - 2 + r->sides) % r->sides)
			return (1);
		if (v == (rv[(i - 1) % 3] + 2) % r->sides)
			return (1);
		if (v == rv[(i - 1) % 3] && r->rule_26)
			return (1);
	}
	return (0);
}

int	ft_r(int rv[], int v, int i, t_game *r)
{
	//------new rules-----
	if ((rv[(i - 1) % 3] == (rv[(i - 2) % 3] + 1) % r->sides//29 -> Ü
			|| rv[(i - 1) % 3] == (rv[(i - 2) % 3] - 1 + r->sides)
			% r->sides) && r->rule_29)
	{
		if (v == (rv[(i - 2) % 3] - 3 + r->sides) % r->sides)
			return (1);
		if (v == (rv[(i - 2) % 3] + 3) % r->sides)
			return (1);
		if (v == rv[(i - 2) % 3] && r->rule_26)
			return (1);
	}
	//---------
	if ((v == rv[(i - 1) % 3]) && r->rule_1)
		return (1);
	if (v == (rv[(i - 1) % 3] - 1 + r->sides) % r->sides && r->rule_2)
		return (1);
	if (v == (rv[(i - 1) % 3] + 1) % r->sides && r->rule_3)
		return (1);
	if (rv[(i - 2) % 3] == rv[(i - 1) % 3] && r->rule_4)
	{
		if (v == (rv[(i - 1) % 3] - 1 + r->sides) % r->sides)
			return (1);
		if (v == (rv[(i - 1) % 3] + 1) % r->sides)
			return (1);
	}
	if (v == rv[(i - 2) % 3] && r->rule_5)
		return (1);
	if (v == (rv[(i - 1) % 3] - 2 + r->sides) % r->sides && r->rule_6)
		return (1);
	if (v == (rv[(i - 1) % 3] + 2) % r->sides && r->rule_7)
		return (1);
	if (ft_r_2(rv, v, i, r) || ft_r_3(rv, v, i, r))
		return (1);
	if (ft_r_4(rv, v, i, r) || ft_r_5(rv, v, i, r))
		return (1);
	return (0);
}
