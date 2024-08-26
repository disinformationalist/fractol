/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:10:31 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:10:32 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

void	my_pixel_put(int x, int y, t_img *img, int color)
{
	int	offset;

	offset = (y * img->line_len) + (x * (img->bpp / 8));
	*(unsigned int *)(img->pixels_ptr + offset) = color;
}

int	ft_strncmp(char *s1, char *s2, int n)
{
	if (!s1 || !s2 || n <= 0)
		return (0);
	while (*s1 && *s1 == *s2 && n > 0)
	{
		s1++;
		s2++;
		n--;
	}
	return (*s1 - *s2);
}

void	putstr_fd(char *s, int fd)
{
	while (*s)
		write(fd, s++, 1);
}

double	ft_atof(char *s)
{
	int		sign;
	long	int_piece;
	double	float_piece;

	sign = 1;
	int_piece = 0;
	float_piece = 0;
	while (*s == 32 || (*s >= 9 && *s <= 13))
		s++;
	while (*s == 45 || *s == 43)
		if (*s++ == '-')
			sign *= -1;
	while ((*s >= '0' && *s <= '9') && *s && *s != '.')
		int_piece = int_piece * 10 + *s++ - '0';
	if (*s == '.')
		s++;
	while (*s && (*s >= '0' && *s <= '9'))
		float_piece += (*s++ - '0') / 10;
	return (sign * (int_piece + float_piece));
}
