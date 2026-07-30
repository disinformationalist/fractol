#include "fractol.h"

typedef struct s_export16
{
	t_fractal	*fractal;
	double		fern_high;
	double		*filter_rows;
	double		filter_kernel[25];
	double		filter_kernel_sum;
	int			filter_row_id[5];
	png_bytep	row;
}	t_export16;

static void	free_metadata(png_text *text)
{
	int	index;

	if (!text)
		return ;
	index = 0;
	while (text[index].text)
	{
		free(text[index].key);
		free(text[index].text);
		index++;
	}
	free(text[index].key);
	free(text);
}

static uint16_t	to_u16(double value)
{
	if (value < 0.0)
		value = 0.0;
	if (value > 1.0)
		value = 1.0;
	return ((uint16_t)floor(value * 65535.0 + 0.5));
}

static void	preview_rgb(t_fractal *fractal, int x, int y, double rgb[3])
{
	unsigned int	color;

	color = pixel_color_get(x, y, &fractal->img);
	rgb[0] = (double)((color >> 16) & 0xFFu) / 255.0;
	rgb[1] = (double)((color >> 8) & 0xFFu) / 255.0;
	rgb[2] = (double)(color & 0xFFu) / 255.0;
}

static int	mirror_coordinate(int coordinate, int size)
{
	if (size <= 1)
		return (0);
	while (coordinate < 0 || coordinate >= size)
	{
		if (coordinate < 0)
			coordinate = -coordinate;
		else
			coordinate = 2 * size - 2 - coordinate;
	}
	return (coordinate);
}

static void	make_export_gaussian(t_export16 *export)
{
	double	sigma;
	double	value;
	int		x;
	int		y;

	sigma = (double)((export->fractal->buddha->flevel - 1) >> 1) * 0.5;
	if (sigma <= 0.0)
		sigma = 0.5;
	export->filter_kernel_sum = 0.0;
	y = -1;
	while (++y < 5)
	{
		x = -1;
		while (++x < 5)
		{
			value = exp(-((double)((x - 2) * (x - 2)
							+ (y - 2) * (y - 2))
						/ (2.0 * sigma * sigma)))
				/ (2.0 * M_PI * sigma * sigma);
			export->filter_kernel[y * 5 + x] = round(value * 276.0);
			export->filter_kernel_sum += export->filter_kernel[y * 5 + x];
		}
	}
}

static bool	uses_high_precision_gaussian(t_fractal *fractal)
{
	return (fractal->id == 3 && fractal->buddha && fractal->densities
		&& fractal->buddha->filter && fractal->buddha->ftype == GAUSS
		&& !fractal->buddha->importance_view);
}

static int	init_export_filter(t_export16 *export)
{
	size_t	values;
	int		row;

	if (!uses_high_precision_gaussian(export->fractal))
		return (0);
	values = (size_t)export->fractal->width_orig * 5 * BUDDHA_CHANNELS;
	export->filter_rows = malloc(values * sizeof(*export->filter_rows));
	if (!export->filter_rows)
		return (-1);
	row = -1;
	while (++row < 5)
		export->filter_row_id[row] = -1;
	make_export_gaussian(export);
	return (0);
}

static int	find_filter_row(t_export16 *export, int source_y)
{
	int	slot;

	slot = -1;
	while (++slot < 5)
		if (export->filter_row_id[slot] == source_y)
			return (slot);
	return (-1);
}

static bool	row_is_required(int row_id, int required[5])
{
	int	index;

	index = -1;
	while (++index < 5)
		if (required[index] == row_id)
			return (true);
	return (false);
}

static int	load_filter_row(t_export16 *export, int source_y, int required[5])
{
	double	*row;
	double	rgb[3];
	int		slot;
	int		channel;
	int		x;

	slot = find_filter_row(export, source_y);
	if (slot >= 0)
		return (slot);
	slot = -1;
	while (++slot < 5)
		if (!row_is_required(export->filter_row_id[slot], required))
			break ;
	if (slot == 5)
		slot = 0;
	row = export->filter_rows + (size_t)slot
		* (size_t)export->fractal->width_orig * BUDDHA_CHANNELS;
	x = -1;
	while (++x < export->fractal->width_orig)
	{
		buddha_pixel_rgb(export->fractal, x, source_y, rgb);
		channel = -1;
		while (++channel < BUDDHA_CHANNELS)
			row[(size_t)x * BUDDHA_CHANNELS + (size_t)channel]
				= rgb[channel];
	}
	export->filter_row_id[slot] = source_y;
	return (slot);
}

