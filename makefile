default:
	g++ main.cc -o main.out
	./main.out > image.ppm
	code image.ppm

