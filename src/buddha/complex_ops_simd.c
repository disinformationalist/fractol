#include "fractol.h"

__m256d sum_complex_simd(__m256d z1_x, __m256d z1_y, __m256d z2_x, __m256d z2_y)
{
    __m256d res_x = _mm256_add_pd(z1_x, z2_x);
    __m256d res_y = _mm256_add_pd(z1_y, z2_y);
    // Combine results if needed
    // In this example, just return res_x as res_y is not needed
    return res_x; 
}

void square_complex_simd(__m256d z_x, __m256d z_y, __m256d* res_x, __m256d* res_y)
{
    __m256d z_x2 = _mm256_mul_pd(z_x, z_x);
    __m256d z_y2 = _mm256_mul_pd(z_y, z_y);
    __m256d two_z_x_z_y = _mm256_mul_pd(_mm256_set1_pd(2.0), _mm256_mul_pd(z_x, z_y));

    *res_x = _mm256_sub_pd(z_x2, z_y2);
    *res_y = two_z_x_z_y;
}

//-------------CONJUGATES----------
void square_complex_conj_simd(__m256d z_x, __m256d z_y, __m256d* res_x, __m256d* res_y)
{
    // Compute z.x^2
    __m256d z_x2 = _mm256_mul_pd(z_x, z_x);
    
    // Compute z.y^2
    __m256d z_y2 = _mm256_mul_pd(z_y, z_y);

    // Compute real part: z.x^2 - z.y^2
    *res_x = _mm256_sub_pd(z_x2, z_y2);

    // Compute imaginary part: -2 * z.x * z.y
    __m256d two = _mm256_set1_pd(2.0);
    __m256d neg_two = _mm256_sub_pd(_mm256_setzero_pd(), two);
    *res_y = _mm256_mul_pd(neg_two, _mm256_mul_pd(z_x, z_y));
}
