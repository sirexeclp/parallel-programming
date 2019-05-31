all: hoi

hoi: hoi.cpp
	g++ -pthread -fopenmp $< -O3 -o $@ -lssl -lcrypto

clean:
	rm hoi

test: hoi
	./hoi c0ffee 4 0 2

validate: hoi
	python3 validator.py;exit=$$?; if [ $$exit -eq 0 ]; then echo "\033[32mOK"; else echo "\033[31mValidation failed!"; fi; echo "Validation exited with code $$exit";

.PHONY: clean all test