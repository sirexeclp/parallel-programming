all: hoi

hoi: hoi.cpp
	g++ -pthread -fopenmp $< -O3 -o $@ -lssl -lcrypto

clean:
	rm hoi


.PHONY: clean all 