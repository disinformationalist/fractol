/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   colors.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:08:28 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:08:30 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

void	wheel_helper(t_colors *co)
{
	if (co->hue >= 0 && co->hue < 60)
	{
		co->r_ = co->c;
		co->g_ = co->x;
		co->b_ = 0;
	}
	if (co->hue >= 60 && co->hue < 120)
	{
		co->r_ = co->x;
		co->g_ = co->c;
		co->b_ = 0;
	}
	if (co->hue >= 120 && co->hue < 180)
	{
		co->r_ = 0;
		co->g_ = co->c;
		co->b_ = co->x;
	}
}

void	wheel_helper_2(t_colors *co)
{
	if (co->hue >= 180 && co->hue < 240)
	{
		co->r_ = 0;
		co->g_ = co->x;
		co->b_ = co->c;
	}
	if (co->hue >= 240 && co->hue < 300)
	{
		co->r_ = co->x;
		co->g_ = 0;
		co->b_ = co->c;
	}
	if (co->hue >= 300 && co->hue <= 360)
	{
		co->r_ = co->c;
		co->g_ = 0;
		co->b_ = co->x;
	}
}

void	get_color_wheel(int num_colors, t_colors *co, t_fractal *fractal)
{
	int	i;

	co->colors = (int *)malloc(num_colors * sizeof(int));
	if (!co->colors)
		clear_all(fractal);
	i = -1;
	co->hue_step = 360.0 / num_colors;
	while (++i < num_colors)
	{
		co->hue = fmod(co->base_hue + i * co->hue_step, 360.0);
		co->c = (1 - fabs(2 * co->lightness - 1)) * co->saturation;
		co->x = co->c * (1 - fabs(fmod(co->hue / 60.0, 2) - 1));
		co->m = co->lightness - co->c / 2.0;
		wheel_helper(co);
		wheel_helper_2(co);
		co->colors[i] = ((int)(255 * (co->r_ + co->m)) << 16)
			| ((int)(255 * (co->g_ + co->m)) << 8)
			| (int)(255 * (co->b_ + co->m));
	}
}
