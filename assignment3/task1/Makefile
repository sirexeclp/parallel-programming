all: parsum

parsum: parsum.cpp kernel.c
	g++ -std=c++0x parsum.cpp -O3 -o $@ -lOpenCL

validate: parsum
	python3 validator.py;exit=$$?; echo "Validation exited with code $$exit"; if [ $$exit -eq 0 ]; then echo "OK"; else echo "Validation failed!"; fi;

clean:
	rm -f parsum

test: parsum
	./parsum 1 100

.PHONY: validate clean all test