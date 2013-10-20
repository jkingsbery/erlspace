
/* Defines */
#define NUM_DOTS 10

/* Includes */
#include <time.h>
#include <SDL.h>
#include "SDL/SDL_image.h"
#include <stdlib.h>
#include <string>

//Added for networking
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

/* Types */
typedef struct {
  int red,green,blue; 
  float vx,vy; 
  float x,y; 
}dot;


int sockfd = 0;
int shipNum=0;
char * playerName;
int cameraX=0;
int cameraY=0;

int send_socket(char * msg){

  char sendBuff[1025];
  strcpy(sendBuff,msg);
  write(sockfd,sendBuff,strlen(sendBuff));

}

pthread_t thread;

void * readSocket(void * argument){
  int n=0;
  char recvBuff[1024];
  memset(recvBuff, '0',sizeof(recvBuff));

  while ( (n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0) {
    recvBuff[n] = 0;
    if(fputs(recvBuff, stdout) == EOF)  {
      printf("\n Error : Fputs error\n");
    }
  } 
} 

int initNetworking(){
  
  //First, set up the write part
  struct sockaddr_in serv_addr; 
  int rc;

  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      printf("\n Error : Could not create socket \n");
      return 1;
    } 

  memset(&serv_addr, '0', sizeof(serv_addr)); 

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(1234); 
  serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");

  if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
      printf("\n inet_pton error occured\n");
      return 1;
    } 

  if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
      printf("\n Error : Connect Failed \n");
      return 1;
    } 
  //Then, set up the read part listening in another thread:
  rc = pthread_create(&thread, NULL, readSocket, (void *) NULL);

  //Then, login:
  
  char buf[256];
  sprintf(buf,"Name: %s\n", playerName);
  send_socket(buf);
  
}



/* Globals */
SDL_Surface *screen;
float demo_time_measure = 0.0f;
float demo_time_step = 0.0f;
dot demo_dots[NUM_DOTS];

float randFloat()
{
  float r;
  r = (float)(rand()%RAND_MAX); /* 0 ~ whatever RAND_MAX is */
  r /= (float)(RAND_MAX-1); /* one less than RAND_MAX makes it possible for 1.0f to happen */
  return r;
}

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800
#define SCREEN_BPP 32
#define SHIP_DRAW_X ((SCREEN_WIDTH/2)-16)
#define SHIP_DRAW_Y ((SCREEN_HEIGHT/2)-16)

/* Initialize dots */
void demo_init()
{
  int i;
  for(i = 0;i < NUM_DOTS;i++)
    {
      demo_dots[i].red = 255; //rand()%255;
      demo_dots[i].green = 255; //rand()%255;
      demo_dots[i].blue = 255; //rand()%255;
      demo_dots[i].vx = randFloat()*16.0f-8.0f;
      demo_dots[i].vy = randFloat()*16.0f-8.0f;
      demo_dots[i].x = randFloat()*SCREEN_WIDTH*1.0f;
      demo_dots[i].y = randFloat()*SCREEN_HEIGHT*1.0f;
    }
}

/* Handle dots */
void demo_handle()
{
  // TODO
}

SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };



/* Draw dots */
void demo_draw()
{
  int i,bpp,rank,x,y;
  Uint32 *pixel;
  /* Lock surface */
  SDL_LockSurface(screen);
  rank = screen->pitch/sizeof(Uint32);
  pixel = (Uint32*)screen->pixels;
  /* Draw all dots */
  for(i = 0;i < NUM_DOTS;i++)
    {
      /* Rasterize position as integer */
      x = (int)demo_dots[i].x+(cameraX*10);
      y = (int)demo_dots[i].y+(cameraY*10);
      /* Set pixel */
      if(0<=x && x<SCREEN_WIDTH && 0<=y && y<SCREEN_HEIGHT){
	pixel[x+y*rank] = SDL_MapRGBA(screen->format,demo_dots[i].red,demo_dots[i].green,demo_dots[i].blue,255);
      }
    }
  /* Unlock surface */
  SDL_UnlockSurface(screen);
}

/* Convert from timespec to float */
float demo_convert_time(struct timespec *ts)
{
  float accu;
  /* Combine the value into floating number */
  accu = (float)ts->tv_sec; /* Seconds that have gone by */
  accu *= 1000000000.0f; /* One second is 1000x1000x1000 nanoseconds, s -> ms, us, ns */
  accu += (float)ts->tv_nsec; /* Nanoseconds that have gone by */
  /* Bring it back into the millisecond range but keep the nanosecond resolution */
  accu /= 1000000.0f;
  return accu;
}

