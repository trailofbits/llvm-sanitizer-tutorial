
extern "C" {
	void simpsan_AllocateShadowMemory(int max_elements);
	int simpsan_StoreInteger(int value, int index); 
	int simpsan_FetchInteger(int index);
	int simpsan_PutsWrapper(char * some_string); //TODO temp
	void simpsan_Callback(void);
} //Extern C, provide linkage with C programs that will use our interface. 
