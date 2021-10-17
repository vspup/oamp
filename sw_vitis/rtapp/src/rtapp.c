/* This is a sample demonstration application that showcases usage of rpmsg
This application is meant to run on the remote CPU running baremetal code.
This application echoes back data that was sent to it by the master core. */

#include "rtapp.h"
#include "xil_printf.h"
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"
#include <stdbool.h>
#include "../../mps2/mps2.h"

#define SHUTDOWN_MSG	0xEF56A55A

#define LPRINTF(format, ...) xil_printf(format, ##__VA_ARGS__)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

static struct rpmsg_endpoint lept;
static int shutdown_req = 0;


static struct _msg *inc_buf;
static struct _msg *rep_buf;

// calculate max size of buffer
#define RPMSG_HEADER_LEN 16
#define MAX_RPMSG_BUFF_SIZE (512 - RPMSG_HEADER_LEN)
#define DATA_MAX_SIZE	(MAX_RPMSG_BUFF_SIZE - 3*sizeof(int))





bool f_cmd = false;

// me variable
int on_off = 0;
int V = 0, I = 0;
int T[12] = {200, 201, 202, 203, 404, 505, 606, 77, 89, 109, 110, 111};

Command_Codes code;

int data_size;
int dt[DATA_MAX_SIZE];





/* External functions */
extern int init_system(void);
extern void cleanup_system(void);

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
			     uint32_t src, void *priv)
{
	(void)priv;
	(void)src;

	/* On reception of a shutdown we signal the application to terminate */
	if ((*(unsigned int *)data) == SHUTDOWN_MSG) {
		LPRINTF(" [R5--> shutdown message is received] ");
		shutdown_req = 1;
		return RPMSG_SUCCESS;
	}


	inc_buf = (struct _msg *)malloc((3 + DATA_MAX_SIZE) * sizeof(int));
	memcpy(inc_buf, data, len);
	f_cmd = true;


	code = inc_buf->code;
	data_size = inc_buf->size;
	if(data_size > 0){
		memcpy(&dt, inc_buf->data, inc_buf->size);
		LPRINTF(" [R5-->  data[0] = %d.] ", inc_buf->data[0]);
	}

	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	LPRINTF(" [R5--> unexpected Remote endpoint destroy] ");
	shutdown_req = 1;
}




int main(int argc, char *argv[])
{
	void *platform;
	struct rpmsg_device *rpdev;
	int ret, i, j;

	/* Initialize platform */
	LPRINTF(" [R5]--> Starting application...] ");
	ret = platform_init(argc, argv, &platform);
	if (ret) {
		LPERROR(" [err R5--> Failed to initialize platform.] ");
		ret = -1;
	}
	else {
		rpdev = platform_create_rpmsg_vdev(platform, 0, VIRTIO_DEV_SLAVE, NULL, NULL);
		if (!rpdev) {
			LPERROR(" [err R5--> Failed to create rpmsg virtio device.]");
			ret = -1;
		}
		else {
			/* Initialize RPMSG framework */
			LPRINTF(" [R5--> Try to create rpmsg endpoint.]");

			ret = rpmsg_create_ept(&lept, rpdev, RPMSG_SERVICE_NAME, 0, RPMSG_ADDR_ANY, rpmsg_endpoint_cb, rpmsg_service_unbind);
			if (ret) {
				LPERROR(" [err R5--> Failed to create endpoint.]");
				return -1;
			}

			LPRINTF(" [R5--> Successfully created rpmsg endpoint.]");
			ret = 1;
		}
	}
	/* ***** */


	/* main loop */

	rep_buf = (struct _msg *)malloc((3 + DATA_MAX_SIZE) * sizeof(int));

	while(1) {
		platform_poll(platform);

		/* we got a shutdown request, exit */
		if (shutdown_req) {
			break;
		}

		if (f_cmd){
			rep_buf->size = 1;
			rep_buf->code = ACK;

			switch (code){
				case SET_REGIME:
					on_off = dt[0];
					break;
				case SET_V:
					V = dt[0];
					break;
				case GET_REGIME:
					rep_buf->data[0] = on_off;
					break;
				case GET_V:
					rep_buf->data[0] = V;
					break;
				case GET_T:
					rep_buf->size = 12;
					for(j = 0; j<12; j++){
						rep_buf->data[j] = T[j];
					}
					break;


				default:
					rep_buf->code = ERR;
					rep_buf->data[0] = E_CODE;
					break;
			}

			if (rpmsg_send(&lept, rep_buf, (3 + rep_buf->size) * sizeof(int)) < 0) {
					LPERROR(" [err R5--> rpmsg_send failed]");
			}
			f_cmd = false;
		}
	}
	/* ***** */


	// finalize RPMSG
	rpmsg_destroy_ept(&lept);
	platform_release_rpmsg_vdev(rpdev);
	ret = 0;
	LPRINTF(" [R5--> Stopping application...]");
	platform_cleanup(platform);
	// *****

	return ret;
}
