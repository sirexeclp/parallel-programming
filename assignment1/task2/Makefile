all: dinner

dinner: dinner.c
	cc -pthread dinner.c -o $@

validate: dinner
	python3 validator.py;exit=$$?; echo "Validation exited with code $$exit"; if [ $$exit -eq 0 ]; then echo "OK"; else echo "Validation failed!"; fi;

clean:
	rm dinner

.PHONY: validate clean all