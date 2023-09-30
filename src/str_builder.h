
#define c_str_builder_size 1024

struct s_str_builder
{
	int tab_count;
	int len;
	char data[c_str_builder_size];
};
