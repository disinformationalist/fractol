NAME := libimage_processing.a

SRC_DIR = ./src
OBJ_DIR = ./obj
INC_DIR = ./inc

SRCS := ./src/mean_convo_filter.c ./src/gaussian_convo_filter.c ./src/my_pixel_puts.c ./src/gamma_correction.c \
./src/import_export/export_png.c ./src/import_export/export_png_utils.c ./src/sample.c ./src/create_img.c ./src/adjust_pixels_rgb.c \
./src/import_export/import_png.c ./src/import_export/import_png_utils.c ./src/padding/pad_borders.c ./src/sobel.c ./src/convolution_utils.c \
./src/padding/pad_mirror.c ./src/printing_utils.c ./src/padding/pad_mirror_kernel.c ./src/matrix_utils.c \
./src/set_color_wheel.c ./src/get_next_name.c ./src/libft_utils.c ./src/interpolate_two_colors.c \
./src/color_source_wheel.c ./src/threading_tools.c ./src/nlm/non_local_means.c ./src/padding/pad_mirror_circle.c \
./src/padding/pad_mirror_pix.c \
./src/padding/pad_mirror_mat.c \
./src/nlm_rouselle/nlm_rouselle.c \
./src/nlm/nlm_utils.c \
./src/re_color_buddha.c \



#SRCS = $(wildcard $(SRC_DIR)/*.c)

CFLAGS := -Wall -Wextra -Werror -I$(INC_DIR)

CC := cc

OBS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

#OBS := $(SRCS:.c=.o)

COLOR_RESET = \033[0m
COLOR_GREEN = \033[1;92m
COLOR_BLUE = \033[1;96m 

MAKEFLAGS += --no-print-directory

define print_colored
	@echo "$(COLOR_GREEN)$(1)$(COLOR_BLUE)$(2)$(COLOR_RESET)$(3)"
endef

.SILENT:

all: $(NAME)

$(NAME): $(OBS)
	ar rcs $(NAME) $(OBS)
	$(call print_colored, "[SUCCESS]", "archive $(NAME)", "Ready")

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBS)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re