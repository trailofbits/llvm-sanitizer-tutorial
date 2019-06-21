#include <stdlib.h> 
#include <string.h> 
#include <stdio.h> 

char * foo(int size) {
		return malloc(sizeof(char) * size); 
}

int main(int argc, char * argv[]) {

	if (argc < 2) {
		printf("Provide a size for malloc!\n Ex: ./malloc_target 10\n"); 
		return -1; 	
	}
	char * some_ptr = foo(atoi(argv[1]));
	return 0; 
}
