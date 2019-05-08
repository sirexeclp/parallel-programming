all: parsum

parsum: parsum.c
	cc -pthread parsum.c -o $@

validate: parsum
	python3 validator.py;exit=$$?; echo "Validation exited with code $$exit"; if [ $$exit -eq 0 ]; then echo "OK"; else echo "Validation failed!"; fi;

clean:
	rm -f parsum

.PHONY: validate clean all