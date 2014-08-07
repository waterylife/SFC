/*
* 使用范例
* Linux共享存储（父子进程读写同一共享内存段）
*/

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define SHM_KEY 0x20140503
#define SHM_SIZE 1024
#define SHM_MODE 0600 //read & write

extern int pfd1[2]; //parent --> child
extern int pfd2[2]; //parent <-- child

int tell_wait();
int tell_child();
int wait_child();
int tell_parent();
int wait_parent();

int parent_process()
{
	int shm_id;
	void* shm_ptr = NULL;

	shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | IPC_EXCL | SHM_MODE);
	if(-1 == shm_id) {
		printf("[P]Fail to create shm, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return -1;
	}

	shm_ptr = shmat(shm_id, 0, 0);
	if((void*)-1 == shm_ptr) {
		printf("[P]Fail to attach to shm, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return -1;
	}

	char buf[1024] = "String from parent.";
	memcpy(shm_ptr, (const void*)buf, strlen(buf));

	if(-1 == tell_child()) {
		printf("[P]fail to tell_child, process exit\n");
		shmdt(shm_ptr);
		shmctl(shm_id, IPC_RMID, 0);
		_exit(0);
	}

	if(-1 == wait_child()) {
		printf("[P]fail to wait_child, process exit\n");
		shmdt(shm_ptr);
		shmctl(shm_id, IPC_RMID, 0);
		_exit(0);
	}

	shmctl(shm_id, IPC_RMID, 0);

	return 0;
}

int child_process()
{
	int shm_id;
	void* shm_ptr = NULL;

	if(-1 == wait_parent()) {
		printf("[C]fail to wait_parent\n");
		return -1;
	}


	shm_id = shmget(SHM_KEY, SHM_SIZE, SHM_MODE);
	if(-1 == shm_id) {
		printf("[C]Fail to open shm, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return -1;
	}

	shm_ptr = shmat(shm_id, 0, 0);
	if((void*)-1 == shm_ptr) {
		printf("[C]Fail to attach to shm, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return -1;
	}

	char buf[1024] = {0};
	memcpy((void*)buf, shm_ptr, strlen(shm_ptr));
	printf("[C]Shm contents: %s\n", buf);

	shmdt(shm_ptr);

	if(-1 == tell_parent()) {
		printf("[C]Fail to tell_parent\n");
		return -1;
	}

	return 0;
}

int main()
{
	if(-1 == tell_wait()) {
		printf("fail to tell_wait\n");
		return 0;
	}

	pid_t pid = fork();
	if(0 > pid) {
		printf("fail to fork, errno: %d, errmsg: %s\n", errno, strerror(errno));
		return 0;
	}
	else if(0 == pid) {
		child_process();
		_exit(0);
	}

	parent_process();
	int status;
	if(-1 == wait(&status)) {
		printf("[P]fail to wait for child process, errno: %d, errmsg: %s\n", errno, strerror(errno));
	}

	return 0;
}