static void	gaussian_buddha_rgb(t_export16 *export, int x, int y,
		double rgb[3])
{
	double	*row;
	int		required[5];
	int		slot[5];
	int		source_x;
	int		kx;
	int		ky;
	int		channel;

	memset(rgb, 0, sizeof(double) * BUDDHA_CHANNELS);
	ky = -1;
	while (++ky < 5)
		required[ky] = mirror_coordinate(y + ky - 2,
				export->fractal->height_orig);
	ky = -1;
	while (++ky < 5)
		slot[ky] = load_filter_row(export, required[ky], required);
	ky = -1;
	while (++ky < 5)
	{
		row = export->filter_rows + (size_t)slot[ky]
			* (size_t)export->fractal->width_orig * BUDDHA_CHANNELS;
		kx = -1;
		while (++kx < 5)
		{
			source_x = mirror_coordinate(x + kx - 2,
					export->fractal->width_orig);
			channel = -1;
			while (++channel < BUDDHA_CHANNELS)
				rgb[channel] += row[(size_t)source_x * BUDDHA_CHANNELS
						+ (size_t)channel]
					* export->filter_kernel[ky * 5 + kx];
		}
	}
	channel = -1;
	while (++channel < BUDDHA_CHANNELS)
		rgb[channel] /= export->filter_kernel_sum;
}

static void	adjust_buddha_rgb(t_fractal *fractal, int x, int y,
		double rgb[3])
{
	double	multiplier;
	int		channel;

	buddha_pixel_rgb(fractal, x, y, rgb);
	multiplier = (double)((fractal->buddha->flevel - 1) >> 1) * 0.5 + 1.0;
	channel = -1;
	while (++channel < BUDDHA_CHANNELS)
		rgb[channel] *= multiplier;
}

static int	image_color_index(t_fractal *fractal)
{
	int	index;

	index = fractal->col_i;
	if (!fractal->toggle_color && fractal->num_colors > 0)
		index = (index - 7 + fractal->num_colors) % fractal->num_colors;
	return (index);
}

static int	pixel_base_color(t_fractal *fractal, int x, int y,
		int width, int height)
{
	t_complex	d;
	int			angle;
	int			index;

	index = image_color_index(fractal);
	if (!fractal->color_spectrum)
		return (fractal->w_colors[index]);
	d.x = map(x, -2, +2, width) - fractal->move_col_x;
	d.y = map(y, +2, -2, height) + fractal->move_col_y;
	angle = (int)floor((atan2(d.y, d.x) * RAD_TO_DEG + 360.0)
			+ 360.0) % 360;
	return (fractal->w_colors[(angle + index) % fractal->num_colors]);
}

static void	scale_base_color(int base, double iteration, double maximum,
		double rgb[3])
{
	double	scale;

	if (iteration >= maximum || maximum <= 0.0)
	{
		memset(rgb, 0, sizeof(double) * 3);
		return ;
	}
	scale = iteration / maximum;
	rgb[0] = scale * (double)((base >> 16) & 0xFFu) / 255.0;
	rgb[1] = scale * (double)((base >> 8) & 0xFFu) / 255.0;
	rgb[2] = scale * (double)(base & 0xFFu) / 255.0;
}

static double	mandelbrot_iterations(t_fractal *fractal, double cx, double cy)
{
	double	zx;
	double	zy;
	double	zx2;
	double	zy2;
	int		iterations;

	zx = 0.0;
	zy = 0.0;
	iterations = 0;
	while (zx * zx + zy * zy < fractal->bound
		&& iterations < fractal->max_i)
	{
		zx2 = zx * zx - zy * zy + cx;
		zy2 = 2.0 * zx * zy + cy;
		zx = zx2;
		zy = zy2;
		iterations++;
	}
	return ((double)iterations);
}

