#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>

#include <linux/videodev.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

#include "jconfig.h"
#include "avilib.h"
#include "utils.h"

#define WAVE_AUDIO_PCM 1
#define CHANNELS 1
#define AUDIO_SAMPLERATE		22050 // 44100 // 11025 // 8000 // 22050
#define AUDIO_SAMPLES			512 // 1024
#define AUDIO_FORMAT			AUDIO_S16
#define AUDIOSIZE 2
#define NUM_CHUNCK 64 // 256 a large ring buffer 
#define MAXBUFFERSIZE (AUDIO_SAMPLES*AUDIOSIZE*NUM_CHUNCK)
#define HELPBUFFERSIZE (AUDIO_SAMPLES*AUDIOSIZE)

/* picture structure */
typedef struct pictstruct{
	unsigned char *data;
	int sizeIn;
	int width;
	int height;
	int formatIn;
	int mode;
	avi_t *out_fd;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} pictstruct;

static int Oneshoot = 1;
static int numshoot = 0;
static int modshoot = 0;
volatile int pictFlag = 0;

static void spcaPrintParam (int fd, struct video_param *videoparam)
{
	if(ioctl(fd,SPCAGVIDIOPARAM, videoparam) == -1){
		printf ("wrong spca5xx device\n");
	} else 
		printf("quality %d autoexpo %d Timeframe %d \n",
			 videoparam->quality,videoparam->autobright,videoparam->time_interval);
}

void *waitandshoot(void* ptr) { 
  pictstruct *mypict = (pictstruct *) ptr;
  int width = mypict->width;
  int height = mypict->height;
  int format = mypict->formatIn;
  int size = mypict->sizeIn;
  int mode = mypict->mode;
  avi_t *fd = mypict->out_fd;
  int status;
  while(1){
    status = pthread_mutex_lock (&mypict->mutex);
    if (status != 0) {fprintf(stderr,"Lock error!\n"); exit(-1);}
    while (pictFlag == 0) {
      printf("pictflag is 0\r\n");
      /* Set predicate */
      status = pthread_cond_wait(&mypict->cond, &mypict->mutex);
      if (status != 0) {fprintf(stderr,"Wait error!\n"); exit(-1);}
    }
    pictFlag = 0;
    getJpegPicture(mypict->data,width,height,format,size,mode,fd);
    if (modshoot)
      if (!(--numshoot)) Oneshoot = 0;
    status = pthread_mutex_unlock (&mypict->mutex);
    if (status != 0) {fprintf(stderr,"Unlock error!\n"); exit(-1);}
  }
  return NULL;
}

static int setVideoPict (struct video_picture *videopict, int fd)
{
  if (ioctl (fd, VIDIOCSPICT, videopict) < 0) {
    perror ("Couldnt get videopict params with VIDIOCSPICT\n");
    return -1;
  }
  printf ("VIDIOCSPICT\n");
  printf ("brightness=%d hue=%d color=%d contrast=%d whiteness=%d \n",
      videopict->brightness, videopict->hue,
      videopict->colour, videopict->contrast, videopict->whiteness);
  printf("depth=%d palette=%d \n",videopict->depth, videopict->palette);

  return 0;
}

static int isSpcaChip (const char *BridgeName)
{ 
  int i = -1;
  int size =0;
  //size = strlen(BridgeName)-1;
  /* Spca506 return more with channel video, cut it */
  //if (size > 10) size = 8;
  /* return Bridge otherwhise -1 */
  for (i=0; i < MAX_BRIDGE ;i++){
    size=strlen(Blist[i].name);
    if(strncmp(BridgeName,Blist[i].name,size) == 0) {
      printf("Bridge find %s number %d\n",Blist[i].name,i);
      break;
    }
  }

  return i;
}

static int probeSize (const char *BridgeName, int *width, int *height)
{ 	int bridge = -1;
	int i;
	unsigned int intwidth;
	unsigned int intheight;
	unsigned int intformat;
	int match =0;
/* return 1->ok otherwhise -1 */
	if ((bridge= isSpcaChip(BridgeName)) < 0) {
	printf ("Cannot Probe Size !! maybe not an Spca5xx Camera\n");
	return -1;
	}
	
	for (i=0; (unsigned int)(GET_EXT_MODES(bridge)[i][0]);i++){
		intwidth = GET_EXT_MODES(bridge)[i][0];
		intheight = GET_EXT_MODES(bridge)[i][1];
		intformat = (GET_EXT_MODES(bridge)[i][2] & 0xF0) >> 4;
		if ((intwidth== *width) && (intheight == *height)){
			match = 1;
		} else {
			match = 0;
		}
		
		printf("Available Resolutions width %d  heigth %d %s %c\n",
			intwidth,intheight,(intformat)?"decoded":"native",
			(match)?'*':' ');
	}

}

