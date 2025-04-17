#include "image_processing.h"

static inline void top_and_bottom_k(t_pixel **pix, t_pixel ***mirror, int width, int height, int layers)
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
			(*mirror)[x + layers][y] = pix[x][layers - y];
			//bottom
			(*mirror)[x + layers][height + layers + y] = pix[x][height - (y + 2)];
		}
	}
}

static inline void right_and_left_k(t_pixel **pix, t_pixel ***mirror, int width, int height, int layers)
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
			(*mirror)[x][y + layers] = pix[layers - x][y];
			//right s
			(*mirror)[width + layers + x][y + layers] = pix[width - (x + 2)][y];
		}
	}
}

static inline void corners_k(t_pixel **pix, t_pixel ***mirror, int width, int height, int layers)
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
			(*mirror)[x][y] = pix[layers - x][layers - y];
			//top right 
			(*mirror)[width + layers + x][y] = pix[width - (x + 2)][layers - y];
			//bot left
			(*mirror)[x][height + layers + y] = pix[layers - x][height - (y + 2)];
			// bot right
			(*mirror)[width + layers + x][height + layers + y] = pix[width - (x + 2)][height - (y + 2)];
		}
	}
}

static void copy_center_k(t_pixel **pix, t_pixel ***mirror, int width, int height, int layers)
{
	int				x;
	int				y;

	y = layers - 1;
	while (++y < height + layers)
	{
		x = layers - 1;
		while (++x < width + layers)
			(*mirror)[x][y] = pix[x - layers][ y - layers];
	}
}

t_pixel	**pad_mirror_pix(t_pixel **pix, int width, int height, int layers)
{
	t_pixel		**mirror_pix;

	/* width += 2 * layers;
	height += 2 * layers; */
	
	mirror_pix = malloc_pixel_matrix(height + layers * 2, width + layers * 2);
	if (!mirror_pix)
		return (NULL);
	copy_center_k(pix, &mirror_pix, width, height, layers);
	top_and_bottom_k(pix, &mirror_pix, width, height, layers);
	right_and_left_k(pix, &mirror_pix, width, height, layers);
	corners_k(pix, &mirror_pix, width, height, layers);
	return (mirror_pix);
}


void	set_rows_zero(t_pixel ***pad, int width, int s_col_index, int s_row_index, int num_rows)
{
	int	x;
	int	y;

	y = -1;
	while (++y < num_rows)
	{
		x = -1;
		while (++x < width)
		{
			(*pad)[s_row_index + y][s_col_index + x].red = 0;
			(*pad)[s_row_index + y][s_col_index + x].green = 0;
			(*pad)[s_row_index + y][s_col_index + x].blue = 0;
		}
	}
}

t_pixel **pad_zeros_pix(t_pixel **pix, int width, int height, int layers)
{
	t_pixel **pad;
	int x;
	int y;

	pad = malloc_pixel_matrix(width + layers * 2, height + layers * 2);
	if (!pad)
		return (NULL);
	set_rows_zero(&pad, width + 2 * layers, 0, 0, layers);
	y = -1;
	while (++y < height)
	{
		set_rows_zero(&pad, layers, 0, y + layers, 1);
		x = -1;
		while (++x < width)
			pad[y + layers][x + layers] = pix[y][x];
		set_rows_zero(&pad, layers, layers + width, y + layers, 1);
	}
	set_rows_zero(&pad, width + 2 * layers, 0, height + layers, layers);
	return (pad);
}

void	set_rows_zero_d(t_pixeld ***pad, int width, int s_col_index, int s_row_index, int num_rows)
{
	int	x;
	int	y;

	y = -1;
	while (++y < num_rows)
	{
		x = -1;
		while (++x < width)
		{
			(*pad)[s_row_index + y][s_col_index + x].red = 0;
			(*pad)[s_row_index + y][s_col_index + x].green = 0;
			(*pad)[s_row_index + y][s_col_index + x].blue = 0;
		}
	}
}

t_pixeld **pad_zeros_pix_d(t_pixeld **pix, int width, int height, int layers)
{
	t_pixeld **pad;
	int x;
	int y;

	pad = malloc_pixel_d_matrix(width + layers * 2, height + layers * 2);
	if (!pad)
		return (NULL);
	set_rows_zero_d(&pad, width + 2 * layers, 0, 0, layers);
	y = -1;
	while (++y < height)
	{
		set_rows_zero_d(&pad, layers, 0, y + layers, 1);
		x = -1;
		while (++x < width)
			pad[y + layers][x + layers] = pix[y][x];
		set_rows_zero_d(&pad, layers, layers + width, y + layers, 1);
	}
	set_rows_zero_d(&pad, width + 2 * layers, 0, height + layers, layers);
	return (pad);
}