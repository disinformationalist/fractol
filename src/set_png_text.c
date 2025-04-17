#include "fractol.h"

char	*safe_ft_strdup(const char *s)
{
	char *dup;

	dup = ft_strdup(s);
	if (!dup)
	{
		perror("ft_strdup malloc failure.");
		exit(EXIT_FAILURE);
	}
	return (dup);
}

png_text *build_fractal_text(t_fractal *fractal)
{
	png_text 	*text;
	int			i;
	
	text = (png_text *)malloc(6 * sizeof(png_text));
	if (!text)
	{
		perror("png text malloc failure");
		close_handler(fractal);
	}
	i = -1;
	while (++i < 3)
		text[i].compression = PNG_TEXT_COMPRESSION_NONE;
	/* location coordinates( find center, just center compute + movex + move_y!!), center is 0 to start in complex coordinates
	 zoom level,
	  current color if app,
	   supersample level, 
	   name: mandelbrot  julia  bhudda fern*/
	
	text[0].key = safe_ft_strdup("");
	text[0].text = safe_ft_strdup("");
	text[1].key = safe_ft_strdup("ZOOM");
	text[1].text = safe_ft_strdup("");
	return (text);
}


