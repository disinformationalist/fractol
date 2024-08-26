/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   density_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 18:08:17 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 18:08:20 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

void	init_density(t_fractal *fractal)
{
	int	i;

	fractal->density = (int **)malloc(WIDTH * sizeof(int *));
	if (fractal->density == NULL)
		exit(EXIT_FAILURE);
	i = -1;
	while (++i < WIDTH)
	{
		fractal->density[i] = (int *)malloc(HEIGHT * sizeof(int));
		if (fractal->density[i] == NULL)
		{
			free_density(fractal, i);
			exit(EXIT_FAILURE);
		}
	}
	zero_density(fractal);
}

void	zero_density(t_fractal *fractal)
{
	int	i;
	int	j;

	i = -1;
	while (++i < WIDTH)
	{
		j = -1;
		while (++j < HEIGHT)
			*(*(fractal->density + i) + j) = 0;
	}
}

void	free_density(t_fractal *fractal, int j)
{
	while (--j >= 0)
	{
		free (fractal->density[j]);
		fractal->density[j] = NULL;
	}
	free(fractal->density);
	fractal->density = NULL;
}
