#include "image_processing.h"

int	error_1(t_png_io *png_img, const char *message)
{
	if (message)
		perror(message);
	if (png_img->fp)
		fclose(png_img->fp);
	free(png_img);
	return (-1);
}

static int	pixel_size(t_png_io *png_img, t_pixel_format format)
{
	if (format == PALETTE)
	{
		png_img->color_type = PNG_COLOR_TYPE_PALETTE;
		return (-1);
	}
	if (format == GRAY)
	{
		png_img->color_type = PNG_COLOR_TYPE_GRAY;
		return (1);
	}
	if (format == GRAY_A)
	{
		png_img->color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
		return (2);
	}
	if (format == RGB)
	{
		png_img->color_type = PNG_COLOR_TYPE_RGB;
		return (3);
	}
	if (format == RGBA)
	{
		png_img->color_type = PNG_COLOR_TYPE_RGBA;
		return (4);
	}
	return (-1);
}

void	init_vars(t_png_io *png_img, t_pixel_format format)
{
	png_img->y = 0;
	png_img->x = 0;
	png_img->pixel_size = pixel_size(png_img, format);
	png_img->depth = 8;
	png_img->png_ptr = NULL;
	png_img->info = NULL;
	png_img->row_pointers = NULL;
	png_img->fp = NULL;
	png_img->text = NULL;
}

void	free_png_rows(png_structp png_ptr, png_byte **row_pointers, int count)
{
	int	row;

	if (!row_pointers)
		return ;
	row = -1;
	while (++row < count)
		png_free(png_ptr, row_pointers[row]);
	png_free(png_ptr, row_pointers);
}

void	clean_memory(t_png_io *png_img, int rows, bool export)
{
	free_png_rows(png_img->png_ptr, png_img->row_pointers, rows);
	if (export)
		png_destroy_write_struct(&png_img->png_ptr, &png_img->info);
	else
		png_destroy_read_struct(&png_img->png_ptr, &png_img->info, NULL);
	if (png_img->fp)
		fclose(png_img->fp);
	free(png_img);
}

void	get_pixel(t_pixel *pixel, t_img *img, int x, int y)
{
	unsigned int	value;
	int				offset;

	offset = y * img->line_len + x * (img->bpp / 8);
	value = *(unsigned int *)(img->pixels_ptr + offset);
	pixel->alpha = (value >> 24) & 0xFF;
	pixel->red = (value >> 16) & 0xFF;
	pixel->green = (value >> 8) & 0xFF;
	pixel->blue = value & 0xFF;
}
