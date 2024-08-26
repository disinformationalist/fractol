#include "chaos.h"

double gaussian_value(int x, int y, double sigma)
{
	return (exp(-((x * x + y * y)) / (2 * (sigma * sigma))) / 2 * M_PI * pow(sigma * sigma));
}

void	set_kernal(int size, double sigma, )
{
	int i 
}