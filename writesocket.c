#define _GNU_SOURCE
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	struct sockaddr_un path;
	int s;

	if (argc < 2)
		{ printf("%s: <filename> [uid] [gid]\n", argv[0]); return 2; }

	if (argc > 3) {
		gid_t gid = atoi(argv[3]);
		gid_t groups[1] = { gid };

		if (setresgid(gid, gid, gid)!=0)
			{ perror("setresgid"); return 1; }

		if (setgroups(1, groups)!=0)
			{ perror("setgroups"); return 1; }
	}

	if (argc > 2) {
		uid_t uid = atoi(argv[2]);
		if (setresuid(uid, uid, uid)!=0)
			{ perror("setresuid"); return 1; }
	}

	if (strlen(argv[1]) > sizeof(path.sun_path)-1)
		{ fprintf(stderr, "Filename \"%s\" too long.\n", argv[1]); return 2; }

	if ((s = socket(PF_FILE, SOCK_STREAM, 0))==-1)
		{ perror("socket"); return 1; }

	memset(&path, 0, sizeof(path));
	path.sun_family = AF_FILE;
	strncpy(path.sun_path, argv[1], sizeof(path.sun_path)-1);

	if (connect(s, (struct sockaddr*)&path, sizeof(path))!=0)
		{ perror("connect"); return 1; }

	if (dup2(s, 1)!=1)
		{ perror("dup2"); return 1; }

	if (close(s)!=0)
		{ perror("close"); return 1; }

	execlp("cat", "writesocket", NULL);
	perror("execlp");
	return 1;
}
