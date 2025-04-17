#include "image_processing.h"

static int round_to_int(double d)
{
	return(floor(d + 0.5));
}

int	recolor_channel(int chan_color)//can make generalized extract and re with a passed in function.
{
	double ratio;
	int	new_val;
	
	ratio = (double)chan_color / 255.0;
	new_val = round_to_int(sqrt(ratio) * 255.0);
	if (new_val > 255) 
			new_val = 255;
	return (new_val);
}

void	recolor_pixel(t_img *img, int x, int y, t_channel c)
{
	unsigned int	pixel;
	int				red;
	int				green;
	int				blue;

	pixel = pixel_color_get(x, y, img);
	red = (int)((pixel >> 16) & 0xFF);
	green = (int)((pixel >> 8) & 0xFF);
	blue = (int)(pixel & 0xFF);
	if (c & R)
		red = recolor_channel(red);
	if (c & G)
		green = recolor_channel(green);
	if (c & B)
		blue = recolor_channel(blue);
	my_pixel_put(x, y, img, (red << 16 | green << 8 | blue));
}

void re_color_buddha(t_img *img, int width, int height, t_channel c)
{
	int i = 0;
	int j = 0;
	
	/* j = piece->y_s - 1;
	while (++j < piece->y_e)
	{
		i = piece->x_s - 1;
		while (++i < piece->x_e)
		{
			recolor_pixel(img, i, j, c);
		}
	} */

	j = -1;
	while (++j < height)
	{
		i = -1;
		while (++i < width)
		{
			recolor_pixel(img, i, j, c);
		}
	}
}
