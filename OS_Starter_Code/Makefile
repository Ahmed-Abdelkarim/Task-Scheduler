build:
	gcc process_generator.c -o process_generator.out
	gcc clk.c -o clk.out
	gcc scheduler.c -o scheduler.out
	gcc process.c -o process.out
	gcc processestest.c -o processestest.out
clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./processestest.out
