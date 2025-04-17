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

//------------test threads-------
void	set_thread_colors(t_fractal *fractal, t_piece piece[][fractal->num_cols])
{
	int				i;
	int				j;
	unsigned int	thread_col;

	i = -1;
	while (++i < fractal->num_rows)
	{
		j = -1;
		while (++j < fractal->num_cols)
		{
			thread_col = *(fractal->w_colors + i * fractal->num_cols + j);
			piece[i][j].thread_color = thread_col;
		}
	}
}
//------------test threads-------

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
			//set_pieces_half(fractal, piece, i, j);

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

/* void	fern(t_fractal *fractal)//to multithread fern one day
{
	t_piece	piece[fractal->num_rows][fractal->num_cols];
	int		i;
	int		j;

	i = -1;
	while (++i < fractal->num_rows)
	{
		j = -1;
		while (++j < fractal->num_cols)
		{
			set_pieces(fractal, piece, i, j);
			if (pthread_create(&fractal->threads[i * fractal->num_cols + j], NULL, \
				fern_set, (void *)&piece[i][j]) != 0)
				thread_error(fractal, fractal->num_rows * fractal->num_cols);
		}
	}
	join_threads(fractal);
//render_barnsleyfern(fractal);
} */


void	copy_buddha_half(double **density, int height, int width)
{
	int i;
	int j;

	j = -1;
	while (++j < height)
	{
		i = -1;
		while (++i < width / 2)//even number width only handled
		{
			density[j][i] += density[j][width - i - 1];
			density[j][width - i - 1] = density[j][i];
		}
	}
}

void	copy_buddha_half_map(double **density, int height, int width)
{
	int i;
	int j;

	j = -1;
	while (++j < height / 2)
	{
		i = -1;
		while (++i < width)
		{
			density[j][i] += density[height - j - 1][i];
			density[height - j - 1][i] = density[j][i];
		}
	}
}

void	buddha(t_fractal *fractal)
{
	//t_piece	piece[fractal->num_rows][fractal->num_cols];
	//t_data	data[r->num_rows * r->num_cols];
	int		j;

	j = -1;
	while (++j < fractal->num_rows * fractal->num_cols)
	{
			//set_pieces(fractal, piece, i, j);
			//set_pieces_half(fractal, piece, i, j);
		if (pthread_create(&fractal->threads[j], NULL, \
				buddha_set, (void *)fractal) != 0)
				thread_error(fractal, j);
	}
	join_threads(fractal->threads, fractal->num_rows, fractal->num_cols);
	//copy_buddha_half(fractal->density, fractal->height, fractal->width);
}

void	set_pieces_half_map(t_fractal *fractal, t_piece piece[][fractal->num_cols], int i, int j) 
{
	double map_n;

	if (fractal->id == 3)
		map_n = fractal->buddha->map_n;
	else
		map_n = 1;
	piece[j][i].x_s = i * ((fractal->width * map_n) / fractal->num_cols);
	piece[j][i].x_e = (i + 1) * ((fractal->width * map_n) / fractal->num_cols);
	piece[j][i].y_s = j * (((fractal->height / 2) * map_n) / fractal->num_rows);
	piece[j][i].y_e = (j + 1) * (((fractal->height / 2) * map_n) / fractal->num_rows);
	piece[j][i].fractal = fractal;

/* 	piece[j][i].x_s = ft_round((double)i * ((double)fractal->width * map_n) / (double)fractal->num_cols);
	piece[j][i].x_e = ft_round(((double)i + 1) * ((double)fractal->width * map_n) / (double)fractal->num_cols);
	piece[j][i].y_s = ft_round((double)j * (((double)fractal->height / 2) * map_n) / (double)fractal->num_rows);
	piece[j][i].y_e = ft_round(((double)j + 1) * (((double)fractal->height / 2) * map_n) / (double)fractal->num_rows);
	piece[j][i].fractal = fractal; */


}

void	set_pieces_map(t_fractal *fractal, t_piece piece[][fractal->num_cols], int i, int j) 
{
	double map_n;

	if (fractal->id == 3)
		map_n = fractal->buddha->map_n;
	else
		map_n = 1;
	
	piece[j][i].x_s = i * ((fractal->width * (map_n)) / fractal->num_cols);
	piece[j][i].x_e = (i + 1) * ((fractal->width * (map_n)) / fractal->num_cols);
	piece[j][i].y_s = j * (((fractal->height) * (map_n)) / fractal->num_rows);
	piece[j][i].y_e = (j + 1) * (((fractal->height) * (map_n)) / fractal->num_rows);
	piece[j][i].fractal = fractal;

	/* piece[j][i].x_s = ft_round((double)i * ((double)fractal->width * map_n) / (double)fractal->num_cols);
	piece[j][i].x_e = ft_round(((double)i + 1) * ((double)fractal->width * map_n) / (double)fractal->num_cols);
	piece[j][i].y_s = ft_round((double)j * ((double)fractal->height * map_n) / (double)fractal->num_rows);
	piece[j][i].y_e = ft_round(((double)j + 1) * ((double)fractal->height * map_n) / (double)fractal->num_rows);
	piece[j][i].fractal = fractal;
 */
}

//build importance map on a channel using grid for best results

void	buddha_map(t_fractal *fractal)
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
			if (!fractal->move_y && fractal->buddha->copy_half)
				set_pieces_half_map(fractal, piece, i, j);
			else	
				set_pieces_map(fractal, piece, i, j);
			if (pthread_create(&fractal->threads[j * fractal->num_cols + i], NULL, \
				buddha_set_map, (void *)&piece[j][i]) != 0)
				thread_error(fractal, j * fractal->num_cols + i);
		}
	}
	join_threads(fractal->threads, fractal->num_rows, fractal->num_cols);
	if (!fractal->move_y && fractal->buddha->copy_half)
		copy_buddha_half_map(fractal->densities[fractal->hist_num], fractal->height, fractal->width);
		//copy_buddha_half(fractal->density, fractal->height, fractal->width);
}

