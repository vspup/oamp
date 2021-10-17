/*

*/

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <linux/rpmsg.h>
#include <stdbool.h>

#include "../../mps2/mps2.h"



static int charfd = -1, fd = -1, err_cnt;

struct _msg *i_msg; // issue massage
struct _msg *r_msg; // reply massage

Command_Codes code;

// calculate max size of buffer
#define RPMSG_HEADER_LEN 16
#define MAX_RPMSG_BUFF_SIZE (512 - RPMSG_HEADER_LEN)
#define DATA_MAX_SIZE	(MAX_RPMSG_BUFF_SIZE - 3*sizeof(int))

// name of channel
#define RPMSG_BUS_SYS "/sys/bus/rpmsg"


static int rpmsg_create_ept(int rpfd, struct rpmsg_endpoint_info *eptinfo)
{
	int ret;

	ret = ioctl(rpfd, RPMSG_CREATE_EPT_IOCTL, eptinfo);
	if (ret)
		perror("! err [Linux]-> Failed to create end point.\n");
	return ret;
}

static char *get_rpmsg_ept_dev_name(const char *rpmsg_char_name,
				    const char *ept_name,
				    char *ept_dev_name)
{
	char sys_rpmsg_ept_name_path[64];
	char svc_name[64];
	char *sys_rpmsg_path = "/sys/class/rpmsg";
	FILE *fp;
	int i;
	int ept_name_len;

	for (i = 0; i < 128; i++) {
		sprintf(sys_rpmsg_ept_name_path, "%s/%s/rpmsg%d/name",
			sys_rpmsg_path, rpmsg_char_name, i);
		printf("checking %s\n", sys_rpmsg_ept_name_path);
		if (access(sys_rpmsg_ept_name_path, F_OK) < 0)
			continue;
		fp = fopen(sys_rpmsg_ept_name_path, "r");
		if (!fp) {
			printf("! err [Linux]-> Failed to open %s\n", sys_rpmsg_ept_name_path);
			break;
		}
		fgets(svc_name, sizeof(svc_name), fp);
		fclose(fp);
		printf("[Linux]-> svc_name: %s.\n",svc_name);
		ept_name_len = strlen(ept_name);
		if (ept_name_len > sizeof(svc_name))
			ept_name_len = sizeof(svc_name);
		if (!strncmp(svc_name, ept_name, ept_name_len)) {
			sprintf(ept_dev_name, "rpmsg%d", i);
			return ept_dev_name;
		}
	}

	printf("! err [Linux]-> Not able to RPMsg endpoint file for %s:%s.\n",
	       rpmsg_char_name, ept_name);
	return NULL;
}

static int bind_rpmsg_chrdev(const char *rpmsg_dev_name)
{
	char fpath[256];
	char *rpmsg_chdrv = "rpmsg_chrdev";
	int fd;
	int ret;


	/* rpmsg dev overrides path */
	sprintf(fpath, "%s/devices/%s/driver_override",	RPMSG_BUS_SYS, rpmsg_dev_name);
	fd = open(fpath, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "! err [Linux]-> Failed to open %s, %s\n", fpath, strerror(errno));
		return -EINVAL;
	}
	ret = write(fd, rpmsg_chdrv, strlen(rpmsg_chdrv) + 1);
	if (ret < 0) {
		fprintf(stderr, "! err [Linux]-> Failed to write %s to %s, %s\n", rpmsg_chdrv, fpath, strerror(errno));
		return -EINVAL;
	}
	close(fd);

	/* bind the rpmsg device to rpmsg char driver */
	sprintf(fpath, "%s/drivers/%s/bind", RPMSG_BUS_SYS, rpmsg_chdrv);
	fd = open(fpath, O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "! err [Linux]-> Failed to open %s, %s\n",
			fpath, strerror(errno));
		return -EINVAL;
	}
	ret = write(fd, rpmsg_dev_name, strlen(rpmsg_dev_name) + 1);
	if (ret < 0) {
		fprintf(stderr, "! err [Linux]-> Failed to write %s to %s, %s\n",
			rpmsg_dev_name, fpath, strerror(errno));
		return -EINVAL;
	}
	close(fd);
	return 0;
}

