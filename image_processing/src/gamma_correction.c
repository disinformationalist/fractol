#include "image_processing.h"

static int	correct_channel(double channel, double inv_gamma)
{
	double corrected;
	
	corrected = pow(channel / 255.0f, inv_gamma);
	return ((uint8_t)(corrected * 255.0f));
}

void	apply_gamma_action(t_img *img, t_position pos, double inv_gamma, t_channel c)
{
	unsigned int	pixel;
	uint8_t			red;
	uint8_t			green;
	uint8_t			blue;

	pixel = pixel_color_get(pos.x, pos.y, img);
	red = (int)((pixel >> 16) & 0xFF);
	green = (int)((pixel >> 8) & 0xFF);
	blue = (int)(pixel & 0xFF);
	if (c & R)
		red = correct_channel(red, inv_gamma);
	if (c & G)
		green = correct_channel(green, inv_gamma);
	if (c & B)
		blue = correct_channel(blue, inv_gamma);
	my_pixel_put(pos.x, pos.y, img, (red << 16 | green << 8 | blue));
}

void	gamma_correct_rgb(t_img *img, int width, int height, double gamma, t_channel c)
{
	t_position	pos;
	double		inv_gamma;

	inv_gamma = 1.0 / gamma;
	pos.y = -1;
	while (++pos.y < height)
	{
		pos.x = -1;
		while (++pos.x < width)
			apply_gamma_action(img, pos, inv_gamma, c);
	}
}
