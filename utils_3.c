/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_3.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:09:49 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:09:53 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

void	filter_section(t_img *img, t_img *img_new, int kern_size, t_filter *f)
{
	f->red = 0;
	f->green = 0;
	f->blue = 0;
	f->j = -f->half_k - 1;
	while (++f->j <= f->half_k)
	{
		f->i = -f->half_k - 1;
		while (++f->i <= f->half_k)
		{
			f->offset = (f->y + f->j) * img->line_len + \
			((f->x + f->i) * (img->bpp / 8));
			f->pixel = *(unsigned int *)(img->pixels_ptr + f->offset);
			f->red += (f->pixel >> 16) & 0xFF;
			f->green += (f->pixel >> 8) & 0xFF;
			f->blue += f->pixel & 0xFF;
		}
	}
	f->avg_r = f->red / (kern_size * kern_size);
	f->avg_g = f->green / (kern_size * kern_size);
	f->avg_b = f->blue / (kern_size * kern_size);
	my_pixel_put(f->x, f->y, img_new, \
	((f->avg_r << 16) | (f->avg_g << 8) | f->avg_b));
}

void	mean_convo_filter(t_img *img, t_img *img_new, int kern_size)
{
	t_filter	f;

	f.half_k = kern_size / 2;
	f.y = f.half_k - 1;
	while (++f.y < HEIGHT - f.half_k)
	{
		f.x = f.half_k - 1;
		while (++f.x < WIDTH - f.half_k)
			filter_section(img, img_new, kern_size, &f);
	}
}

void	apply_rgb_action(t_img *img, t_position pos, double times, double plus)
{
	unsigned int	pixel;
	int				offset;
	int				red;
	int				green;
	int				blue;

	offset = pos.y * img->line_len + (pos.x * (img->bpp / 8));
	pixel = *(unsigned int *)(img->pixels_ptr + offset);
	red = (int)(((pixel >> 16) & 0xFF) * times + plus);
	if (red > 255)
		red = 255;
	else if (red < 0)
		red = 0;
	green = (int)(((pixel >> 8) & 0xFF) * times + plus);
	if (green > 255)
		green = 255;
	else if (green < 0)
		green = 0;
	blue = (int)((pixel & 0xFF) * times + plus);
	if (blue > 255)
		blue = 255;
	else if (blue < 0)
		blue = 0;
	my_pixel_put(pos.x, pos.y, img, (red << 16 | green << 8 | blue));
}

void	adjust_pixels_rgb(t_img *img, double times, double plus)
{
	t_position	pos;

	pos.y = -1;
	while (++pos.y < HEIGHT)
	{
		pos.x = -1;
		while (++pos.x < WIDTH)
			apply_rgb_action(img, pos, times, plus);
	}
}