static int getStreamId (const char * BridgeName)
{ 
  int i = -1;
  int match = -1;
  /* return Stream_id otherwhise -1 */
  if((match=isSpcaChip(BridgeName)) < 0){
    printf("Not an Spca5xx Camera !!\n");
    return match;
  }
  switch (match) {
    case BRIDGE_SPCA505:
    case BRIDGE_SPCA506: 
      i= YYUV;
      break;
    case BRIDGE_SPCA501:
      i = YUYV;
      break;
    case BRIDGE_SPCA508:
      i = YUVY;
      break;
    case BRIDGE_SPCA504:
    case BRIDGE_SPCA500:
    case BRIDGE_SPCA504B:
    case BRIDGE_SPCA533:
    case BRIDGE_SPCA504C:
    case BRIDGE_SPCA536:
    case BRIDGE_ZR364XX:
    case BRIDGE_ZC3XX:
    case BRIDGE_CX11646:
    case BRIDGE_SN9CXXX:
    case BRIDGE_MR97311:
      i = JPEG;
      break;
    case BRIDGE_ETOMS:
    case BRIDGE_SONIX:
    case BRIDGE_SPCA561:
    case BRIDGE_TV8532:
      i = GBRG;
      break;
    default:
      i = -1;
      printf("Unable to find a StreamId !!\n");
      break;

  }
  return i;	 
}