static double	julia_iterations(t_fractal *fractal, double zx, double zy)
{
	double	next_x;
	double	magnitude;
	int		iterations;

	iterations = 0;
	while (zx * zx + zy * zy < fractal->bound
		&& iterations < fractal->max_i)
	{
		next_x = zx * zx - zy * zy + fractal->julia_x;
		zy = 2.0 * zx * zy + fractal->julia_y;
		zx = next_x;
		iterations++;
	}
	if (iterations >= fractal->max_i)
		return ((double)iterations);
	magnitude = sqrt(zx * zx + zy * zy);
	return ((double)iterations + 1.0
		- log(log(magnitude)) / log(2.0));
}

static void	escape_source_rgb(t_fractal *fractal, int x, int y,
		int width, int height, double rgb[3])
{
	double	aspect;
	double	zoom_x;
	double	cx;
	double	cy;
	double	iteration;
	int		base;

	aspect = (double)width / (double)height;
	zoom_x = fractal->zoom * aspect;
	cx = map(x, -2, +2, width) * zoom_x
		+ fractal->move_x * aspect;
	cy = map(y, +2, -2, height) * fractal->zoom - fractal->move_y;
	if (fractal->id == 1)
		iteration = mandelbrot_iterations(fractal, cx, cy);
	else
		iteration = julia_iterations(fractal, cx, cy);
	base = pixel_base_color(fractal, x, y, width, height);
	scale_base_color(base, iteration, fractal->max_i, rgb);
}

static void	escape_pixel_rgb(t_fractal *fractal, int x, int y,
		double rgb[3])
{
	double	sample[3];
	int		width;
	int		height;
	int		scale;
	int		sx;
	int		sy;
	int		channel;

	memset(rgb, 0, sizeof(double) * 3);
	scale = 1;
	if (fractal->supersample)
		scale = fractal->s_kernel;
	width = fractal->width_orig * scale;
	height = fractal->height_orig * scale;
	sy = y * scale - 1;
	while (++sy < (y + 1) * scale)
	{
		sx = x * scale - 1;
		while (++sx < (x + 1) * scale)
		{
			escape_source_rgb(fractal, sx, sy, width, height, sample);
			channel = -1;
			while (++channel < 3)
				rgb[channel] += sample[channel];
		}
	}
	channel = -1;
	while (++channel < 3)
		rgb[channel] /= (double)(scale * scale);
}

static void	fern_source_rgb(t_export16 *export, int x, int y, double rgb[3])
{
	t_fractal	*f;
	double		position;
	double		segment;
	double		amount;
	int			colors[4];
	int			low;
	int			high;
	int			channel;
	int			shift;

	f = export->fractal;
	memset(rgb, 0, sizeof(double) * 3);
	if (f->fdensity[y][x] <= 0 || export->fern_high <= 1.0)
		return ;
	colors[0] = f->colors.color_1;
	colors[1] = f->colors.color_2;
	colors[2] = f->colors.color_3;
	colors[3] = f->colors.color_4;
	position = log((double)f->fdensity[y][x]) / log(export->fern_high);
	if (position < 0.0)
		position = 0.0;
	if (position > 1.0)
		position = 1.0;
	segment = position * 3.0;
	low = (int)segment;
	if (low > 2)
		low = 2;
	high = low + 1;
	amount = segment - (double)low;
	if (position >= 1.0)
		amount = 1.0;
	channel = -1;
	while (++channel < 3)
	{
		shift = 16 - channel * 8;
		rgb[channel] = ((double)((colors[low] >> shift) & 0xFFu)
				* (1.0 - amount)
				+ (double)((colors[high] >> shift) & 0xFFu) * amount)
			/ 255.0;
	}
}

static void	fern_pixel_rgb(t_export16 *export, int x, int y, double rgb[3])
{
	double	sample[3];
	int		scale;
	int		sx;
	int		sy;
	int		channel;

	memset(rgb, 0, sizeof(double) * 3);
	scale = 1;
	if (export->fractal->supersample)
		scale = export->fractal->s_kernel;
	sy = y * scale - 1;
	while (++sy < (y + 1) * scale)
	{
		sx = x * scale - 1;
		while (++sx < (x + 1) * scale)
		{
			fern_source_rgb(export, sx, sy, sample);
			channel = -1;
			while (++channel < 3)
				rgb[channel] += sample[channel];
		}
	}
	channel = -1;
	while (++channel < 3)
		rgb[channel] /= (double)(scale * scale);
}

