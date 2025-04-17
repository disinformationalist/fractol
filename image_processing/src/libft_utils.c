#include "image_processing.h"

size_t	ft_strlen(const char *s)
{
	size_t	i;

	if (s == NULL)
		return (0);
	i = 0;
	while (s[i])
		i++;
	return (i);
}

char	*ft_strdup(const char *s)
{
	size_t	i;
	char	*dest;
	char	*src;

	src = (char *)s;
	i = 0;
	dest = (char *)malloc((ft_strlen(src) + 1) * sizeof(char));
	if (dest == NULL)
		return (NULL);
	while (src[i])
	{
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
	return (dest);
}

char	*ft_strjoin_n(int count, ...)
{
	va_list args;
	int		i;
	int		j;
	size_t	total_length;
	char	*result;
	const char *s;

	va_start(args, count);
	total_length = 0;
	i = -1;
	while (++i < count)
	{
		s = va_arg(args, const char *);
		total_length += ft_strlen(s);
	}
	va_end(args);
	result = (char *)malloc((total_length + 1) * sizeof (char));
	if (result == NULL)
		return (NULL);
	va_start(args, count);
	j = 0;
	i = -1;
	while (++i < count)
	{
		s = va_arg(args, const char *);
		while (*s)
			result[j++] = *s++;
	}
	va_end(args);

	result[j] = '\0';
	return (result);
}


static int	spaces(const char *str)
{
	int	i;

	i = 0;
	while (str[i] == 32 || (str[i] >= 9 && str[i] <= 13))
		i++;
	return (i);
}

int	ft_atoi(const char *nptr)
{
	int	i;
	int	sign;
	int	result;

	result = 0;
	sign = 1;
	i = spaces(nptr);
	if (nptr[i] == 45 || nptr[i] == 43)
	{
		sign = 44 - nptr[i];
		i++;
	}
	while (nptr[i] >= '0' && nptr[i] <= '9')
	{
		result *= 10;
		result += nptr[i] - '0';
		i++;
	}
	result *= sign;
	return (result);
}

void	*ft_memset(void *s, int c, size_t n)
{
	char	*s1;
	size_t	i;

	i = 0;
	s1 = (char *)s;
	while (i < n)
	{
		*s1 = (char)c;
		i++;
		s1++;
	}
	return ((char *)s);
}

double	ft_atof(char *s)
{
	int		sign;
	long	int_piece;
	double	float_piece;
	int		div;

	div = 1;
	sign = 1;
	int_piece = 0;
	float_piece = 0;
	while (*s == 32 || (*s >= 9 && *s <= 13))
		s++;
	while (*s == 45 || *s == 43)
		if (*s++ == '-')
			sign *= -1;
	while ((*s >= '0' && *s <= '9') && *s && *s != '.')
		int_piece = int_piece * 10 + (*s++ - '0');
	if (*s == '.')
		s++;
	while (*s && (*s >= '0' && *s <= '9'))
	{
		float_piece = float_piece * 10 + (*s++ - '0');
		div *= 10;
	}
	return (sign * (int_piece + float_piece / div));
}

void	putstr_fd(char *s, int fd)
{
	while (*s)
		write(fd, s++, 1);
}

void	ft_putstr_color(char *s, char *color)
{
	char *reset_color;

	reset_color = "\033[0m";
	(void)write(1, color, ft_strlen(color));
	while (*s)
		(void)write(1, s++, 1);
	(void)write(1, reset_color, ft_strlen(reset_color));
}