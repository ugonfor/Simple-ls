#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/time.h>



extern char *optarg;

typedef struct FILE_INFO{
	struct dirent *dirp;
	struct stat *f_stat;
} file_info;

int compare(file_info* f_a, file_info* f_b){
	if(strcasecmp(f_a->dirp->d_name, f_b->dirp->d_name) < 0) return -1;
	if(strcasecmp(f_a->dirp->d_name, f_b->dirp->d_name) > 0) return 1;
	else return 0;
}

int nooption(char* file_name){
	DIR *dp;
	struct dirent *dirp;
	
	if ((dp = opendir(file_name)) == NULL)
		printf("can't open %s", file_name);
	
	while ((dirp = readdir(dp)) != NULL)
		printf("%s\n", dirp->d_name);


	closedir(dp);
	
	return 0;
};


void print_file_info(file_info* f_info){
	struct stat* t_stat = f_info->f_stat;

	mode_t t_mode = t_stat->st_mode;
	
	if ((t_mode & S_IFMT) == S_IFLNK) printf("l");
	if ((t_mode & S_IFMT) == S_IFREG) printf("-");
	if ((t_mode & S_IFMT) == S_IFBLK) printf("b");
	if ((t_mode & S_IFMT) == S_IFDIR) printf("d");
	if ((t_mode & S_IFMT) == S_IFCHR) printf("c");


	for (int i = 0; i < 3; i++)
	{
		if(t_mode & S_IRUSR) printf("r");
		else printf("-");
		if(t_mode & S_IWUSR) printf("w");
		else printf("-");
		if(t_mode & S_IXUSR) printf("x");
		else printf("-");
		t_mode <<=3;
	}

	printf(" %ld\t", t_stat->st_nlink);
	
	printf(" %-10s", getpwuid(t_stat->st_uid)->pw_name);
	printf(" %-10s", getgrgid(t_stat->st_gid)->gr_name);
		
	printf("%11ld", t_stat->st_size);

	struct tm* t_time;
	t_time = gmtime(&t_stat->st_atim.tv_sec);

	char s[100];
	if (121 == t_time->tm_year) strftime(s, sizeof(s), "%b %d %R",t_time);
	else strftime(s, sizeof(s), "%b %d %Y",t_time);
	printf(" %s", s);
	
	printf(" %s", f_info->dirp->d_name);
	printf("\n");
	return ;
}

int loption(char* file_name){
	DIR				*dp;
	struct dirent	*dirp;
	int dir_cnt=0;
	int total_block = 0;

	if ((dp = opendir(file_name)) == NULL)
		printf("can't open %s", file_name);

	while ((dirp = readdir(dp)) != NULL)
		dir_cnt++;
	closedir(dp);

	file_info* f_info;
	f_info = malloc(sizeof(file_info) * dir_cnt);
	
	struct stat* list_stat;
	list_stat = malloc(sizeof(struct stat) * dir_cnt);


	dp = opendir(file_name);
	int t_cnt = 0; //tmp_cnt

	while ((dirp = readdir(dp)) != NULL)
	{	
		char t_path[0x100] = {0};
		strcat(t_path, file_name);
		strcat(t_path, "/");
		strcat(t_path, dirp->d_name);
		lstat(t_path, (list_stat+t_cnt));
		((f_info + t_cnt)->f_stat) = list_stat + t_cnt;
		((f_info + t_cnt)->dirp) = dirp;		
		if (*(char*)((f_info + t_cnt)->dirp->d_name) != '.')	total_block += ((f_info + t_cnt)->f_stat)->st_blocks;
		t_cnt++;
	}

	printf("total %d\n", total_block/2);

	qsort(f_info, t_cnt, sizeof(file_info), (__compar_fn_t)compare);

	for (size_t i = 0; i < t_cnt; i++)	
	{
		file_info* tmp =(f_info+i);
		if (*(char*)(tmp->dirp->d_name) == '.') continue;
		print_file_info(tmp);
	}
	
	return 0;

};

void usage(char* name){
	printf("\nusage: %s [OPTION]... [FILE]\n", name);
	fputs("List information about the FILEs (the current directory by defualt)\n\
Sort entries alphabetically.\n\n\
For CYDF Subject 37 By 2019330023 Hyogon Ryu\n\
\n\
Options:\n\
\t-h\t show this help and exit\n\
\t-l\t use a long listing format\n", stdout);
}

typedef struct FLAG{
	char hflag;
	char lflag;
} Flag;
int main(int argc, char *argv[])
{
	int opt;
	Flag flag = {0,0};

	while( (opt = getopt(argc, argv, "hl")) != -1)
	{
		switch (opt){
			case 'h':
				flag.hflag = 1;
				break;

			case 'l':
				flag.lflag = 1;
				break;
			
			case '?':
				usage(argv[0]);
				exit(0);
		}

	}
	
	if (flag.hflag) {
		usage(argv[0]); 
		exit(0);
	}

	if (flag.lflag){
		if (argv[argc-1][0] == '-') loption(".");
		else loption(argv[argc-1]);
		exit(0);
	}

	if (argc == 1) nooption(".");
	else nooption(argv[argc-1]);
	exit(0);
}
