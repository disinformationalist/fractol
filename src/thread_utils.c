#include "fractol.h"

void	set_pieces_half(t_fractal *fractal, t_piece piece[][fractal->num_cols], int i, int j) 
{
	double n;
	
	if (fractal->id == 3)
		n = fractal->buddha->n;
	else
		n = 1;

	piece[j][i].x_s = i * ((fractal->width * n) / fractal->num_cols);
	piece[j][i].x_e = (i + 1) * ((fractal->width * n) / fractal->num_cols);
	piece[j][i].y_s = j * (((fractal->height / 2) * n) / fractal->num_rows);
	piece[j][i].y_e = (j + 1) * (((fractal->height / 2) * n) / fractal->num_rows);
	piece[j][i].fractal = fractal;
}

void	set_pieces(t_fractal *fractal, t_piece piece[][fractal->num_cols], int i, int j)
{
	double n;
	
	if (fractal->id == 3)
		n = fractal->buddha->n;
	else
		n = 1;
	piece[j][i].x_s = i * ((fractal->width * n) / fractal->num_cols);
	piece[j][i].x_e = (i + 1) * ((fractal->width * n) / fractal->num_cols);
	piece[j][i].y_s = j * ((fractal->height * n) / fractal->num_rows);
	piece[j][i].y_e = (j + 1) * ((fractal->height * n) / fractal->num_rows);
	piece[j][i].fractal = fractal;
}

