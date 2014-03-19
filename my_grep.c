#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<dirent.h>
#include<limits.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<time.h>
#include<dirent.h>


int* prefix(char* s) {
    int* pref = malloc(strlen(s) * sizeof(int));
    int i=1, k=0;
    int imax = strlen(s);
    for (;i<imax; ++i)
    {
        while (k > 0 && s[k] != s[i])
            k = pref[k-1];
        if (s[k] == s[i])
            ++k;
        pref[i] = k;
	}
    return pref;
 }

void walk(char* substr, char* path);

void search_in_file(char* substr, char* path) {
	FILE* cur_file = fopen(path, "r");
	if (cur_file == NULL) {
		perror("Ошибка открытия на чтение");
		exit(1);
	}
	char* buf = malloc(100);
	if (buf == NULL) {
		perror(buf);
		fclose(cur_file);
		return;
	}
	char c;
	int i = 0;
	while (!feof(cur_file)) {
		c = fgetc(cur_file);
		if (c != '\n') {
			buf[i] = c;
			i++;
		} else {
			buf[i] = '\0';
			i = 0;
			char *new_str = malloc(strlen(buf) + strlen(substr) + 2);
			strcpy(new_str, substr);
			strcat(new_str, "$");
			strcat(new_str, buf);
			int *pref = prefix(new_str);
			int k;
			for(k = 0; k < strlen(new_str); k++) 
				if (pref[k] == strlen(substr)) {
					printf("%s\n", buf);
					break;
				}
			free(new_str);
			free(pref);
		}
	}
}

void my_grep(char* substr, char* path) {
	struct stat stbuf;
	if (stat(path, &stbuf) == -1) 
		fprintf(stderr, "can't access %s\n", path);
	if (stbuf.st_mode & S_IFREG)
		search_in_file(substr, path);
	if (stbuf.st_mode & S_IFDIR)
		walk(substr, path);
}

void walk(char* substr, char* path) {
	DIR *dir = opendir(path);
	if (dir == NULL) {
		perror(path);
	}
	struct dirent* dp;
	while ((dp = readdir(dir)) != NULL) {
		if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
			continue;
		size_t path_len = strlen(path);
		size_t name_len = strlen(dp->d_name);
		char *new_path = malloc(path_len + 1 + name_len + 1);
		strcpy(new_path, path);
		strcat(new_path, "/");
		strcat(new_path, dp->d_name);
		if (dp->d_type & DT_DIR) 
			walk(substr, new_path);
		if (dp->d_type & DT_REG) 
			search_in_file(substr, new_path);
		free(new_path);
	}
	closedir(dir);
}

int main(int argc, char* argv[]) {
	char rec = 0;
	char ch;
	while ((ch = getopt(argc, argv, "R")) != -1) {
		if (ch == 'R')
			rec = 1;
		else {
			fprintf(stderr, "Некорректныый аргумент\n");
			return 1;
		}
	} 
	argv += optind;
	argc -= optind;
	if (argc < 2) {
		fprintf(stderr, "Некорректные аргументы\n");
		return 1;
	}
	my_grep(argv[0], argv[1]);
	return 0;
}
