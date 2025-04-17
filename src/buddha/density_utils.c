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

/* int	**calloc_int_matrix(int width, int height)
{
	int **matrix;
	int i;

	i = -1;
	matrix = (int **)malloc(height * sizeof(int *));
	if (!matrix)
		return (NULL);
	while (++i < height)
	{
		matrix[i] = (int *)malloc(width * sizeof(int));
		if (!matrix[i])
		{
			while(--i >= 0)
				free(matrix[i]);
			free(matrix);
			return (NULL);
		}
	}
	return (matrix);
} */

void	zero_int_matrix(int **matrix, int width, int height)
{
	int j;
	int i;
	
	j = -1;
	while (++j < height)
	{
		i = -1;
		while (++i < width)
			matrix[j][i] = 0;
	}
}

void	zero_matrix(double **matrix, int width, int height)
{
	int	i;
	int	j;

	j = -1;
	while (++j < height)
	{
		i = -1;
		while (++i < width)
			matrix[j][i] = 0.0;
	}
}

void	free_3d_array_i(double ***array3d, int k, int height)
{
	while(--k >= 0)
		free_matrix_i(array3d[k], height);
	free(array3d);
	array3d = NULL;
}

void	init_matricies(t_fractal *fractal)
{
	int	k;
	int histograms = fractal->histograms;

	fractal->cdf = (double *)malloc(fractal->size * sizeof(double));
	if (!fractal->cdf)
		clear_all(fractal);
	//memset(fractal->cdf, 0, fractal->size * sizeof(double));
	fractal->densities = (double ***)malloc(histograms * sizeof(double **));
	if (!fractal->densities)
	{
		free (fractal->cdf);
		clear_all(fractal);
	}
	k = -1;
	while (++k < histograms)
	{
		fractal->densities[k] = malloc_matrix(fractal->width, fractal->height);
		if (!fractal->densities[k])
		{
			free_3d_array_i(fractal->densities, k, fractal->height);
			clear_all(fractal);
		}
		zero_matrix(fractal->densities[k], fractal->width, fractal->height);
	}
	/* fractal->density = malloc_matrix(fractal->width, fractal->height);
	if (!fractal->density)
		clear_all(fractal); */
	fractal->pdf = malloc_matrix(fractal->width, fractal->height);//pro
}

void	zero_densities(t_fractal *fractal)
{
	int k;

	k = -1;
	while (++k < fractal->histograms)
		zero_matrix(fractal->densities[k], fractal->width, fractal->height);
}




/* void	init_density(t_fractal *fractal)
{
	int	i;

	fractal->density = (int **)malloc(fractal->height * sizeof(int *));
	if (fractal->density == NULL)
		clear_all(fractal);
	i = -1;
	while (++i < fractal->height)
	{
		fractal->density[i] = (int *)malloc(fractal->width * sizeof(int));
		if (fractal->density[i] == NULL)
		{
			free_density(fractal, i);
			clear_all(fractal);
		}
	}
	zero_density(fractal);
}

void	zero_density(t_fractal *fractal)
{
	int	i;
	int	j;

	j = -1;
	while (++j < fractal->height)
	{
		i = -1;
		while (++i < fractal->width)
			fractal->density[j][i] = 0;
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
} */
