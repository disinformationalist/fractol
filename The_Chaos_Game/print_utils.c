/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:14:55 by jhotchki          #+#    #+#             */
/*   Updated: 2024/02/20 10:14:58 by jhotchki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "chaos.h"

void	ft_putstr_color(char *s, char *color)
{
	write(1, color, 7);
	while (*s)
		write(1, s++, 1);
	write(1, "\033[0m", 5);
}

static void	put_line(int first_rule, int next_rule, char *s1, char *s2)
{
	ft_putstr(s1);
	put_bool(first_rule);
	ft_putstr(s2);
	put_bool(next_rule);
	ft_putstr("      ║\n");
}

static void	print_title(void)
{
	ft_putstr("╔══════════════════════════════════════════╗\n");
	ft_putstr("║     ");
	ft_putstr_color(" ✰ ✰ ✰ ", "\033[29m");
	ft_putstr_color(" THE CHAOS GAME", "\033[1;34m");
	ft_putstr_color(" ✰ ✰ ✰ ", "\033[29m");
	ft_putstr("      ║\n");
	ft_putstr("╚══════════════════════════════════════════╝\n");
}

void	print_board(t_game *r)
{
	print_title();
	ft_putstr("           ╔════════════════════╗           \n");
	ft_putstr("╔══════════╣ RULES AND SETTINGS ╠══════════╗\n");
	ft_putstr("║          ╚════════════════════╝          ║\n");
	put_line(r->rule_1, r->rule_16, "║      RULE_1: ", "       RULE_16: ");
	put_line(r->rule_2, r->rule_17, "║      RULE_2: ", "       RULE_17: ");
	put_line(r->rule_3, r->rule_18, "║      RULE_3: ", "       RULE_18: ");
	put_line(r->rule_4, r->rule_19, "║      RULE_4: ", "       RULE_19: ");
	put_line(r->rule_5, r->rule_20, "║      RULE_5: ", "       RULE_20: ");
	put_line(r->rule_6, r->rule_21, "║      RULE_6: ", "       RULE_21: ");
	put_line(r->rule_7, r->rule_22, "║      RULE_7: ", "       RULE_22: ");
	put_line(r->rule_8, r->rule_23, "║      RULE_8: ", "       RULE_23: ");
	put_line(r->rule_9, r->rule_24, "║      RULE_9: ", "       RULE_24: ");
	put_line(r->rule_10, r->rule_25, "║     RULE_10: ", "       RULE_25: ");
	put_line(r->rule_11, r->rule_26, "║     RULE_11: ", "       RULE_26: ");
	put_line(r->rule_12, r->rule_27, "║     RULE_12: ", "       RULE_27: ");
	put_line(r->rule_13, r->rule_28, "║     RULE_13: ", "       RULE_28: ");
	put_line(r->rule_14, r->disinfo_1, "║     RULE_14: ", "     DISINFO_1: ");
	put_line(r->rule_15, r->disinfo_2, "║     RULE_15: ", "     DISINFO_2: ");
	put_ratio(r);
	ft_putstr("║           BROUGHT TO YOU BY:             ║\n");
	ft_putstr("║                ");
	ft_putstr_color("JHOTCHKI", "\033[1;34m");
	ft_putstr("                  ║\n");
	ft_putstr("╚══════════════════════════════════════════╝\n");
}
