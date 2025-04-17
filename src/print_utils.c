#include "fractol.h"

size_t	ft_strlen_f(const char *s)
{
	size_t	i;

	if (s == NULL)
		return (0);
	i = 0;
	while (s[i])
		i++;
	return (i);
}

int	ft_putchar_fd(char c, int fd)
{
	int	len;

	len = 0;
	len += write(fd, &c, 1);
	return (len);
}

int	ft_putstr_color_fd(int fd, char *s, char *color)
{
	int			len;
	const char	*reset_col;

	len = 0;
	reset_col = "\033[0m";
	write(fd, color, ft_strlen(color));
	while (*s)
		len += write(fd, s++, 1);
	write(fd, reset_col, ft_strlen(reset_col));
	return (len);
}