static int get_rpmsg_chrdev_fd(const char *rpmsg_dev_name,
			       char *rpmsg_ctrl_name)
{
	char dpath[256];
	char fpath[256];
	char *rpmsg_ctrl_prefix = "rpmsg_ctrl";
	DIR *dir;
	struct dirent *ent;
	int fd;

	sprintf(dpath, "%s/devices/%s/rpmsg", RPMSG_BUS_SYS, rpmsg_dev_name);
	dir = opendir(dpath);
	if (dir == NULL) {
		fprintf(stderr, "! err [Linux]-> Failed to open dir %s\n", dpath);
		return -EINVAL;
	}
	while ((ent = readdir(dir)) != NULL) {
		if (!strncmp(ent->d_name, rpmsg_ctrl_prefix,
			    strlen(rpmsg_ctrl_prefix))) {
			printf("[Linux]-> Opening file %s.\n", ent->d_name);
			sprintf(fpath, "/dev/%s", ent->d_name);
			fd = open(fpath, O_RDWR | O_NONBLOCK);
			if (fd < 0) {
				fprintf(stderr,
					"! err [Linux]-> Failed to open rpmsg char dev %s,%s\n",
					fpath, strerror(errno));
				return fd;
			}
			sprintf(rpmsg_ctrl_name, "%s", ent->d_name);
			return fd;
		}
	}

	fprintf(stderr, "! err [Linux]-> No rpmsg char dev file is found\n");
	return -EINVAL;
}

