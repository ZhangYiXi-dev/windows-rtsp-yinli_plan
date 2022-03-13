#include <iostream>
#include <WinSock2.h>
#include <windows.h>
#include <stdio.h>
#include<conio.h>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "legacy_stdio_definitions.lib")
#define __STDC_CONSTANT_MACROS
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }
//引入头文件
extern "C"
{
#include "libavformat/avformat.h"
	//引入时间
#include "libavutil/time.h"
#include <libavutil/opt.h>
#include <libavutil/avutil.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>

}
//引入库
#pragma comment(lib,"avformat.lib")
//工具库，包括获取错误信息等
#pragma comment(lib,"avutil.lib")
//编解码的库
#pragma comment(lib,"avcodec.lib")

#pragma comment(lib,"avdevice.lib")
#pragma comment(lib,"swscale.lib")
bool Mouse_Flag = 0;
int MOVE_STEP = 15;
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
		Sleep(2);
 		char recvBuff[1024] = {0};
		if (recv(socketClient, recvBuff, sizeof(recvBuff), 0) <= 0)
		{
			std::cout << "recv error";
			break;
		}
		int s = atoi(recvBuff);

		if (Mouse_Flag == 0)
		{
		    if (s == 7777)
		    {
				Mouse_Flag = 1;
			}
			if ((s >= 65) && (s <= 90))
			{
				keybd_event(VK_SHIFT, 0, 0, 0);
				keybd_event(s, 0, 0, 0);
				Sleep(20);
				keybd_event(s, 0, KEYEVENTF_KEYUP, 0);
				keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);

			}
			else if ((s >= 97) && (s <= 122))
			{
				keybd_event(s - 32, 0, 0, 0);
				Sleep(20);
				keybd_event(s - 32, 0, KEYEVENTF_KEYUP, 0);
			}
			else if (s == 1216)
			{
				keybd_event(VK_SHIFT, 0, 0, 0);
				keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
			}
			
			else
			{
				keybd_event(s, 0, 0, 0);
				Sleep(20);
				keybd_event(s, 0, KEYEVENTF_KEYUP, 0);
			}
		}
		else
		{
			if (s == 7777)
			{
				Mouse_Flag = 0;
			}
			if (s==87) //上
			{
				mouse_event(MOUSEEVENTF_MOVE, 0, -MOVE_STEP, 0, 0);
			}
			else if (s == 83)
			{
				mouse_event(MOUSEEVENTF_MOVE, 0, MOVE_STEP, 0, 0);
			}
			else if (s == 65)
			{
				mouse_event(MOUSEEVENTF_MOVE, -MOVE_STEP, 0, 0, 0);
			}
			else if (s == 68)
			{
				mouse_event(MOUSEEVENTF_MOVE, MOVE_STEP, 0, 0, 0);
			}
			else if (s == 13)
			{
				mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			}
			else if (s == 49)
			{
				if (MOVE_STEP >= 50)
				{
					continue;
				}
				MOVE_STEP += 5;
			}
			else if (s == 50)
			{
				if (MOVE_STEP <=5)
				{
					continue;
				}
				MOVE_STEP += 5;
			}
		}
	}

	closesocket(socketClient);

	WSACleanup();
}
int main(int argc, char* argv[])
{
	//创建线程,交互部分
	HANDLE hThread;
	DWORD dwThreadId;

	hThread = CreateThread(NULL	// 默认安全属性
		, NULL		// 默认堆栈大小
		, ThreadProFunc // 线程入口地址
		, NULL	//传递给线程函数的参数
		, 0		// 指定线程立即运行
		, &dwThreadId	//线程ID号
	);

	//推流部分
	AVFormatContext* pInputFormatContext = NULL;
	AVCodec* pInputCodec = NULL;
	AVCodecContext* pInputCodecContex = NULL;

	AVFormatContext* pOutputFormatContext = NULL;
	AVCodecContext* pOutCodecContext = NULL;
	AVCodec* pOutCodec = NULL;
	AVStream* pOutStream = NULL;

	av_register_all();

	avformat_network_init();

	avdevice_register_all();

	//const char* out_file = "rtmp://121.5.5.221:1935/rtmp_live/zyx";
	const char* out_file = "rtsp://121.5.5.221:8554/zyx";   //推流地址
	int ret, i;
	int videoindex = -1;
	//输入（Input） 抓屏部分设置
	pInputFormatContext = avformat_alloc_context();
	AVDictionary* options = NULL;
	AVInputFormat* ifmt = av_find_input_format("gdigrab");
	av_dict_set(&options, "framerate", "25", 0);
	av_dict_set(&options,"video_size","1920x1080",0);
	av_dict_set(&options, "start_time_realtime", 0, 0);
	if (avformat_open_input(&pInputFormatContext, "desktop", ifmt, &options) != 0) { //Grab at position 10,20 真正的打开文件,这个函数读取文件的头部并且把信息保存到我们给的AVFormatContext结构体中
		printf("Couldn't open input stream.\n");
		getchar();
		return -1;
	}
	if ((ret = avformat_find_stream_info(pInputFormatContext, 0)) < 0) {
		printf("Failed to retrieve input stream information");
		getchar();
		return -1;
	}
	av_dump_format(pInputFormatContext, 0, "desktop", 0);
	for (i = 0; pInputFormatContext->nb_streams; i++)
		if (pInputFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			cout << "pInputFormatContext=" << pInputFormatContext->streams[i]->codec->bit_rate << endl;
			videoindex = i;
			break;
		}

	//输入流解码器初始化
	pInputCodecContex = pInputFormatContext->streams[videoindex]->codec;
	pInputCodecContex->flags |= 0x00080000;
	pInputCodec = avcodec_find_decoder(pInputCodecContex->codec_id);
	if (pInputCodec == NULL)
	{
		printf("Codec not found.\n");
		getchar();
		return -1;
	}
	//打开解码器
	if (avcodec_open2(pInputCodecContex, pInputCodec, NULL) < 0)
	{
		printf("Could not open codec.\n");
		getchar();
		return -1;
	}
	//为一帧图像分配内存
	AVFrame* pFrame;
	AVFrame* pFrameYUV;
	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();//为转换来申请一帧的内存(把原始帧->YUV)
	pFrameYUV->format = AV_PIX_FMT_YUV420P;
	pFrameYUV->width = pInputCodecContex->width;
	pFrameYUV->height = pInputCodecContex->height;
	unsigned char* out_buffer = (unsigned char*)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pInputCodecContex->width, pInputCodecContex->height));
	//现在我们使用avpicture_fill来把帧和我们新申请的内存来结合
	avpicture_fill((AVPicture*)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pInputCodecContex->width, pInputCodecContex->height);
	struct SwsContext* img_convert_ctx;
	img_convert_ctx = sws_getContext(pInputCodecContex->width, pInputCodecContex->height, pInputCodecContex->pix_fmt, pInputCodecContex->width, pInputCodecContex->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	//=============================================================================================
	//输出流配置
	avformat_alloc_output_context2(&pOutputFormatContext, NULL, "rtsp", out_file); //RTMP  定义一个输出流信息的结构体
	if (!pOutputFormatContext) {
		printf("Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		getchar();
		return -1;
	}
	av_opt_set(pOutputFormatContext->priv_data, "rtsp_transport", "tcp", 0);
	//输出流编码器配置
	pOutCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!pOutCodec) {
		printf("Can not find encoder! \n");
		getchar();
		return -1;
	}
	pOutCodecContext = avcodec_alloc_context3(pOutCodec);
	//pOutCodecContext->flags |= 0x00080000;
	//像素格式,
	pOutCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
	//size
	pOutCodecContext->width = pInputCodecContex->width;
	pOutCodecContext->height = pInputCodecContex->height;
	//目标码率
	pOutCodecContext->bit_rate = 4000000;
	//每250帧插入一个I帧,I帧越小视频越小
	pOutCodecContext->gop_size = 10;
	//Optional Param B帧
	pOutCodecContext->max_b_frames = 0;  //设置B帧为0，则DTS与PTS一致

	pOutCodecContext->time_base.num = 1;
	pOutCodecContext->time_base.den = 25;

	//pOutCodecContext->lmin = 1;
	//pOutCodecContext->lmax = 50;
	//最大和最小量化系数
	pOutCodecContext->qmin = 10;
	pOutCodecContext->qmax = 51;
	if (pOutputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
		pOutCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	//avcodec_parameters_from_context(pOutStream->codecpar, pInputCodecContex);

	AVDictionary* param = 0;
	//    //H.264
	//av_opt_set(pOutCodecContext->priv_data, "preset", "superfast", 0);
	av_opt_set(pOutCodecContext->priv_data, "tune", "zerolatency", 0);//实时编码


	if (avcodec_open2(pOutCodecContext, pOutCodec, &param) < 0)
	{
		printf("Failed to open encoder! \n");
		getchar();
		return -1;
	}
	pOutStream = avformat_new_stream(pOutputFormatContext, pOutCodec);
	if (pOutStream == NULL)
	{
		printf("Failed create pOutStream!\n");
		getchar();
		return -1;
	}
	pOutStream->time_base.num = 1;
	pOutStream->time_base.den = 25;
	pOutStream->codec = pOutCodecContext;
	
	//写文件头
	int r = avformat_write_header(pOutputFormatContext, NULL);
	if (r < 0)
	{
		printf("Failed write header!\n");
		getchar();
		return -1;
	}

	AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
	int got_picture;

	AVPacket pkt;
	int picture_size = avpicture_get_size(pOutCodecContext->pix_fmt, pOutCodecContext->width, pOutCodecContext->height);
	av_new_packet(&pkt, picture_size);

	int frame_index = 0;
	while ((av_read_frame(pInputFormatContext, packet)) >= 0)
	{
		printf("to av_read_frame! \n");
		if (packet->stream_index == videoindex)
		{

			//真正解码,packet to pFrame
			 avcodec_decode_video2(pInputCodecContex, pFrame, &got_picture, packet);
			//printf("真正解码,packet to pFrame! \n%d", got_picture);
			if (got_picture)
			{
				sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pInputCodecContex->height, pFrameYUV->data, pFrameYUV->linesize);
				pFrameYUV->pts = frame_index;
				
				int picture;
				//真正编码pkt
				int ret = avcodec_encode_video2(pOutCodecContext, &pkt, pFrameYUV, &picture);
				//pkt.pts = frame_index;
				//pkt.dts = pkt.pts;
				if (ret < 0) {
					printf("Failed to encode! \n");
					getchar();
					return -1;
				}
				if (picture == 1)
				{
					pkt.stream_index = pOutStream->index;
					AVRational time_base = pOutStream->time_base;//{ 1, 1000 };
					AVRational r_framerate1 = { 50, 2 };//{ 50, 2 };
					int64_t calc_pts = (double)frame_index * (AV_TIME_BASE) * (1 / av_q2d(r_framerate1));
					pkt.pts = av_rescale_q(calc_pts, { 1, AV_TIME_BASE }, time_base);  //enc_pkt.pts = (double)(framecnt*calc_duration)*(double)(av_q2d(time_base_q)) / (double)(av_q2d(time_base));
					pkt.dts = pkt.pts;
					frame_index++;
					ret = av_interleaved_write_frame(pOutputFormatContext, &pkt);
					//av_dump_format(pOutputFormatContext, 0, out_file, 1);

					if (ret < 0) {
						printf("Error muxing packet\n");
						break;
					}

					av_free_packet(&pkt);
				}
			}
		}
		av_free_packet(packet);
	}

	//Write file trailer
	av_write_trailer(pOutputFormatContext);

	sws_freeContext(img_convert_ctx);
	//fclose(fp_yuv);
	av_free(out_buffer);
	av_free(pFrameYUV);
	av_free(pFrame);
	avcodec_close(pInputCodecContex);
	avformat_close_input(&pInputFormatContext);

	avcodec_close(pOutStream->codec);
	av_free(pOutCodec);
	avcodec_close(pOutCodecContext);
	avformat_free_context(pOutputFormatContext);


	return 0;
}

void Input_decode_init()
{

}