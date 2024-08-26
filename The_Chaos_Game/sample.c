/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sample.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:13:46 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:13:48 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "chaos.h"

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
			f->offset = (f->y + f->j) * img_new->line_len + \
			((f->x + f->i) * (img_new->bpp / 8));
			f->pixel = *(unsigned int *)(img_new->pixels_ptr + f->offset);
			f->red += (f->pixel >> 16) & 0xFF;
			f->green += (f->pixel >> 8) & 0xFF;
			f->blue += f->pixel & 0xFF;
		}
	}
	f->avg_r = f->red / (kern_size * kern_size);
	f->avg_g = f->green / (kern_size * kern_size);
	f->avg_b = f->blue / (kern_size * kern_size);
	my_pixel_put(f->pix_x, f->pix_y, img, \
	((f->avg_r << 16) | (f->avg_g << 8) | f->avg_b));
}

void	downsample(t_game *r, t_img *img, t_img *img_new, int kern_size)
{
	t_filter	f;

	f.pix_y = 0;
	f.half_k = kern_size / 2;
	f.y = f.half_k;
	while (f.y < r->height - f.half_k)
	{
		f.x = f.half_k;
		f.pix_x = 0;
		while (f.x < r->width - f.half_k)
		{
			filter_section(img, img_new, kern_size, &f);
			f.x += kern_size;
			f.pix_x++;
		}
		f.y += kern_size;
		f.pix_y++;
	}
}
