#include <stdlib.h> 
#include <string.h> 

char * foo(int size) {
		return malloc(sizeof(char) * size); 
}

int main(char * argv[], int argc) {

	if (argc < 2) {
		printf(
			"
			Provide a size for malloc!\n
			Ex: ./malloc_target 10\n
			"); 	
	}
	char * some_ptr = foo(atoi(argv[1]));
	*some_ptr = 'A';  
	return 0; 
}
