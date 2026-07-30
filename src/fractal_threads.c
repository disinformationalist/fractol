/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fractal_threads.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:10:07 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:10:09 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

void	mandelbrot(t_fractal *fractal)
{
	t_piece	piece[fractal->num_rows][fractal->num_cols];
	int		i;
	int		j;

	j = -1;
	while (++j < fractal->num_rows)
	{
		i = -1;
		while (++i < fractal->num_cols)
		{
			set_pieces(fractal, piece, i, j);
			if (pthread_create(&fractal->threads[j * fractal->num_cols + i], NULL, \
				mandel_set, (void *)&piece[j][i]) != 0)
				thread_error(fractal, j * fractal->num_cols + i);
		}
	}
	join_threads(fractal->threads, fractal->num_rows, fractal->num_cols);
}

void	julia(t_fractal *fractal)
{
	t_piece	piece[fractal->num_rows][fractal->num_cols];
	int		i;
	int		j;

	j = -1;
	while (++j < fractal->num_rows)
	{
		i = -1;
		while (++i < fractal->num_cols)
		{
			set_pieces(fractal, piece, i, j);
			if (pthread_create(&fractal->threads[j * fractal->num_cols + i], NULL, \
				julia_set, (void *)&piece[j][i]) != 0)
				thread_error(fractal, j * fractal->num_cols + i);
		}
	}
	join_threads(fractal->threads, fractal->num_rows, fractal->num_cols);
}

void	fern(t_fractal *fractal)
{
	int			j;
	t_piece		piece[fractal->num_rows * fractal->num_cols];
	uint64_t	glob_seed;
	
	glob_seed = 0xFABDECAF;
	j = -1;
	while (++j < fractal->num_rows * fractal->num_cols)
	{
		sxoro128(&piece[j].rng, glob_seed + j * 142857);
		piece[j].fractal = fractal;
		if (pthread_create(&fractal->threads[j], NULL, \
				fern_set, (void *)&piece[j]) != 0)
			thread_error(fractal, j);
	}
	join_threads(fractal->threads, fractal->num_rows, fractal->num_cols);
}

void	buddha(t_fractal *fractal)
{
	int			j;
	int			buffer;
	t_piece		piece[fractal->num_rows * fractal->num_cols];
	uint64_t	glob_seed;
	
	buffer = -1;
	while (++buffer < fractal->buffs)
	{
		glob_seed = 0xFABDECAF
			+ (uint64_t)buffer * 0x9E3779B97F4A7C15ULL;
		buddha_clear_worker_histograms(fractal);
		j = -1;
		while (++j < fractal->num_rows * fractal->num_cols)
		{
			piece[j].id = j;
			piece[j].buffer = buffer;
			sxoro128(&piece[j].rng, glob_seed + j * 142857);
			piece[j].fractal = fractal;
			if (pthread_create(&fractal->threads[j], NULL, \
					buddha_set, (void *)&piece[j]) != 0)
					thread_error(fractal, j);
		}
		join_threads(fractal->threads, fractal->num_rows, fractal->num_cols);
		buddha_reduce_worker_histograms(fractal, buffer);
	}
}
