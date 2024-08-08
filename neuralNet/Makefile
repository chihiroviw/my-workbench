all: skel perceptron bp auto_encoder mnist_test

skel: skel.c SFMT-src-1.5.1/SFMT.c
	gcc -O3 -std=gnu11 -Wall -I SFMT-src-1.5.1 -D SFMT_MEXP=19937 -o $@ $^ -lm

perceptron: perceptron.c SFMT-src-1.5.1/SFMT.c
	gcc -O3 -std=gnu11 -Wall -I SFMT-src-1.5.1 -D SFMT_MEXP=19937 -o $@ $^ -lm

bp: bp.c SFMT-src-1.5.1/SFMT.c mnist.c
	gcc -O3 -std=gnu11 -Wall -D SFMT_MEXP=19937 \
	-I SFMT-src-1.5.1 -I ./libgd/include -L ./libgd/lib -o $@ $^ -lm -lgd

auto_encoder: auto_encoder.c SFMT-src-1.5.1/SFMT.c mnist.c
	gcc -O3 -std=gnu11 -Wall -D SFMT_MEXP=19937 \
	-I SFMT-src-1.5.1 -I ./libgd/include -L ./libgd/lib -o $@ $^ -lm -lgd

mnist_test: mnist_test.c mnist.c 
	gcc -O3 -std=gnu11 -Wall -Wunused-result -I ./libgd/include -L ./libgd/lib -o $@ $^ -lgd

clean:
	rm -f *.o skel perceptron bp mnist_test
