#include "image_processing.h"
/* 

double calc_delta(int p_val, int q_val, double var_p double var_q, double eps, double kc)
{
	double diff = (p_val - q_val) * (p_val -  q_val);
	double var;
	var = var_p + min(var_p, var_q);

	return ((diff - var) / (eps + kc * kc * var));
}
double calc_weight(double patch_dist)
{
	return exp(-fmax(0, patch_dist));
}


double patch_dist(t_img *img_pad, double vars_p, double vars_q, int patch_area double eps, double kc)//rework this
{
	int i;
	int j;

	
	double dist = 0.0;
	t_pix_d patch_p;
	
	pix.red = 0;
	pix.green = 0;
	pix.blue = 0;
	
//need to compute for each pixel in p and neighbor q, take average.
	j = -half_k - 1;
	while (j <= half_k)
	{
		i = -half_k - 1;
		while (i <= half_k)
		{
			pixel = pixel_color_get(x_r + i, y_r + j, img_pad);
			pixel.red = (pixel >> 16) & 0xFF;
			pixel.green = (pixel >> 8) & 0xFF;
			pixel.blue = pixel & 0xFF;
			
			
			dist += calc_delta(fpixel >> 16) & 0xFF;
			dist += (pixel >> 8) & 0xFF;
			dist += pixel & 0xFF;
		}
	}
	return (dist / (3 * patch_area));
} */

//computes the samp var for a pixel
//CONSIDER CLEANING UP TYPES TO DOUBLE UNTIL VERY END...
//creating window sized squares for some reason??
t_pixeld calc_pix_local_mean(t_pixeld **pad, double patch_area, int half_k, int x, int y)
{
	t_pixeld		pixel;
	t_pixeld		mean;
	int				i;
	int				j;

	mean.red = 0;
	mean.green = 0;
	mean.blue = 0;
	j = -half_k - 1;
	while(++j <= half_k)
	{
		i = -half_k - 1;
		while (++i <= half_k)
		{
			pixel = pad[y + j][x + i];
			mean.red += pixel.red;
			mean.green += pixel.green;
			mean.blue += pixel.blue;
		}
	}
	mean.red /= patch_area;
	mean.green /= patch_area;
	mean.blue /= patch_area;
	return (mean);
}

t_pixeld	calc_local_variance(t_pixeld **img_pad, double patch_area, int half_k, int x, int y)
{
	t_pixeld			mean;
	t_pixeld			variance;
	t_pixeld			pixel;
	int					i;
	int					j;
	
	variance.red = 0;
	variance.green = 0;
	variance.blue = 0;
	mean = calc_pix_local_mean(img_pad, patch_area, half_k, x, y);
	j = -half_k - 1;
	while(++j <= half_k)
	{
		i = -half_k - 1;
		while (++i <= half_k)
		{
			pixel = img_pad[y + j][x + i];
			variance.red += (pixel.red - mean.red) * (pixel.red - mean.red);
			variance.green += (pixel.green - mean.green) * (pixel.green - mean.green);
			variance.blue += (pixel.blue - mean.blue) * (pixel.blue - mean.blue);
		}
	}

	variance.red /= patch_area;
	variance.green /= patch_area;
	variance.blue /= patch_area;
	return (variance);
}

void	smooth_variance(t_pixeld ***var, int width, int height, int kern)
{
	t_pixeld **var_pad;
	int		half_k;
	int		y;
	int		x;

	half_k = kern / 2;
	//var_pad = pad_mirror_pix(*var, width, height, half_k);
	var_pad = pad_zeros_pix_d(*var, width, height, half_k);
	
	y = half_k - 1;
	while (++y < height + half_k)
	{
		x = half_k - 1;
		while (++x < width + half_k)
			(*var)[y - half_k][x - half_k] = calc_pix_local_mean(var_pad, kern * kern, half_k, x, y);

	}
	free_pix_d_matrix(var_pad, height + half_k * 2);
}

int	clamp_rgb(double val)
{
	int rgb;

	rgb = (int)val;
	if (rgb > 255)
		rgb = 255;
	if (rgb < 0)
		rgb = 0;
	return (rgb);
}

unsigned int	**vars_to_ui(t_pixeld **local_vars, int width, int height)
{
	unsigned int **vars;
	vars = malloc_ui_matrix(width, height);
	int y;
	int x;
	int r;
	int g;
	int b;
	y = -1;
	while (++y < height)
	{
		x = -1;
		while (++x < width)
		{
			r = clamp_rgb(local_vars[y][x].red);
			g = clamp_rgb(local_vars[y][x].green);
			b = clamp_rgb(local_vars[y][x].blue);
			vars[y][x] = (r << 16) | (g << 8) | b;
		}
	}
	free_pix_d_matrix(local_vars, height);
	return (vars);
}


unsigned int	**calc_variances(t_pixeld **p_img_pad, int width, int height, int patch_size)
{
	t_pixeld		**local_vars;
	int				pad_layers;
	int				y;
	int				x;
	int				half_k;
	double			patch_area;
	unsigned int	**vars;

	half_k = patch_size / 2;
	patch_area = patch_size * patch_size;
	pad_layers = half_k * 2;
	local_vars = malloc_pixel_d_matrix(width, height);
	if (!local_vars)
		return (NULL);
	//set full buff and smooth..
	
	y = -1;
	while (++y < height)
	{
		x = -1;
		while (++x < width)
		{
			local_vars[y][x] = calc_local_variance(p_img_pad, patch_area, half_k, x + pad_layers, y + pad_layers);
		}
	}
	//smooth_variance(&local_vars, width, height, 3);//change to 21 later 3 for test.
	vars = vars_to_ui(local_vars, width, height);
	return (vars);
}

void	img_to_tpix_r(t_img *img, t_pixeld ***p_img, int width, int height)
{
	unsigned int	pixel;
	t_pixeld		pixel_t;
	int				x;
	int				y;

	y = -1;
	while (++y < height)
	{
		x = -1;
		while (++x < width)
		{	
			pixel = pixel_color_get(x, y, img);
			pixel_t.red = (pixel >> 16) & 0xFF;
			pixel_t.green = (pixel >> 8) & 0xFF;
			pixel_t.blue = pixel & 0xFF;
			(*p_img)[y][x] = pixel_t;
		}
	}
}

t_img	*nlm_rouselle_vars(void *mlx_ptr, t_img *img, int width, int height, int patch_size)
{
	int 			pad_layers;
	int				half_k;
	t_pixeld		**p_img;
	t_pixeld		**p_img_pad;
	unsigned int	**vars;
	
	p_img = malloc_pixel_d_matrix(width, height);
	img_to_tpix_r(img, &p_img, width, height);
	half_k = patch_size / 2;
	pad_layers = half_k * 2;
	// p_img_pad = pad_mirror_pix(p_img, width, height, pad_layers); MUST TEST TO WORK WITH MIRROR
	p_img_pad = pad_zeros_pix_d(p_img, width, height, pad_layers);
	free_pix_d_matrix(p_img, height);
	vars = calc_variances(p_img_pad, width, height, patch_size);
	free_pix_d_matrix(p_img_pad, height + 2 * pad_layers);
//----------------------------img for testing/viewing...
	t_img	*var_img = create_img(mlx_ptr, width, height);
	int i;
	int j;
	j = -1;
	while (++j < height)
	{
		i = -1;
		while (++i < width)
			my_pixel_put(i, j, var_img, vars[j][i]);
	}
	free_ui_matrix(vars, height);
	return (var_img);
}
