/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   chaos.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 13:05:45 by jhotchki          #+#    #+#             */
/*   Updated: 2024/01/22 13:56:05 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "chaos.h"

void	polygon(int ***vertices, t_game *r)
{
	double	angle;
	int		i;

	*vertices = (int **)malloc(r->sides * sizeof(int *));
	if (!(*vertices))
	{
		mlx_destroy_image(r->mlx_connect, r->img.img_ptr);
		mlx_destroy_image(r->mlx_connect, r->img_new.img_ptr);
		mlx_destroy_window(r->mlx_connect, r->mlx_win);
		mlx_destroy_display(r->mlx_connect);
		free(r->mlx_connect);
		exit(EXIT_FAILURE);
	}
	angle = 2 * M_PI / r->sides;
	i = -1;
	while (++i < r->sides)
	{
		(*vertices)[i] = (int *)malloc(2 * sizeof(int));
		(*vertices)[i][0] = r->width / 2 + (r->move_x + r->r * \
		cos(angle * i - M_PI / 2 - r->rotate_l + r->rotate_r)) * r->zoom;
		(*vertices)[i][1] = r->height / 2 + (r->r * sin(angle * \
		i - M_PI / 2 - r->rotate_l + r->rotate_r) - r->move_y) * r->zoom;
	}
}

void	information(t_game *r, int v, int i)
{
	if (r->disinfo_1)
		r->rv[(i - 1) % 3] = v;
	if (r->disinfo_2)
		r->rv[(i - 2) % 3] = v;
	else if (!r->disinfo_1 && !r->disinfo_1)
		r->rv[i % 3] = v;
}

int	r_loop(t_game *r)
{
	int	v;
	int	tries;

	tries = 0;
	v = rand() % r->sides;
	while (ft_r(r->rv, v, r->i, r))
	{
		v = rand() % r->sides;
		tries++;
		if (tries >= 150)
			return (-1);
	}
	return (v);
}

void	chaos_game(t_game *r, int **vertices, int x, int y)
{
	int		v;
	double	distance;
	int		color;

	r->i = 1;
	polygon(&vertices, r);
	while (++r->i <= r->iters + 1)
	{
		v = r_loop(r);
		if (v == -1)
			break ;
		information(r, v, r->i);
		x = x + (vertices[v][0] - x) * r->dist_ratio;
		y = y + (vertices[v][1] - y) * r->dist_ratio;
		if ((r->i > 10) && (x > 0 && x < r->width) && (y > 0 && y < r->height))
		{
			distance = sqrt((x - r->width / 2) * (x - r->width / 2) + \
			(y - r->height / 2) * (y - r->height / 2));
			color = calc_color_4(distance, r->max_distance, &r->colors);
			put_pixel(x, y, r, color);
		}
	}
	free_poly(&vertices, r);
}

int	main(int ac, char **av)
{
	t_game	r;

	if (ac != 2)
	{
		ft_putstr_color("Syntax: ./chaos  <sample level> (odd positive"
			" int, min 3, reccommended 3, 5, or 7, max 15)\n", "\033[31m");
		exit(EXIT_SUCCESS);
	}
	r.s_kernal = ft_atoi(av[1]);
	if (r.s_kernal < 3 || r.s_kernal > 15 || r.s_kernal % 2 == 0)
	{
		ft_putstr_color("sample level must be an odd integer"
			", 3 minimum, 15 maximum)\n", "\033[31m");
		exit(EXIT_SUCCESS);
	}
	r_init(&r);
	game_init(&r);
	intermed(&r);
	mlx_loop(r.mlx_connect);
	return (0);
}
