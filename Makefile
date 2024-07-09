all: image2maze


image2maze: main.c stb_image.h stb_image_write.h
	gcc -o image2maze main.c -g -lm