int main(int argc, char *argv[])
{
	int ret, i, j;
	int size, bytes_rcvd, bytes_sent;
	err_cnt = 0;

	/* Adjustment to open amp communication */

	// variable for o_amp
	char *rpmsg_dev="virtio0.rpmsg-openamp-demo-channel.-1.0";
	char fpath[256];
	char rpmsg_char_name[16];
	struct rpmsg_endpoint_info eptinfo;
	char ept_dev_name[16];
	char ept_dev_path[32];

	printf("\n\n[Linux]--> START Adjustment to open amp communication\r\n");

	/* Load rpmsg_char driver */
	printf("[Linux]--> probe rpmsg_char driver\r\n");
	ret = system("modprobe rpmsg_char");
	if (ret < 0) {
		perror("! err [Linux]--> Failed to load rpmsg_char driver.\n");
		return -EINVAL;
	}

	/* Access rpmsg device */
	printf("[Linux]--> Open rpmsg dev %s! \r\n", rpmsg_dev);
	sprintf(fpath, "%s/devices/%s", RPMSG_BUS_SYS, rpmsg_dev);
	if (access(fpath, F_OK)) {
		fprintf(stderr, "! err [Linux]--> Not able to access rpmsg device %s, %s\n",
			fpath, strerror(errno));
		return -EINVAL;
	}
	ret = bind_rpmsg_chrdev(rpmsg_dev);
	if (ret < 0)
		return ret;
	charfd = get_rpmsg_chrdev_fd(rpmsg_dev, rpmsg_char_name);
	if (charfd < 0)
		return charfd;

	/* Create endpoint from rpmsg char driver */
	printf("[Linux]--> Create endpoint from rpmsg char driver \r\n");
	strcpy(eptinfo.name, "rpmsg-openamp-demo-channel");
	eptinfo.src = 0;
	eptinfo.dst = 0xFFFFFFFF;
	ret = rpmsg_create_ept(charfd, &eptinfo);
	if (ret) {
		printf("! err [Linux]--> Failed to create RPMsg endpoint.\n");
		return -EINVAL;
	}
	if (!get_rpmsg_ept_dev_name(rpmsg_char_name, eptinfo.name,
				    ept_dev_name))
		return -EINVAL;
	sprintf(ept_dev_path, "/dev/%s", ept_dev_name);
	fd = open(ept_dev_path, O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		perror("! err [Linux]--> Failed to open rpmsg device.");
		close(charfd);
		return -1;
	}

	printf("[Linux]--> FINISH Adjustment to open amp communication\r\n\n\n");
	/* finish Adjustment to open amp communication */


	/* allocate memory for massages */
	printf("[Linux]--> Allocate memory for massages \r\n");
	i_msg = (struct _msg *)malloc((3 + DATA_MAX_SIZE) * sizeof(int));
	r_msg = (struct _msg *)malloc((3 + DATA_MAX_SIZE) * sizeof(int));

	if (i_msg == 0 || r_msg == 0) {
		printf("! err [Linux]--> Failed to allocate memory for massages.\n");
		return -1;
	}

	/* cycle of responding messages */
	i = 0;


	bool job = true;

	//int command [] = {1, 2, 3, 4, 5};
	//int count_command = sizeof(command)/sizeof(command[0]);

	while (job){
		int cmd =-1;
		i_msg->num =  i;
		// Wait new command
		printf("[Linux]--> Input new command \r\n");
		scanf("%d", &cmd);

		switch (cmd){
		case 1: // set regime on
			i_msg->code = SET_REGIME;
			size =  1;
			i_msg->data[0] = RAMP_UP;
			break;
		case 2: // get current regime
			i_msg->code = GET_REGIME;
			size =  0;
			i_msg->data[0] = 0;
			break;
		case 3: // get current V
			i_msg->code = GET_V;
			size =  0;
			i_msg->data[0] = 0;
			break;
		case 4: // set V
			i_msg->code = SET_V;
			size =  1;
			i_msg->data[0] = 14;
			break;
		case 5: // get current V
			i_msg->code = GET_V;
			size =  0;
			i_msg->data[0] = 0;
			break;
		case 6: // get current T
			i_msg->code = GET_T;
			size =  0;
			i_msg->data[0] = 0;
			break;
		case 7: // set regime STANDBY
			i_msg->code = SET_REGIME;
			size =  1;
			i_msg->data[0] = STANDBY;
			break;
		case 8: // get current regime
			i_msg->code = GET_REGIME;
			size =  0;
			i_msg->data[0] = 0;
			job = false;
			break;

		default:
			i_msg->code = 333;
			size =  1;
			i_msg->data[0] = 0;
			break;
		}

		i_msg->size =  size;


		//printf((cmd == 0) ? "ON %d \r\n" : ((cmd == 1)? "REGIME %d \r\n": "Other %d \r\n"), i_msg->code);
		printf("[Linux]--> Send message ");
		printf("num = %d, code = %d, size %d, data[0] %d \r\n", i_msg->num, i_msg->code, i_msg->size, i_msg->data[0]);


		bytes_sent = write(fd, i_msg, (3 + size) * sizeof(int));

		if (bytes_sent <= 0) {
			printf("!err [Linux]--> Error sending data \r\n");
			break;
		}

		printf("\r\n [Linux]--> sent : %d\n", bytes_sent);

		r_msg->num = 0;
		r_msg->code = 0;

		bytes_rcvd = read(fd, r_msg, (3 + DATA_MAX_SIZE) * sizeof(unsigned long));
		while (bytes_rcvd <= 0) {
			usleep(10000);
			bytes_rcvd = read(fd, r_msg, (3 + DATA_MAX_SIZE) * sizeof(unsigned long));
		}
		printf("\r\n [Linux]--> received massages number ");
		printf("%d of size %d\r\n", r_msg->num, bytes_rcvd);
		printf("\r\n [Linux]--> Return Code =  %d ", r_msg->code);
		printf(" Read data[%d] =");
		for (j=0; j < r_msg->size; j++){
			printf(" %d ", r_msg->data[j]);
		}
		printf("\r\n\n");


		i++;
	}

	free(i_msg);
	free(r_msg);

	close(fd);
	if (charfd >= 0)
		close(charfd);
	return 0;
}
