#include "image_processing.h"

double	get_matrix_sum(double **matrix, int width, int height)
{
	int		i;
	int		j;
	double	sum;

	sum = 0;
	j = -1;
	while (++j < height)
	{
		i = -1;
		while (++i < width)
				sum += matrix[j][i];
	}
	return (sum);
}


double	**malloc_matrix(int width, int height)
{
	double	**matrix;
	int i;

	i = -1;
	matrix = (double **)malloc(height * sizeof(double *));
	if (!matrix)
		return (NULL);
	while (++i < height)
	{
		matrix[i] = (double *)malloc(width * sizeof(double));
		if (!matrix[i])
		{
			free_matrix_i(matrix, i);
			return (NULL);
		}
	}
	return (matrix);
}

unsigned char	**malloc_uc_matrix(int width, int height)
{
	unsigned char	**matrix;
	int i;

	i = -1;
	matrix = (unsigned char **)malloc(height * sizeof(unsigned char *));
	if (!matrix)
		return (NULL);
	while (++i < height)
	{
		matrix[i] = (unsigned char *)malloc(width * sizeof(unsigned char));
		if (!matrix[i])
		{
			//free_uc_matrix_i(matrix, i);
			return (NULL);
		}
	}
	return (matrix);
}

void	free_ui_matrix(unsigned int **matrix, int j)
{
	while (--j >= 0)
	{
		free (matrix[j]);
		matrix[j] = NULL;
	}
	free(matrix);
	matrix = NULL;
}

void	zero_ui_matrix(unsigned int **pixels_xl, int width, int height)
{
	int j;
	int i;
	
	j = -1;
	while (++j < height)
	{
		i = -1;
		while (++i < width)
			pixels_xl[j][i] = 0;
	}
}

unsigned int	**malloc_ui_matrix(int width, int height)
{
	unsigned int	**matrix;
	int i;

	i = -1;
	matrix = (unsigned int **)malloc(height * sizeof(unsigned int *));
	if (!matrix)
		return (NULL);
	while (++i < height)
	{
		matrix[i] = (unsigned int *)malloc(width * sizeof(unsigned int));
		if (!matrix[i])
		{
			free_ui_matrix(matrix, i);
			return (NULL);
		}
	}
	return (matrix);
}

void	free_int_matrix(int **matrix, int j)
{
	while (--j >= 0)
	{
		free (matrix[j]);
		matrix[j] = NULL;
	}
	free(matrix);
	matrix = NULL;
}

int	**malloc_int_matrix(int width, int height)
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
}

void	free_pix_matrix(t_pixel **matrix, int j)
{
	while (--j >= 0)
	{
		free (matrix[j]);
		matrix[j] = NULL;
	}
	free(matrix);
	matrix = NULL;
}

t_pixel	**malloc_pixel_matrix(int width, int height)
{
	t_pixel	**matrix;
	int i;

	i = -1;
	matrix = (t_pixel **)malloc(height * sizeof(t_pixel *));
	if (!matrix)
		return (NULL);
	while (++i < height)
	{
		matrix[i] = (t_pixel *)malloc(width * sizeof(t_pixel));
		if (!matrix[i])
		{
			while(--i >= 0)
				free(matrix[i]);
			free(matrix);
			return (NULL);
		}
	}
	return (matrix);
}

void	free_pix_d_matrix(t_pixeld **matrix, int j)
{
	while (--j >= 0)
	{
		free (matrix[j]);
		matrix[j] = NULL;
	}
	free(matrix);
	matrix = NULL;
}

t_pixeld	**malloc_pixel_d_matrix(int width, int height)
{
	t_pixeld	**matrix;
	int			i;

	i = -1;
	matrix = (t_pixeld **)malloc(height * sizeof(t_pixeld *));
	if (!matrix)
		return (NULL);
	while (++i < height)
	{
		matrix[i] = (t_pixeld *)malloc(width * sizeof(t_pixeld));
		if (!matrix[i])
			free_pix_d_matrix(matrix, i);
	}
	return (matrix);
}