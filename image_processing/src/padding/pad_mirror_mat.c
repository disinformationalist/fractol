#include "image_processing.h"

static inline void top_and_bottom_k(double **mat, double ***mirror_mat, int width, int height, int layers)
{
	int				x;
	int				y;

	y = -1;
	while (++y < layers)
	{
		x = -1;
		while (++x < width)
		{
			//top correct
			(*mirror_mat)[x + layers][y] = mat[x][layers - y];
			//bottom
			(*mirror_mat)[x + layers][height + layers + y] = mat[x][height - (y + 2)];
		}
	}
}

static inline void right_and_left_k(double **mat, double ***mirror_mat, int width, int height, int layers)
{
	int				x;
	int				y;

	y = -1;
	while (++y < height)
	{
		x = -1;
		while (++x < layers)
		{
			//left s
			(*mirror_mat)[x][y + layers] = mat[layers - x][y];
			//right s
			(*mirror_mat)[width + layers + x][y + layers] = mat[width - (x + 2)][y];
		}
	}
}

static inline void corners_k(double **mat, double ***mirror_mat, int width, int height, int layers)
{
	int				x;
	int				y;

	y = -1;
	while (++y < layers)
	{
		x = -1;
		while (++x < layers)
		{
			// top left
			(*mirror_mat)[x][y] = mat[layers - x][layers - y];
			//top right 
			(*mirror_mat)[width + layers + x][y] = mat[width - (x + 2)][layers - y];
			//bot left
			(*mirror_mat)[x][height + layers + y] = mat[layers - x][height - (y + 2)];
			// bot right
			(*mirror_mat)[width + layers + x][height + layers + y] = mat[width - (x + 2)][height - (y + 2)];
		}
	}
}

static void copy_center_k(double **mat, double ***mirror_mat, int width, int height, int layers)
{
	int				x;
	int				y;

	y = layers - 1;
	while (++y < height + layers)
	{
		x = layers - 1;
		while (++x < width + layers)
			(*mirror_mat)[x][y] = mat[x - layers][ y - layers];
	}
}

double	**pad_mirror_matrix(double **mat, int width, int height, int layers)
{
	double		**mirror_mat;

	/* width += 2 * layers;
	height += 2 * layers; */
	
	mirror_mat = malloc_matrix(height + layers * 2, width + layers * 2);
	if (!mirror_mat)
		return (NULL);
	copy_center_k(mat, &mirror_mat, width, height, layers);
	top_and_bottom_k(mat, &mirror_mat, width, height, layers);
	right_and_left_k(mat, &mirror_mat, width, height, layers);
	corners_k(mat, &mirror_mat, width, height, layers);
	return (mirror_mat);
}