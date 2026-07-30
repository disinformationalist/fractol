#include "fractol.h"

void	set_pieces(t_fractal *fractal, t_piece piece[][fractal->num_cols], int i, int j)
{
	piece[j][i].x_s = i * (fractal->width / fractal->num_cols);
	piece[j][i].x_e = (i + 1) * (fractal->width / fractal->num_cols);
	piece[j][i].y_s = j * (fractal->height / fractal->num_rows);
	piece[j][i].y_e = (j + 1) * (fractal->height / fractal->num_rows);
	if (i + 1 == fractal->num_cols)
		piece[j][i].x_e = fractal->width;
	if (j + 1 == fractal->num_rows)
		piece[j][i].y_e = fractal->height;
	piece[j][i].fractal = fractal;
}
