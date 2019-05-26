all: decrypt

decrypt: decrypt.cpp
	g++ $< -lcrypt -fopenmp -O3 -o $@

validate: decrypt
	python3 validator.py;exit=$$?; echo "Validation exited with code $$exit"; if [ $$exit -eq 0 ]; then echo "OK"; else echo "Validation failed!"; fi;

clean:
	rm decrypt
