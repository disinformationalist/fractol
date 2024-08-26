/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   colors.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 14:12:58 by jhotchki          #+#    #+#             */
/*   Updated: 2024/01/22 17:28:57 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "chaos.h"

void	put_pixel(int x, int y, t_game *r, int color)
{
	if (!r->supersample)
		my_pixel_put(x, y, &r->img, color);
	else
		my_pixel_put(x, y, &r->img_new, color);
}

static void	assign_colors(int colors_arr[4], t_colors *colors)
{
	colors_arr[0] = colors->color_1;
	colors_arr[1] = colors->color_2;
	colors_arr[2] = colors->color_3;
	colors_arr[3] = colors->color_4;
}

static int	calculate_color_component(int color_low, \
int color_high, double t, int shift)
{
	return ((int)((1 - t) * ((color_low >> shift) & 0xFF) + t * \
	((color_high >> shift) & 0xFF)) << shift);
}

static int	interpolate_colors(int color_low, int color_high, double t)
{
	int	red;
	int	green;
	int	blue;

	red = calculate_color_component(color_low, color_high, t, 16);
	green = calculate_color_component(color_low, color_high, t, 8);
	blue = calculate_color_component(color_low, color_high, t, 0);
	return (red | green | blue);
}

int	calc_color_4(double distance, double max_distance, t_colors *colors)
{
	t_color_vars	vars;
	double			norm_distance;
	double			color_index_d;
	double			t;
	int				colors_arr[4];

	vars.num_colors = 4;
	norm_distance = distance / max_distance;
	color_index_d = norm_distance * (vars.num_colors - 1);
	vars.color_index_low = (int)color_index_d;
	vars.color_index_high = vars.color_index_low + 1;
	assign_colors(colors_arr, colors);
	t = color_index_d - vars.color_index_low;
	vars.color_low = colors_arr[vars.color_index_low];
	vars.color_high = colors_arr[vars.color_index_high];
	vars.color = interpolate_colors(vars.color_low, vars.color_high, t);
	return (vars.color);
}
