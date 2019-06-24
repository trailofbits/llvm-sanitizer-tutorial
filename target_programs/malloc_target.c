#include <stdlib.h> 

//Just allocate a buffer of size 10
char * foo() {
		return malloc(sizeof(char) * 10); 
}

int main(int argc, char * argv[]) {

	char * some_ptr = foo();
	return 0; 
}
