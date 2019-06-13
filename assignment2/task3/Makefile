all: hoi

hoi: hoi.cpp
	g++-9 -std=c++17 -pthread -fopenmp $< -O3 -o $@ -lssl -lcrypto -ltbb

clean:
	rm hoi

test: hoi
	./hoi c0ffee 4 0 2

validate: hoi
	python3 validator.py;exit=$$?; if [ $$exit -eq 0 ]; then echo "\033[32mOK"; else echo "\033[31mValidation failed!"; fi; echo "Validation exited with code $$exit";

test2: hoi
	./hoi deadc0deba5e 65536 0 1 24319 24320

.PHONY: clean all test test2