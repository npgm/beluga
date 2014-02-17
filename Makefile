
all: beluga

clean:
	rm beluga

beluga:
	gcc -o beluga beluga.c cont.c `pkg-config --libs --cflags wayland-server wayland-client swc`