int main(void) {
  pictstruct mypict ;
  int image_width  = IMAGE_WIDTH;
  int image_height = IMAGE_HEIGHT;
  int frame_size   = 0;
  int format       = VIDEO_PALETTE_YUV420P;
  int bpp          = 3;

  const char *videodevice = NULL;
  int fd;

  struct video_mbuf videombuf;
  struct video_mmap vmmap;
  struct video_capability videocap;
  struct video_picture videopict;
  struct video_channel videochan;
  struct video_param videoparam;
  unsigned char *pFramebuffer;
  unsigned char *tmp = NULL;	// don't forget the Till lesson
  int isVideoChannel = 1;
  int norme = 0;
  int channel = 0;
  int mmapsize;
  int streamid ;

  pthread_t waitandshoot_id;
  int wstatus ;
  int f;
  int ff;

  numshoot = 1;
  modshoot = 1;
  pictFlag = 1;
  format   = VIDEO_PALETTE_JPEG;
  Oneshoot = 1;

  switch (format) {
    case VIDEO_PALETTE_JPEG:{
			      frame_size = image_width * image_height;
			    }
			    break;
    case VIDEO_PALETTE_RAW_JPEG:{
				  frame_size = image_width *
				    image_height * 3;
				}
				break;
    case VIDEO_PALETTE_YUV420P:{
				 frame_size =
				   (image_width *
				    image_height * 3) >> 1;
			       }
			       break;
    case VIDEO_PALETTE_RGB565:
    case VIDEO_PALETTE_RGB24:
    case VIDEO_PALETTE_RGB32:{
			       frame_size =
				 image_width *
				 image_height * bpp;
			     }
			     break;
    default:
			     break;

  }

  /* Init video device */
  if (videodevice == NULL || *videodevice == 0) {
    videodevice = "/dev/video0";
  }
  printf ("Using video device %s.\n", videodevice);

  /* Init v4l */
  printf ("Initializing v4l.\n");
  if ((fd = open (videodevice, O_RDWR)) == -1) {
    perror ("ERROR opening V4L interface \n");
    exit (1);
  }
  
  /* Init Camera */
  printf("**************** PROBING CAMERA *********************\n");
  if (ioctl (fd, VIDIOCGCAP, &videocap) == -1) {
    printf ("wrong device\n");
    exit (1);
  }

  printf("Camera found: %s \n",videocap.name);

  if (ioctl (fd, VIDIOCGCHAN, &videochan) == -1) {
    printf ("Hmm did not support Video_channel\n");
    isVideoChannel = 0;
  }
  if (isVideoChannel){
    videochan.norm = norme;
    videochan.channel = channel;
    if (ioctl (fd, VIDIOCSCHAN, &videochan) == -1) {
      printf ("ERROR setting channel and norme \n");
      exit (1);
    }
    /************ just to be sure *************/
    if (ioctl (fd, VIDIOCGCHAN, &videochan) == -1) {
      printf ("wrong device\n");
      exit (1);
    }
    printf("Bridge found: %s \n",videochan.name);
    streamid = getStreamId (videochan.name);

    if (streamid >= 0){
      printf("StreamId: %s Camera\n",Plist[streamid].name);
      /* look a spca5xx webcam try to set the video param struct */
      spcaPrintParam (fd,&videoparam);
    } else {
      printf("StreamId: %d Unknow Camera\n",streamid);
    } 
    /* test jpeg capability if not and jpeg ask without raw data 
       set default format to YUVP */
    if ((format == VIDEO_PALETTE_RAW_JPEG || format == VIDEO_PALETTE_JPEG )&& streamid != JPEG ) {
      printf ("Camera unable to stream in JPEG mode switch to YUV420P\n");
      format = VIDEO_PALETTE_YUV420P;
    }
    if(probeSize(videochan.name,&image_width,&image_height) < 0)
      printf("unable to probe size !!\n");
  }
  printf("*****************************************************\n");
  printf(" grabbing method default MMAP asked \n");
  /* MMAP VIDEO acquisition */
  memset (&videombuf, 0, sizeof (videombuf));
  if (ioctl (fd, VIDIOCGMBUF, &videombuf) < 0) {
    perror (" init VIDIOCGMBUF FAILED\n");
  }
  printf ("VIDIOCGMBUF size %d  frames %d  offets[0]=%d offsets[1]=%d\n", videombuf.size, videombuf.frames, videombuf.offsets[0], videombuf.offsets[1]);

  pFramebuffer =
    (unsigned char *) mmap (0, videombuf.size,
			    PROT_READ | PROT_WRITE,
			    MAP_SHARED, fd, 0);
  mmapsize = videombuf.size;
  vmmap.height = image_height;
  vmmap.width = image_width;
  vmmap.format = format;
  for (f = 0; f < videombuf.frames; f++) {
    vmmap.frame = f;
    if (ioctl (fd, VIDIOCMCAPTURE, &vmmap)) {
      perror ("cmcapture");
    }
  }
  vmmap.frame = 0;

  /* struct video_picture VIDIOCGPICT VIDIOCSPICT */
  if (ioctl (fd, VIDIOCGPICT, &videopict) < 0) {
    perror ("Couldnt get videopict params with VIDIOCGPICT\n");
  }
  printf ("VIDIOCGPICT\n");
  printf("brightnes=%d hue=%d color=%d contrast=%d whiteness=%d \n" ,
      videopict.brightness, videopict.hue, videopict.colour, videopict.contrast, videopict.whiteness);
  printf("depth=%d palette=%d\n", videopict.depth, videopict.palette);

  videopict.palette = format;
  videopict.depth = bpp * 8;
  //videopict.brightness = INIT_BRIGHT;
  sleep (1);
  setVideoPict (&videopict, fd);

  /* Allocate buffer for one frame. */
  tmp = (unsigned char*)malloc (frame_size);

  mypict.data = malloc(frame_size);
  mypict.sizeIn = frame_size;
  mypict.width = image_width;
  mypict.height = image_height;
  mypict.formatIn = format;
  mypict.mode = PICTURE;

  pthread_mutex_init(&mypict.mutex, NULL);
  pthread_cond_init(&mypict.cond, NULL);
  wstatus = pthread_create (&waitandshoot_id, NULL, (void *) waitandshoot, &mypict);
  if (wstatus != 0) {
    fprintf(stderr,"thread shoot Create error!\n");
    exit(-1);
  }
  
  while (Oneshoot) {

    printf("Grabbing\r\n");
    memset(tmp,0x00,frame_size);

    ff = vmmap.frame;
    if (ioctl (fd, VIDIOCSYNC, &ff) < 0) {
      perror ("cvsync err\n");
    }
    vmmap.frame = ff;
    memcpy (tmp, pFramebuffer + videombuf.offsets[vmmap.frame], frame_size);
    if ((wstatus = ioctl (fd, VIDIOCMCAPTURE, &vmmap)) < 0) {
      perror ("cmcapture");
      printf (">>cmcapture err %d\n", wstatus);
    }
    vmmap.frame = (vmmap.frame + 1) % videombuf.frames;

    /* here the frame is in tmp ready to used */
    if (pictFlag){
      printf("pictflag returned to 1!\r\n");
    }
  }
  
  printf("Frame available\r\n");
  wstatus = pthread_mutex_lock (&mypict.mutex);
  if (wstatus != 0) {fprintf(stderr,"Lock error!\n"); exit(-1);}
  memcpy (mypict.data,tmp,frame_size);	

  wstatus = pthread_cond_signal (&mypict.cond);
  if (wstatus != 0) {fprintf(stderr,"Signal error!\n"); exit(-1);}
  wstatus = pthread_mutex_unlock (&mypict.mutex);
  if (wstatus != 0) {fprintf(stderr,"Unlock error!\n"); exit(-1);}


  printf ("closing\n");
  close (fd);
  printf ("closed\n");
  pthread_cancel(waitandshoot_id);
  pthread_join (waitandshoot_id,NULL);
  pthread_cond_destroy(&mypict.cond);
  pthread_mutex_destroy(&mypict.mutex);
  if (mypict.data)
    free(mypict.data);
  printf ("Destroy Picture thread ...\n");

  /* Shutdown all subsystems */
  free (tmp);
  printf ("Quiting....\n");
  
  return 0;	 
}
