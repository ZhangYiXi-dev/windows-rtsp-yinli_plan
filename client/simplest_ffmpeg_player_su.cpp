/**
 * 最简单的基于FFmpeg的视频播放器2(SDL升级版)
 * Simplest FFmpeg Player 2(SDL Update)
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序实现了视频文件的解码和显示(支持HEVC，H.264，MPEG2等)。
 * 是最简单的FFmpeg视频解码方面的教程。
 * 通过学习本例子可以了解FFmpeg的解码流程。
 * 本版本中使用SDL消息机制刷新视频画面。
 * This software is a simplest video player based on FFmpeg.
 * Suitable for beginner of FFmpeg.
 *
 */

#include <stdio.h>
#include <iostream>
#include <WinSock2.h>
#include <windows.h>
#include <conio.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "legacy_stdio_definitions.lib")
#define __STDC_CONSTANT_MACROS
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }       
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "SDL2/SDL.h"
};

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0) //必要
//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)
DWORD WINAPI ThreadProFunc(LPVOID lpParam);
DWORD WINAPI ThreadProFunc(LPVOID lpParam)
{
	WSADATA data;

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
	{
		std::cout << "WSAStartup error";
		system("pause");
		return 1;
	}

	SOCKADDR_IN addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(12160);
	addrServer.sin_addr.S_un.S_addr = inet_addr("121.5.5.221");

	SOCKET socketClient = socket(AF_INET, SOCK_STREAM, 0);
	if (socketClient == INVALID_SOCKET)
	{
		std::cout << "socket create error";
		system("pause");
		return 1;
	}

	if (connect(socketClient, (struct sockaddr*)&addrServer, sizeof(addrServer)) == INVALID_SOCKET)
	{
		std::cout << "connect error";
		system("pause");
		return 1;
	}
	std::cout << "connect success,wait data......" << std::endl;
	while (1)
	{
		int ch;
		if (KEY_DOWN(VK_CONTROL))
		{
			while (KEY_DOWN(VK_CONTROL))
			{
				;
			}
			char sendData[] = "1216";
			int a;
			send(socketClient, sendData, sizeof(sendData), 0);
			Sleep(1);
		}
		if (KEY_DOWN(VK_MENU))
		{
			while (KEY_DOWN(VK_MENU))
			{
				;
			}
			char sendData[] = "7777";
			int a;
			send(socketClient, sendData, sizeof(sendData), 0);
			Sleep(1);
		}
		if (_kbhit())
		{
			ch = _getch();
			while (_kbhit())
			{
				;
			}
			char p[20];
			sprintf(p, "%d", ch);
			send(socketClient, p, strlen(p), 0);
			Sleep(1);
		}
		Sleep(1);
	}

	closesocket(socketClient);

	WSACleanup();
}
int thread_exit=0;

int sfp_refresh_thread(void *opaque){
	thread_exit=0;
	while (!thread_exit) {
		SDL_Event event;
		event.type = SFM_REFRESH_EVENT;
		SDL_PushEvent(&event);
		SDL_Delay(1);
	}
	thread_exit=0;
	//Break
	SDL_Event event;
	event.type = SFM_BREAK_EVENT;
	SDL_PushEvent(&event);

	return 0;
}


int main(int argc, char* argv[])
{
	//创建线程
	HANDLE hThread;
	DWORD dwThreadId;

	hThread = CreateThread(NULL	// 默认安全属性
		, NULL		// 默认堆栈大小
		, ThreadProFunc // 线程入口地址
		, NULL	//传递给线程函数的参数
		, 0		// 指定线程立即运行
		, &dwThreadId	//线程ID号
	);
	//正式程序
	AVFormatContext	*pFormatCtx;
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	AVFrame	*pFrame,*pFrameYUV;
	uint8_t *out_buffer;
	AVPacket *packet;
	int ret, got_picture;

	//------------SDL----------------
	int screen_w,screen_h;
	SDL_Window *screen; 
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;
	SDL_Thread *video_tid;
	SDL_Event event;

	struct SwsContext *img_convert_ctx;

	//const char* url = "rtmp://121.5.5.221:1935/rtmp_live/zyx";
    const char* url = "rtsp://121.5.5.221:8554/zyx";
	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();
	avdevice_register_all();


	//AVInputFormat* ifmt = av_find_input_format("gdigrab");
	AVDictionary* avdic = NULL;
	char option_key[] = "rtsp_transport";
	char option_value[] = "tcp";
	av_dict_set(&avdic, option_key, option_value, 0);
	/*char option_key2[] = "max_delay";
	char option_value2[] = "0";
	av_dict_set(&avdic, option_key2, option_value2, 0);*/
	
	if (avformat_open_input(&pFormatCtx, url, 0, &avdic) != 0) {
		printf("Couldn't open input stream.\n");
		return -1;
	}
	
	if(avformat_find_stream_info(pFormatCtx,NULL)<0){
		printf("Couldn't find stream information.\n");
		return -1;
	}
	videoindex=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++) 
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			videoindex=i;
			break;
		}
	if(videoindex==-1){
		printf("Didn't find a video stream.\n");
		return -1;
	}
	pCodecCtx=pFormatCtx->streams[videoindex]->codec;
	pCodecCtx->flags |= 0x00080000;
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	
	if(pCodec==NULL){
		printf("Codec not found.\n");
		return -1;
	}
	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
		printf("Could not open codec.\n");
		return -1;
	}

	pFrame=av_frame_alloc();
	pFrameYUV=av_frame_alloc();
	out_buffer=(uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
		pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL); 
	

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {  
		printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
		return -1;
	} 
	//SDL 2.0 Support for multiple windows
	screen_w = 0.8*pCodecCtx->width;
	screen_h = 0.8 *pCodecCtx->height;
	//screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		//screen_w, screen_h,SDL_WINDOW_OPENGL);
	screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_w , screen_h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

	if(!screen) {  
		printf("SDL: could not create window - exiting:%s\n",SDL_GetError());  
		return -1;
	}
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);  
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,pCodecCtx->width,pCodecCtx->height);  

	sdlRect.x=0;
	sdlRect.y=0;
	sdlRect.w=screen_w;
	sdlRect.h=screen_h;

	packet=(AVPacket *)av_malloc(sizeof(AVPacket));

	video_tid = SDL_CreateThread(sfp_refresh_thread,NULL,NULL);
	//------------SDL End------------
	//Event Loop
	
	for (;;) {
		//Wait
		SDL_WaitEvent(&event);
		if(event.type==SFM_REFRESH_EVENT){
			//------------------------------
			if(av_read_frame(pFormatCtx, packet)>=0){
				if(packet->stream_index==videoindex){
					ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
					if(ret < 0){
						printf("Decode Error.\n");
						//return -1;
					}
					if(got_picture){
						sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
						//SDL---------------------------
						SDL_UpdateTexture( sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0] );  
						SDL_RenderClear( sdlRenderer );  
						//SDL_RenderCopy( sdlRenderer, sdlTexture, &sdlRect, &sdlRect );  
						SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, NULL);  
						SDL_RenderPresent( sdlRenderer );  
						//SDL End-----------------------
					}
				}
				av_free_packet(packet);
			}else{
				//Exit Thread
				break;
				thread_exit=1;
			}
		}else if(event.type==SDL_QUIT){
			break;
			thread_exit=1;
		}else if(event.type==SFM_BREAK_EVENT){
			break;
		}

	}

	sws_freeContext(img_convert_ctx);

	SDL_Quit();
	//--------------
	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}

