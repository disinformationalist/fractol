# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jhotchki <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/02/20 10:07:54 by jhotchki          #+#    #+#              #
#    Updated: 2024/02/21 05:19:13 by jhotchki         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME := fractol
TEST_DIR := ./tests
BUDDHA_TEST := $(TEST_DIR)/buddha_regression
BUDDHA_TEST_DEP := $(BUDDHA_TEST).d

SRC_DIR = ./src
OBJ_DIR = ./obj
INC_DIR = ./inc

#SRCS := \
$(SRC_DIR)/main.c \
$(SRC_DIR)/events.c \
$(SRC_DIR)/render.c \
$(SRC_DIR)/utils.c \
$(SRC_DIR)/math_utils.c \
$(SRC_DIR)/init.c \
$(SRC_DIR)/colors.c \
$(SRC_DIR)/fractal_threads.c \
$(SRC_DIR)/mandelbrot.c \
$(SRC_DIR)/julia.c \
$(SRC_DIR)/thread_utils.c \
$(SRC_DIR)/print_utils.c \
$(SRC_DIR)/barnsley_fern.c \
$(SRC_DIR)/fern_utils.c \
$(SRC_DIR)/set_png_text.c \
$(SRC_DIR)/clean_up.c \
$(SRC_DIR)/complex_ops.c \
$(SRC_DIR)/print_guides.c \
$(SRC_DIR)/buddha/buddha_events.c \
$(SRC_DIR)/buddha/buddhabrot.c \
$(SRC_DIR)/buddha/density_utils.c \
$(SRC_DIR)/buddha/fast_buddha.c \
$(SRC_DIR)/buddha/map_importance.c \
$(SRC_DIR)/buddha/render_buddha.c \
$(SRC_DIR)/buddha/set_buddha_colors.c \
$(SRC_DIR)/buddha/print_buddha_vals.c


  
SRCS = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*/*.c) 
#$(wildcard $(SRC_DIR)/*/*/*.c) $(wildcard $(SRC_DIR)/*/*/*/*.c)

#CFLAGS := -Wall -Wextra -Werror -I$(INC_DIR) -O2 -march=native -w
CFLAGS := -Wall -Wextra -Werror -I$(INC_DIR) -g -O2 -mavx -mavx2 -march=native -Wno-unused-parameter -Wunused-result 
CFLAGS += -Wno-unused-result -Wno-unused-variable -Wno-unused-function -MMD -MP
CC := cc



#OBS := $(SRCS:.c=.o)
OBS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
CORE_OBS = $(filter-out $(OBJ_DIR)/main.o,$(OBS))
DEPS = $(OBS:.o=.d)


IMG_PATH = image_processing
IMG_ARCH = $(IMG_PATH)/libimage_processing.a

COLOR_RESET = \033[0m
COLOR_GREEN = \033[1;92m
COLOR_BLUE = \033[1;96m 

MAKEFLAGS += --no-print-directory

define print_colored
	@echo "$(COLOR_GREEN)$(1)$(COLOR_BLUE)$(2)$(COLOR_RESET)$(3)"
endef

.SILENT:

all: $(IMG_ARCH) $(NAME)

$(NAME): $(OBS)
	$(CC) $(OBS)  -L$(IMG_PATH) -limage_processing -lm -Lminilibx-linux -lmlx_Linux -lX11 -lXext -o $@ -lpng
	$(call print_colored, "[SUCCESS]", "./$(NAME)", "Ready")

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(IMG_ARCH):
	$(MAKE) -C $(IMG_PATH)

$(BUDDHA_TEST): $(CORE_OBS) $(TEST_DIR)/buddha_regression.c $(IMG_ARCH)
	$(CC) $(CFLAGS) $(TEST_DIR)/buddha_regression.c $(CORE_OBS) \
		-L$(IMG_PATH) -limage_processing -lm -Lminilibx-linux \
		-lmlx_Linux -lX11 -lXext -lpng -o $@

test-buddha: $(BUDDHA_TEST)
	$(BUDDHA_TEST)

reference-buddha: $(BUDDHA_TEST)
	$(BUDDHA_TEST) --print-reference

benchmark-buddha: $(BUDDHA_TEST)
	$(BUDDHA_TEST) --benchmark

benchmark-buddha-nlm: $(BUDDHA_TEST)
	$(BUDDHA_TEST) --benchmark-nlm

benchmark-buddha-importance-zoom: $(BUDDHA_TEST)
	$(BUDDHA_TEST) --benchmark-importance-zoom

clean:
	rm -rf $(OBS) $(DEPS) $(BUDDHA_TEST_DEP)
	cd image_processing && make clean

fclean: clean
	rm -rf $(NAME) $(BUDDHA_TEST)
	cd image_processing && make fclean
	
re: fclean all

.PHONY: all clean fclean re chaos test-buddha reference-buddha \
	benchmark-buddha benchmark-buddha-nlm benchmark-buddha-importance-zoom

-include $(DEPS) $(BUDDHA_TEST_DEP)
