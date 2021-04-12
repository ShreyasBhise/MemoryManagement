all : multi

test: ../my_vm.h
	gcc test.c -L../ -lmy_vm -m32 -lm -o test

multi: ../my_vm.h
	gcc multi_test.c -L../ -lpthread -lmy_vm -m32 -lm -o test

clean:
	rm -rf test