#include "image_processing.h"
/* 
double patch_dist(, t_img *img_pad, int patch_area)//rework this
{
	double dist = 0.0;
	double pix_r;
	double pix_g;
	double pix_b;
	
	pix_r = 0;//maybe switch to use t_color
	pix_g = 0;
	pix_b = 0;
	
//need to compute for each pixel in p and neighbor q, take average.
	f->j = -f->half_k - 1;
	while (++f->j <= f->half_k)
	{
		f->i = -f->half_k - 1;
		while (++f->i <= f->half_k)
		{
			f->pixel = pixel_color_get(f->x_r + f->i, f->y_r + f->j, img_pad);
			dist += delta(f->pixel >> 16) & 0xFF;
			dist += (f->pixel >> 8) & 0xFF;
			dist += f->pixel & 0xFF;
		}
	}
	return (dist / (3 * patch_area));
} */

//computes the samp var for a pixel
//CONSIDER CLEANING UP TYPES TO DOUBLE UNTIL VERY END...

t_pixeld calc_pix_local_mean(t_pixeld **pad, double patch_area, int half_k, int x, int y)
{
	t_pixeld			pixel;
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

	/* t_pixel	mean_ret;
	mean_ret.red = (int)mean.red;
	mean_ret.green = (int)mean.green;
	mean_ret.blue = (int)mean.blue; */

	return (mean);
}

t_pixeld	calc_local_variance(t_pixeld **img_pad, double patch_area, int half_k, int x, int y)
{
	t_pixeld			mean;
	t_pixeld		variance;
	t_pixeld			pixel;
	int				i;
	int				j;
	
	variance.red = 0;
	variance.green = 0;
	variance.blue = 0;
	j = -half_k - 1;
		mean = calc_pix_local_mean(img_pad, patch_area, half_k, x, y);
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
	/* t_pixel var;
	var.red = (int)variance.red;
	var.green = (int)variance.green;
	var.blue = (int)variance.blue; */

	//return (var);
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
//orig wid and h sent to this
unsigned int	**calc_variances(t_pixeld **p_img_pad, t_pixeld **odds_pad, t_pixeld **evens_pad, int width, int height, int patch_size)
{
	t_pixeld **local_var;
	//t_pixeld **half_buff_var;
	int	pad_layers;
	int y;
	int x;
	int	half_k;
	double patch_area;


	t_pixeld	sv_pix;
	t_pixeld	hbv_pix;
	
	half_k = patch_size / 2;
	patch_area = patch_size * patch_size;
	pad_layers = half_k * 2;
	local_var = malloc_pixel_d_matrix(width, height);
	if (!local_var)
		return (NULL);
	//half_buff_var = malloc_pixel_d_matrix(width, height);
	//pro
	//set full buff and smooth..
	int y_adj;
	int x_adj;
	y = pad_layers - 1;
	while (++y < height + pad_layers)
	{
		x = pad_layers - 1;
		while (++x < width + pad_layers)
		{
			y_adj = y - pad_layers;
			x_adj = x - pad_layers;
			local_var[y_adj][x_adj] = calc_local_variance(p_img_pad, patch_area, half_k, x, y);
			/* if (x % 2 == 0)
				half_buff_var[y_adj][x_adj] = calc_local_variance(evens_pad, patch_area, half_k, x / 2, y);
			else
				half_buff_var[y_adj][x_adj] = calc_local_variance(odds_pad, patch_area, half_k, x / 2, y); */
		}
	}
	smooth_variance(&local_var, width, height, 21);//change to 21 later 3 for test.
	smooth_variance(&half_buff_var, width, height, 21);
	//printf("SMOOTHING DONE\n");
	double ratio;
	unsigned int **vars;
	vars = malloc_ui_matrix(width, height);
	
			int r;
			int g;
			int b;
	y = -1;
	while (++y < height)
	{
		x = -1;
		while (++x < width)
		{
			//double ratio = local_var[y][x] / half_buff_var[y][x];
		
			sv_pix = local_var[y][x];
			hbv_pix = half_buff_var[y][x];
			ratio = (sv_pix.red /(hbv_pix.red + .0001) + sv_pix.green / (hbv_pix.green + .0001) + sv_pix.blue / (hbv_pix.blue + .0001)) / 3.0;
			//printf("here is the ration: %f\n", ratio);


			r = (int)(local_var[y][x].red * ratio);
			if (r > 255)
				r = 255;
			if (r < 0)
				r = 0;
			g = (int)(local_var[y][x].green * ratio);
			if (g > 255)
				g = 255;
			if (g < 0)
				g = 0;
			b = (int)(local_var[y][x].blue * ratio);
			if (b > 255)
				b = 255;
			if (b < 0)
				b = 0;
			
			vars[y][x] = (r << 16) | (g << 8) | b;
			//vars[y][x] = (int)((((double)local_var[y][x].red + (double)local_var[y][x].green + (double)local_var[y][x].blue) / 3.0) * ratio);


		}
	}
	free_pix_d_matrix(local_var, height);
	free_pix_d_matrix(half_buff_var, height);

	return (vars);
}

void	img_to_tpix_h(t_img *img, t_pixeld ***p_img, t_pixeld ***odds, t_pixeld ***evens, int width, int height)
{
	unsigned int pixel;
	t_pixeld	pixel_t;
	int x;
	int y;


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
			if (x % 2 == 0)
				(*evens)[y][x / 2] = pixel_t;
			else
				(*odds)[y][x / 2] = pixel_t;
		}
	}
}

t_img	*nlm_rouselle_vars(void *mlx_ptr, t_img *img, int width, int height, int patch_size)
{
	int 		pad_layers;
	//int			patch_area;
	int			half_k;

	t_pixeld **p_img;
	t_pixeld **odds;
	t_pixeld **evens;

	t_pixeld **p_img_pad;
	t_pixeld **odds_pad;
	t_pixeld **evens_pad;

	p_img = malloc_pixel_d_matrix(width, height);
	odds = malloc_pixel_d_matrix(width / 2, height);
	evens = malloc_pixel_d_matrix(width / 2, height);

	img_to_tpix_h(img, &p_img, &odds, &evens, width, height);
	
	half_k = patch_size / 2;
	pad_layers = half_k * 2;
	//patch_area = patch_size * patch_size;
	
	/* p_img_pad = pad_mirror_pix(p_img, width, height, pad_layers);
	odds_pad = pad_mirror_pix(odds, width / 2, height, pad_layers);
	evens_pad = pad_mirror_pix(evens, width / 2, height, pad_layers);
 */
	p_img_pad = pad_zeros_pix_d(p_img, width, height, pad_layers);
	odds_pad = pad_zeros_pix_d(odds, width / 2, height, pad_layers);
	evens_pad = pad_zeros_pix_d(evens, width / 2, height, pad_layers);
	free_pix_d_matrix(p_img, height);
	free_pix_d_matrix(odds, height);
	free_pix_d_matrix(evens, height);

	unsigned int **vars;
	vars = calc_variances(p_img_pad, odds_pad, evens_pad, width, height, patch_size);

	
	free_pix_d_matrix(p_img_pad, height + 2 * pad_layers);
	free_pix_d_matrix(odds_pad, height + 2 * pad_layers);
	free_pix_d_matrix(evens_pad, height + 2 * pad_layers);

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