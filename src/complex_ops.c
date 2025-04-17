#include "fractol.h"


t_complex	sum_complex(t_complex z1, t_complex z2)
{
	t_complex	res;

	res.x = z1.x + z2.x;
	res.y = z1.y + z2.y;
	return (res);
}

t_complex	square_complex(t_complex z)
{
	t_complex	res;


	res.x = SQ(z.x) - SQ(z.y);
	res.y = 2 * z.x * z.y;

	return (res);
}

t_complex	celtic(t_complex z)
{
	t_complex	res;

	res.x = SQ(z.x) - SQ(z.y);
	res.y = 2.0 * fabs(z.x) * z.y;

	return (res);
}

t_complex	ship(t_complex z)
{
	t_complex	res;

	res.x = SQ(z.x) - SQ(z.y);
	res.y = -fabs(2 * z.x * z.y);

	return (res);
}

t_complex	cube_complex(t_complex z)
{
	t_complex	res;
	double 		zx2;
	double		zy2;

	zx2 = SQ(z.x);
	zy2 = SQ(z.y);
	res.x = z.x * (zx2 - 3 * zy2);
	res.y = z.y * (3 * zx2 - zy2);
	return (res);
}

t_complex	cube_ship(t_complex z)
{
	t_complex	res;
	double 		zx2;
	double		zy2;

	zx2 = SQ(z.x);
	zy2 = SQ(z.y);
	res.x = fabs(z.x) * (zx2 - 3 * zy2);
	res.y = fabs(z.y) * (3 * zx2 - zy2);
	return (res);
}

t_complex	pow4_complex(t_complex z)
{
	t_complex	res;
	double 		zx2;
	double		zy2;

	zx2 = SQ(z.x);
	zy2 = SQ(z.y);
	res.x = SQ(zx2) - 6 * zx2 * zy2  + SQ(zy2);
	res.y = 4 * z.x * z.y * (zx2 - zy2);
	return (res);
}

t_complex	pow5_complex(t_complex z)//test all here down.
{
	t_complex	res;
	double 		zx2;
	double		zy2;

	zx2 = SQ(z.x);
	zy2 = SQ(z.y);
	res.x = z.x * (SQ(zx2) + 5 * zy2 * (zy2 - 2 * zx2));
	res.y = z.y * (SQ(zy2) + 5 * zx2 * (zx2 - 2 * zy2));
	return (res);
}

t_complex	pow6_complex(t_complex z)
{
	t_complex	res;
	double 		zx2;
	double		zy2;
	double		zxzy;

	zx2 = SQ(z.x);
	zy2 = SQ(z.y);
	zxzy = z.x * z.y;

	res.x = (CUBE(zx2) - CUBE(zy2) +  15.0 * zx2 * zy2 * (zy2 - zx2));
	res.y = -(6.0 * zxzy * (SQ(zx2) + SQ(zy2)) - 20.0 * CUBE(zxzy));
	return (res);
}

//------------------ COMPLEX CONJUGATE POWERS---------

t_complex	square_complex_conj(t_complex z)
{
	t_complex	res;
	
	res.x = SQ(z.x) - SQ(z.y);
	res.y = -2.0 * z.x * z.y;
	return (res);
}

t_complex	cube_complex_conj(t_complex z)
{
	t_complex	res;
	double 		zx2;
	double		zy2;

	zx2 = SQ(z.x);
	zy2 = SQ(z.y);

	res.x = z.x * (zx2 - 3 * zy2);
	res.y = z.y * (zy2 - 3 * zx2);
	
	return (res);
}

t_complex	pow4_complex_conj(t_complex z)
{
	t_complex	res;
	double 		zx2;
	double		zy2;

	zx2 = SQ(z.x);
	zy2 = SQ(z.y);

	res.x = SQ(zx2) +  SQ(zy2) - 6.0 * zx2 * zy2;
	res.y = 4.0 * z.x * z.y * (zy2 - zx2); 

	return (res);
}

t_complex	pow5_complex_conj(t_complex z)
{
	t_complex	res;
	double 		zx2;
	double		zy2;
	double 		zx3;
	double		zy3;

	zx2 = SQ(z.x);
	zy2 = SQ(z.y);
	zx3 = CUBE(z.x);
	zy3 = CUBE(z.y);

	res.x = (zx2 * zx3 - 10.0 * zx3 * zy2 +  5.0 * z.x * SQ(zy2));
	res.y = -(zy2 * zy3 - 10.0 * zy3 * zx2 +  5.0 * z.y * SQ(zx2));
	return (res);
}

t_complex	pow6_complex_conj(t_complex z)
{
	t_complex	res;
	double 		zx2;
	double		zy2;
	double		r12;
	double		zxzy;

	zx2 = SQ(z.x);
	zy2 = SQ(z.y);
	zxzy = z.x * z.y;

	res.x = (CUBE(zx2) - CUBE(zy2) +  15.0 * zx2 * zy2 * (zy2 - zx2));
	res.y = -(6.0 * zxzy * (SQ(zx2) + SQ(zy2)) - 20.0 * CUBE(zxzy));
	return (res);
}

//-----------INVERSE------

t_complex	inv_square_complex(t_complex z)
{
	t_complex	res;
	double 		zx2;
	double		zy2;
	double		r2;

	zx2 = SQ(z.x);
	zy2 = SQ(z.y);
	r2 = zx2 + zy2;

	res.x = (zx2 - zy2) / r2;
	res.y = -((2.0 * z.x * z.y) / r2);
	return (res);
}