/* Start time */
void demo_start_time()
{
  struct timespec ts;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&ts);
  demo_time_measure = demo_convert_time(&ts);
}

/* End time */
void demo_end_time()
{
  struct timespec ts;
  float delta;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&ts);
  delta = demo_convert_time(&ts)-demo_time_measure; /* Find the distance in time */
  demo_time_step = delta/(1000.0f/16.0f); /* Weird formula, equals 1.0f at 16 frames a second */
}

SDL_Surface *load_image( std::string filename ) 
{
  //Temporary storage for the image that's loaded
  SDL_Surface* loadedImage = NULL;
    
  //The optimized image that will be used
  SDL_Surface* optimizedImage = NULL;

  //Load the image
  loadedImage = IMG_Load( filename.c_str() );

  //If nothing went wrong in loading the image
  if( loadedImage != NULL ){
      //Create an optimized image
      optimizedImage = SDL_DisplayFormat( loadedImage );
        
      //Free the old image
      SDL_FreeSurface( loadedImage );
  }

  //Return the optimized image
  return optimizedImage;
}

void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination )
{
  //Make a temporary rectangle to hold the offsets
  SDL_Rect offset;
    
  //Give the offsets to the rectangle
  offset.x = x;
  offset.y = y;

  //Blit the surface
  SDL_BlitSurface( source, NULL, destination, &offset );
}

void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL )
{
  //Holds offsets
  SDL_Rect offset;
    
  //Get offsets
  offset.x = x;
  offset.y = y;
    
  //Blit
  SDL_BlitSurface( source, clip, destination, &offset );
}
//TODO remove all this global state
int active=1;


void handleEvent(){
  SDL_Event ev;
  while(SDL_PollEvent(&ev)){

    if(ev.type==SDL_KEYDOWN){
      switch(ev.key.keysym.sym){
      case SDLK_UP:
	send_socket("up\n"); 
	break;
      case SDLK_DOWN:
	send_socket("down\n"); 
	break;
      case SDLK_LEFT:
	send_socket("left\n"); break;
      case SDLK_RIGHT:
	send_socket("right\n"); break;
      case SDLK_ESCAPE:
	active=0;break;
      }
    }
    if(ev.type == SDL_QUIT){
      active = 0;
    }
  }
}


int initializeSdl(){
  SDL_WM_SetCaption("ErlSpace",NULL);
  if(SDL_Init(SDL_INIT_VIDEO) != 0){
    fprintf(stderr,"Could not initialize SDL: %s\n",SDL_GetError());
    return 1;
  }

  screen = SDL_SetVideoMode(SCREEN_WIDTH,SCREEN_HEIGHT,SCREEN_BPP,SDL_SWSURFACE);
  if(!screen){
    fprintf(stderr,"Could not set video mode: %s\n",SDL_GetError());
    return 2;
  }
  SDL_EnableKeyRepeat(100,100);
  return 0;
}

  SDL_Surface * ship;

  SDL_Rect clip[100];


void drawGame(){
  /* Clear screen */
  SDL_FillRect(screen,NULL,SDL_MapRGBA(screen->format,0,0,0,255));
  /* Draw game */
  demo_draw();
  apply_surface(SHIP_DRAW_X,SHIP_DRAW_Y,ship,screen, &clip[shipNum]);
  /* Show screen */
  SDL_Flip(screen);

}

/* Main */
int main(int argn,char **argv)
{
  playerName=argv[1];
  shipNum=atoi(argv[2]);
  printf("%s\n",playerName);
  printf("%d\n",shipNum);
  

  int error=initializeSdl();
  if(error){
    return error;
  }

  /* Load Images */
  ship=load_image("resources/samplespaceships.png");
  int i,j;
  for(i=0; i<10; i++){
    for(j=0; j<10; j++){
      clip[i*10+j].x=44*i;
      clip[i*10+j].y=44*j;
      clip[i*10+j].w=44;
      clip[i*10+j].h=44;
    }
  }
  
  demo_init();
  initNetworking();
  /* Main loop */
  active = 1;
  while(active)
    {
      handleEvent();

      demo_start_time();
      demo_handle();
      
      drawGame();
      /* End time */
      demo_end_time();
    }
  /* Exit */
  SDL_Quit();
  return 0;
}
