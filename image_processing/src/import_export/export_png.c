#include "image_processing.h"

static uint8_t	to_gray(uint8_t red, uint8_t green, uint8_t blue)
{
	return ((uint8_t)((77 * red + 150 * green + 29 * blue + 128) >> 8));
}

static int	write_pixel(png_byte **row, t_pixel pixel, int color_type)
{
	if (color_type == PNG_COLOR_TYPE_RGB)
	{
		*(*row)++ = pixel.red;
		*(*row)++ = pixel.green;
		*(*row)++ = pixel.blue;
	}
	else if (color_type == PNG_COLOR_TYPE_RGBA)
	{
		*(*row)++ = pixel.red;
		*(*row)++ = pixel.green;
		*(*row)++ = pixel.blue;
		*(*row)++ = pixel.alpha;
	}
	else if (color_type == PNG_COLOR_TYPE_GRAY)
		*(*row)++ = to_gray(pixel.red, pixel.green, pixel.blue);
	else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		*(*row)++ = to_gray(pixel.red, pixel.green, pixel.blue);
		*(*row)++ = pixel.alpha;
	}
	else
		return (-1);
	return (0);
}

int	set_png_pixels(t_png_io *png_img, t_img *img, int height, int width)
{
	png_byte	*row;

	png_img->y = -1;
	while (++png_img->y < height)
	{
		row = png_malloc(png_img->png_ptr,
				(size_t)width * (size_t)png_img->pixel_size);
		if (!row)
		{
			perror("Error: export_png row malloc failure");
			clean_memory(png_img, png_img->y, true);
			return (-1);
		}
		png_img->row_pointers[png_img->y] = row;
		png_img->x = -1;
		while (++png_img->x < width)
		{
			get_pixel(&png_img->temp_pixel, img, png_img->x, png_img->y);
			if (write_pixel(&row, png_img->temp_pixel,
					png_img->color_type) != 0)
			{
				clean_memory(png_img, png_img->y + 1, true);
				return (-1);
			}
		}
	}
	return (0);
}

int	init_png_structs(t_png_io *png_img, const char *filename)
{
	png_img->fp = fopen(filename, "wb");
	if (!png_img->fp)
	{
		perror("Error: export_png could not open file");
		free(png_img);
		return (-1);
	}
	png_img->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
			NULL, NULL, NULL);
	if (!png_img->png_ptr)
		return (error_1(png_img,
				"Error: export_png could not create png write struct"));
	png_img->info = png_create_info_struct(png_img->png_ptr);
	if (!png_img->info)
	{
		png_destroy_write_struct(&png_img->png_ptr, NULL);
		return (error_1(png_img,
				"Error: export_png could not create png info struct"));
	}
	return (0);
}

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

static void	write_png(t_png_io *png_img)
{
	int	count;

	count = 0;
	while (png_img->text && png_img->text[count].text)
		count++;
	if (count > 0)
		png_set_text(png_img->png_ptr, png_img->info,
			png_img->text, count);
	png_init_io(png_img->png_ptr, png_img->fp);
	png_set_rows(png_img->png_ptr, png_img->info,
		png_img->row_pointers);
	png_write_png(png_img->png_ptr, png_img->info,
		PNG_TRANSFORM_IDENTITY, NULL);
	free_metadata(png_img->text);
	png_img->text = NULL;
}

int	export_png(const char *filename, t_img *img, int width, int height,
		png_text *text, t_pixel_format format)
{
	t_png_io	*png_img;

	png_img = malloc(sizeof(*png_img));
	if (!png_img)
		return (free_metadata(text), -1);
	init_vars(png_img, format);
	if (png_img->pixel_size < 0)
	{
		free_metadata(text);
		return (error_1(png_img, "Invalid PNG pixel format"));
	}
	if (init_png_structs(png_img, filename) == -1)
		return (free_metadata(text), -1);
	png_set_IHDR(png_img->png_ptr, png_img->info, width, height,
		png_img->depth, png_img->color_type, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_set_sRGB(png_img->png_ptr, png_img->info,
		PNG_sRGB_INTENT_PERCEPTUAL);
	png_img->row_pointers = png_malloc(png_img->png_ptr,
			(size_t)height * sizeof(*png_img->row_pointers));
	if (!png_img->row_pointers)
	{
		png_destroy_write_struct(&png_img->png_ptr, &png_img->info);
		free_metadata(text);
		return (error_1(png_img, "Error: PNG row pointer allocation"));
	}
	if (set_png_pixels(png_img, img, height, width) == -1)
		return (free_metadata(text), -1);
	png_img->text = text;
	write_png(png_img);
	clean_memory(png_img, height, true);
	return (0);
}
