/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 15:07:25 by jhotchki          #+#    #+#             */
/*   Updated: 2024/01/22 15:09:25 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "chaos.h"

void	second_img_init(t_game *r)
{
	r->img_new.img_ptr = mlx_new_image(r->mlx_connect,
			r->width * r->s_kernal, r->height * r->s_kernal);
	if (r->img_new.img_ptr == NULL)
	{
		mlx_destroy_image(r->mlx_connect, r->img.img_ptr);
		mlx_destroy_window(r->mlx_connect, r->mlx_win);
		mlx_destroy_display(r->mlx_connect);
		free(r->mlx_connect);
		exit(EXIT_FAILURE);
	}
	r->img_new.pixels_ptr = mlx_get_data_addr(r->img_new.img_ptr,
			&r->img_new.bpp, &r->img_new.line_len,
			&r->img_new.endian);
}

static void	*ft_memset(void *s, int c, size_t n)
{
	char	*s1;
	size_t	i;

	i = 0;
	s1 = (char *)s;
	while (i < n)
	{
		*s1 = (char)c;
		i++;
		s1++;
	}
	return ((char *)s);
}

void	free_poly(int ***vertices, t_game *r)
{
	int	i;

	i = -1;
	while (++i < r->sides)
		free((*vertices)[i]);
	free(*vertices);
}

void	my_pixel_put(int x, int y, t_img *img, int color)
{
	int	offset;

	offset = (y * img->line_len) + (x * (img->bpp / 8));
	*(unsigned int *)(img->pixels_ptr + offset) = color;
}

void	intermed(t_game *r)
{
	int		**vertices;
	double	x;
	double	y;

	x = r->width / 2;
	y = r->height / 2;
	vertices = NULL;
	if (!r->supersample)
		ft_memset(r->img.pixels_ptr, 0, r->width * r->height * \
		(r->img.bpp / 8));
	if (r->supersample)
		ft_memset(r->img.pixels_ptr, 0, (r->width / r->s_kernal) * \
		(r->height / r->s_kernal) * (r->img.bpp / 8));
	chaos_game(r, vertices, x, y);
	if (r->supersample)
	{
		downsample(r, &r->img, &r->img_new, r->s_kernal);
		ft_memset(r->img_new.pixels_ptr, 0, r->width * r->height * \
		(r->img_new.bpp / 8));
	}
	mlx_put_image_to_window(r->mlx_connect, r->mlx_win, r->img.img_ptr, 0, 0);
}
