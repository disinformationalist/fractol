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

void	set_pieces(t_fractal *fractal, t_piece piece[][NUM_COLS], int i, int j)
{
	piece[i][j].x_s = j * ((WIDTH * fractal->n) / NUM_COLS);
	piece[i][j].x_e = (j + 1) * ((WIDTH * fractal->n) / NUM_COLS);
	piece[i][j].y_s = i * ((HEIGHT * fractal->n) / NUM_ROWS);
	piece[i][j].y_e = (i + 1) * ((HEIGHT * fractal->n) / NUM_ROWS);
	piece[i][j].fractal = fractal;
}

void	join_threads(t_fractal *fractal)
{
	int	i;
	int	j;

	i = -1;
	while (++i < NUM_ROWS)
	{
		j = -1;
		while (++j < NUM_COLS)
			pthread_join(fractal->threads[i * NUM_COLS + j], NULL);
	}
}

void	mandelbrot(t_fractal *fractal)
{
	t_piece	piece[NUM_ROWS][NUM_COLS];
	int		i;
	int		j;

	i = -1;
	while (++i < NUM_ROWS)
	{
		j = -1;
		while (++j < NUM_COLS)
		{
			set_pieces(fractal, piece, i, j);
			if (pthread_create(&fractal->threads[i * NUM_COLS + j], NULL, \
				mandel_set, (void *)&piece[i][j]) != 0)
				thread_error(fractal, NUM_ROWS * NUM_COLS);
		}
	}
	join_threads(fractal);
	if (fractal->toggle_color == 0)
		fractal->col_i = (fractal->col_i + 7) % 360;
	mlx_put_image_to_window(fractal->mlx_connect,
		fractal->mlx_win, fractal->img.img_ptr, 0, 0);
}

void	julia(t_fractal *fractal)
{
	t_piece	piece[NUM_ROWS][NUM_COLS];
	int		i;
	int		j;

	i = -1;
	while (++i < NUM_ROWS)
	{
		j = -1;
		while (++j < NUM_COLS)
		{
			set_pieces(fractal, piece, i, j);
			if (pthread_create(&fractal->threads[i * NUM_COLS + j], NULL, \
				julia_set, (void *)&piece[i][j]) != 0)
				thread_error(fractal, NUM_ROWS * NUM_COLS);
		}
	}
	join_threads(fractal);
	if (fractal->toggle_color == 0)
		fractal->col_i = (fractal->col_i + 7) % 360;
	mlx_put_image_to_window(fractal->mlx_connect,
		fractal->mlx_win, fractal->img.img_ptr, 0, 0);
}

void	buddha(t_fractal *fractal)
{
	t_piece	piece[NUM_ROWS][NUM_COLS];
	int		i;
	int		j;

	i = -1;
	while (++i < NUM_ROWS)
	{
		j = -1;
		while (++j < NUM_COLS)
		{
			set_pieces(fractal, piece, i, j);
			if (pthread_create(&fractal->threads[i * NUM_COLS + j], NULL, \
				buddha_set, (void *)&piece[i][j]) != 0)
				thread_error(fractal, NUM_ROWS * NUM_COLS);
		}
	}
	join_threads(fractal);
}
