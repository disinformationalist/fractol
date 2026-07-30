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
#include <limits.h>

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

static bool	workspace_matches(t_fractal *fractal)
{
	return (fractal->sample_counts && fractal->densities
		&& fractal->worker_histograms
		&& fractal->buddha_workspace_width == fractal->width
		&& fractal->buddha_workspace_height == fractal->height
		&& fractal->buddha_workspace_histograms == fractal->histograms
		&& fractal->buddha_workspace_workers
		== fractal->num_rows * fractal->num_cols);
}

static void	free_partial_densities(t_fractal *fractal, int count)
{
	while (--count >= 0)
		free_matrix_i(fractal->densities[count], fractal->height);
	free(fractal->densities);
	fractal->densities = NULL;
}

int	buddha_ensure_workspace(t_fractal *fractal)
{
	size_t	sample_budget;
	size_t	pixels;
	int	k;

	pixels = (size_t)fractal->width * (size_t)fractal->height;
	if (pixels > INT_MAX)
		return (fprintf(stderr,
				"Buddha workspace exceeds the supported pixel count\n"), -1);
	fractal->size = (int)pixels;
	sample_budget = buddha_sample_budget(fractal);
	if (sample_budget == SIZE_MAX || sample_budget > UINT32_MAX)
		return (fprintf(stderr,
				"Buddha per-buffer sample budget exceeds uint32 capacity\n"),
			-1);
	if (workspace_matches(fractal))
		return (0);
	if (fractal->sample_counts || fractal->densities
		|| fractal->worker_histograms)
		free_matrices(fractal);
	fractal->densities = (double ***)calloc(fractal->histograms,
			sizeof(double **));
	if (!fractal->densities)
		return (-1);
	k = -1;
	while (++k < fractal->histograms)
	{
		fractal->densities[k] = malloc_matrix(fractal->width, fractal->height);
		if (!fractal->densities[k])
		{
			free_partial_densities(fractal, k);
			return (-1);
		}
		zero_matrix(fractal->densities[k], fractal->width, fractal->height);
	}
	fractal->sample_counts = (uint32_t *)calloc(pixels,
			sizeof(*fractal->sample_counts));
	if (!fractal->sample_counts)
	{
		free_partial_densities(fractal, fractal->histograms);
		return (-1);
	}
	if (buddha_init_worker_histograms(fractal) != 0)
	{
		free(fractal->sample_counts);
		fractal->sample_counts = NULL;
		free_partial_densities(fractal, fractal->histograms);
		return (-1);
	}
	fractal->buddha_workspace_width = fractal->width;
	fractal->buddha_workspace_height = fractal->height;
	fractal->buddha_workspace_histograms = fractal->histograms;
	fractal->buddha_workspace_workers
		= fractal->num_rows * fractal->num_cols;
	return (0);
}

void	init_matricies(t_fractal *fractal)
{
	if (buddha_ensure_workspace(fractal) != 0)
		clear_all(fractal);
}