static void	export_pixel_rgb(t_export16 *export, int x, int y, double rgb[3])
{
	t_fractal	*f;

	f = export->fractal;
	if (f->id == 3 && f->buddha->importance_view
		&& f->buddha->importance_ready && f->buddha->importance_values)
		buddha_importance_pixel_rgb(f, x, y, rgb);
	else if (uses_high_precision_gaussian(f))
		gaussian_buddha_rgb(export, x, y, rgb);
	else if (f->id == 3 && f->densities && f->buddha->filter
		&& f->buddha->ftype == ADJUST && !f->buddha->importance_view)
		adjust_buddha_rgb(f, x, y, rgb);
	else if (f->id == 3 && f->densities && !f->buddha->importance_view)
		buddha_pixel_rgb(f, x, y, rgb);
	else if ((f->id == 1 || f->id == 2) && f->w_colors)
		escape_pixel_rgb(f, x, y, rgb);
	else if (f->id == 4 && f->fdensity)
		fern_pixel_rgb(export, x, y, rgb);
	else
		preview_rgb(f, x, y, rgb);
}

static void	pack_row(t_export16 *export, png_bytep row, int y)
{
	double		rgb[3];
	uint16_t	value;
	int			channel;
	int			x;

	x = -1;
	while (++x < export->fractal->width_orig)
	{
		export_pixel_rgb(export, x, y, rgb);
		channel = -1;
		while (++channel < 3)
		{
			value = to_u16(rgb[channel]);
			*row++ = (png_byte)(value >> 8);
			*row++ = (png_byte)value;
		}
	}
}

static int	write_png_rows(png_structp png, png_infop info, FILE *file,
		t_export16 *export, png_text *text)
{
	int			count;
	int			y;

	png_init_io(png, file);
	png_set_IHDR(png, info, export->fractal->width_orig,
		export->fractal->height_orig, 16, PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	png_set_sRGB(png, info, PNG_sRGB_INTENT_PERCEPTUAL);
	count = 0;
	while (text && text[count].text)
		count++;
	if (count > 0)
		png_set_text(png, info, text, count);
	png_write_info(png, info);
	export->row = malloc((size_t)export->fractal->width_orig * 6);
	if (!export->row)
		png_error(png, "16-bit PNG row allocation failed");
	y = -1;
	while (++y < export->fractal->height_orig)
	{
		pack_row(export, export->row, y);
		png_write_row(png, export->row);
	}
	png_write_end(png, info);
	free(export->row);
	export->row = NULL;
	return (0);
}

int	export_png16(const char *filename, t_fractal *fractal, png_text *text)
{
	t_export16	export;
	png_structp	png;
	png_infop	info;
	FILE		*file;

	memset(&export, 0, sizeof(export));
	export.fractal = fractal;
	if (init_export_filter(&export) != 0)
		return (free_metadata(text), -1);
	file = fopen(filename, "wb");
	if (!file)
		return (free(export.filter_rows), free_metadata(text), -1);
	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info = NULL;
	if (!png || setjmp(png_jmpbuf(png)))
		return (free(export.row), free(export.filter_rows), free_metadata(text),
			png_destroy_write_struct(&png, &info), fclose(file), -1);
	info = png_create_info_struct(png);
	if (!info)
		return (free(export.filter_rows), free_metadata(text),
			png_destroy_write_struct(&png, NULL), fclose(file), -1);
	export.fern_high = 0.0;
	if (fractal->id == 4 && fractal->fdensity)
		export.fern_high = high_hit_countf(fractal->width,
				fractal->height, fractal->fdensity);
	write_png_rows(png, info, file, &export, text);
	free(export.filter_rows);
	free_metadata(text);
	png_destroy_write_struct(&png, &info);
	if (fclose(file) != 0)
		return (-1);
	return (0);
}
