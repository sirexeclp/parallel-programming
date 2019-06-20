all: heatmap

heatmap: heatmap.cpp
	g++ -std=c++11 $< -O3 -o $@

validate: heatmap
	python3 validator.py;exit=$$?; if [ $$exit -eq 0 ]; then echo "\033[32mOK"; else echo "\033[31mValidation failed!"; fi; echo "Validation exited with code $$exit";

clean:
	rm heatmap

test: heatmap
	./heatmap 20 7 17 hotspots.csv

test2: heatmap
	./heatmap 20 7 1000000 hotspots.csv coords.txt

.PHONY: validate clean all test test2
