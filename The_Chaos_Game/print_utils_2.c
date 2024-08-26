/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_utils_2.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:13:28 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:13:31 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "chaos.h"

void	ft_putstr(char *s)
{
	write(1, "\033[33m", 6);
	while (*s)
		write(1, s++, 1);
	write(1, "\033[0m", 5);
}

void	put_bool(bool x)
{
	if (x)
		write(1, "\033[32mON\033[0m ", 12);
	else
		write(1, "\033[31mOFF\033[0m", 12);
}

void	write_num_color(int num, char *color)
{
	if (num / 10)
		write_num_color(num / 10, color);
	write(1, color, 7);
	write(1, &("0123456789"[num % 10]), 1);
	write(1, "\033[0m", 5);
}

void	put_ratio(t_game *r)
{
	char	*num_color;

	num_color = "\033[1;35m";
	if (r->dist_ratio > 1)
	{
		ft_putstr("║     DIST_RATIO ");
		write(1, "\033[1;35m> 1\033[0m", 15);
		ft_putstr("   SIDES: ");
		write_num_color(r->sides, num_color);
		if (r->sides < 10)
			write (1, " ", 1);
		ft_putstr("           ║\n");
	}
	else
	{
		ft_putstr("║     DIST_RATIO ");
		write(1, "\033[1;35m< 1\033[0m", 15);
		ft_putstr("   SIDES: ");
		write_num_color(r->sides, num_color);
		if (r->sides < 10)
			write (1, " ", 1);
		ft_putstr("           ║\n");
	}
}
