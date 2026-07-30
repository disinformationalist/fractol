#include "fractol.h"

bool	buddha_should_rotate_display(int width, int height, int screen_height)
{
	return (height > width && height > screen_height);
}

void	display_to_render_coordinates(t_fractal *fractal, int *x, int *y)
{
	int	display_x;

	if (!fractal->display_rotated)
		return ;
	display_x = *x;
	*x = *y;
	*y = fractal->height_orig - 1 - display_x;
}

static void	rotate_clockwise(t_fractal *fractal)
{
	unsigned int	color;
	int				x;
	int				y;

	y = -1;
	while (++y < fractal->height_orig)
	{
		x = -1;
		while (++x < fractal->width_orig)
		{
			color = pixel_color_get(x, y, &fractal->img);
			my_pixel_put(fractal->height_orig - 1 - y, x,
				&fractal->display_img, color);
		}
	}
}

void	display_fractal_image(t_fractal *fractal)
{
	t_img	*display;

	display = &fractal->img;
	if (fractal->display_rotated)
	{
		rotate_clockwise(fractal);
		display = &fractal->display_img;
	}
	mlx_put_image_to_window(fractal->mlx_connect,
		fractal->mlx_win, display->img_ptr, 0, 0);
}
