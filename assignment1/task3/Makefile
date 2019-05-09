all: heatmap

heatmap: heatmap.cpp
	g++ -pthread $< -o $@

validate: heatmap
	python3 validator.py;exit=$$?; echo "Validation exited with code $$exit"; if [ $$exit -eq 0 ]; then echo "OK"; else echo "Validation failed!"; fi;

clean:
	rm heatmap

test: heatmap
	./heatmap 20 7 17 hotspots.csv

test2: heatmap
	./heatmap 20 7 17 hotspots.csv coords.txt

.PHONY: validate clean all test test2