#include "fractol.h"

void	print_mj_guide(void)
{
	ft_putchar_fd('\n', 1);
	ft_putstr_color_fd(1, "                  ╔╦═════════════╦╗\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "                  ╠╣  KEY GUIDE  ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╔╦════════════════╩╩═════════════╩╩════════════════╦╗\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "    This guide applies to Julia and Mandelbrot   ", BRIGHT_GREEN);
	ft_putstr_color_fd(1, "╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [F2]       => Print this guide                  ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [ESC]      => Exit and close window             ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [F3]       => Export image to png               ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ ----------------- MOUSE HOOKS ------------------╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [Wheel UP] => Zoom in                           ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [Wheel DN] => Zoom out                          ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [L CLICK]  => Center vertical axis              ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [R CLICK]  => Center horizontal axis            ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣-------------------------------------------------╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [R  CNTRL] => Mouse Zoom on/off (starts on)     ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ (Mouse Zoom zooms about mouse arrow position)   ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [UP ⬆]     => Move view upward                  ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [Down ⬇]   => Move view downward                ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [Left ⬅]   => Move view left                    ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [right ➡]  => Move view right                   ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [+]   => increase iterations level by 10        ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [-]   => decrease iterations level by 10        ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣", BRIGHT_BLUE);
	ft_putstr_color_fd(1, " [SPACE]    => Supersample on/off(antialiasing)  ", BRIGHT_GREEN);
	ft_putstr_color_fd(1, "╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [NUM 2]    => Pause color shift on current color╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [NUM 1]    => Switch to/from color wheel colors ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [A D,W S]  => Move center of color wheel in x, y╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [Q]        => recenter color wheel              ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [NPAD 0]   => return to starting position       ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [NPAD +]   => increase supersample level by 2   ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [NPAD -]   => decrease supersample level by 2   ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [ALT]      => toggle on off auto iter adjust    ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ -------- Section applies to Julia only -------- ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [L  CNTRL] => Mouse Julia on/off, Julia values  ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ change with mouse position.                     ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [NPAD 8]   => increase Julia y val by .005      ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [NPAD 2]   => decrease Julia y val by .005      ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [NPAD 6]   => increase Julia x val by .005      ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [NPAD 4]   => decrease Julia x val by .005      ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣-------------------------------------------------╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [TAB]  => Switch to/from second key layer       ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ ( some keys will now do something different)    ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣---------- Second layer key functions -----------╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [+]   => increase iterations level by 500       ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [-]   => decrease iterations level by 500       ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╚╩═════════════════════════════════════════════════╩╝\n", BRIGHT_BLUE);
}
//╔ ╗ ╚ ╝ ═ ║, ╦ ╩ ╠ ╣, ➡ ⬅ ⬆ ⬇

void	print_buddha_guide(void)
{
	ft_putchar_fd('\n', 1);
	ft_putstr_color_fd(1, "                  ╔╦═════════════╦╗\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "                  ╠╣  KEY GUIDE  ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╔╦════════════════╩╩═════════════╩╩════════════════╦╗\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "    This guide applies to the Buddhabrot         ", BRIGHT_GREEN);
	ft_putstr_color_fd(1, "╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [F2]     => Print this guide                    ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [ESC]    => Exit and close window               ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [F3]     => Export image to png                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [0]      => Print color settings info           ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [TAB]    => Toggle on/off 2nd layer functions   ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣---- Below control color without rerendering ----╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [Q, A]   => Increase/decrease blue exponent .05 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [W, S]   => Increase/decrease green exponent .05╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [E, D]   => Increase/decrease red exponent .05  ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ rt[STRG] => Averaging filter on/off             ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [U, J]   => Increase/decrease filter strength   ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣------------- Second layer funtions -------------╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [Q, A]   => Increase/decrease blue exponent .01 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [W, S]   => Increase/decrease green exponent .01╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣ [E, D]   => Increase/decrease red exponent .01  ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╠╣                                                 ╠╣\n", BRIGHT_BLUE);
	ft_putstr_color_fd(1, "╚╩═════════════════════════════════════════════════╩╝\n", BRIGHT_BLUE);
}