#include <stdlib.h> 
#include <string.h> 
#include <stdio.h> 

char * foo(int size) {
		return malloc(sizeof(char) * size); 
}

int main(int argc, char * argv[]) {

	if (argc < 1) {
		printf("Provide a size for malloc!\n Ex: ./malloc_target 10\n"); 
		return -1; 	
	}
	char * some_ptr = foo(atoi(argv[1]));
	*some_ptr = 'A';  
	return 0; 
}
