/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   math_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:09:32 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:09:35 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	iter_color(double curr_iter, double max_iter, int base_color)
{
	int		r;
	int		g;
	int		b;
	double	normalized_iter;

	normalized_iter = curr_iter / max_iter;
	r = (int)(normalized_iter * ((base_color >> 16) & 0xFF));
	g = (int)(normalized_iter * ((base_color >> 8) & 0xFF));
	b = (int)(normalized_iter * (base_color & 0xFF));
	return ((r << 16) | (g << 8) | b);
}

double	map(double unscaled_num, double new_min, \
double new_max, double old_max)
{
	return ((new_max - new_min) * unscaled_num / old_max + new_min);
}

double	map_back(double unscaled_num, double new_max, \
double old_min, double old_max)
{
	return ((new_max) * (unscaled_num - old_min) / (old_max - old_min));
}

t_complex	sum_complex(t_complex z1, t_complex z2)
{
	t_complex	res;

	res.x = z1.x + z2.x;
	res.y = z1.y + z2.y;
	return (res);
}

t_complex	square_complex(t_complex z)
{
	t_complex	res;

	res.x = (z.x * z.x) - (z.y * z.y);
	res.y = 2 * z.x * z.y;
	return (res);
}
