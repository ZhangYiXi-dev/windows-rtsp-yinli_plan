#include<unistd.h>
#include<fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#ifndef _WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif
#include <sys/time.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <iostream>
#include <pthread.h>
#include<cstring>
#include <ctime>
using namespace std;
pthread_mutex_t mutex;
int count_sum = 0;
struct bufferevent* usr_info[2];
int usr_num = 0;
static const char MESSAGE[] = "Hello, World!\n";

static const int PORT = 12160;

static void listener_cb(struct evconnlistener*, evutil_socket_t,
struct sockaddr*, int socklen, void*);
static void conn_writecb(struct bufferevent*, void*);
static void conn_readcb(struct bufferevent*, void*);
static void conn_eventcb(struct bufferevent*, short, void*);
static void signal_cb(evutil_socket_t, short, void*);

int main(int argc, char** argv)
{
	int mu = pthread_mutex_init(&mutex, NULL);
	struct event_base* base;
	struct evconnlistener* listener;
	struct event* signal_event;

	struct sockaddr_in sin;
#ifdef _WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
#endif

	base = event_base_new();
	if (!base) {
		fprintf(stderr, "Could not initialize libevent!\n");
		return 1;
	}
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);

	listener = evconnlistener_new_bind(base, listener_cb, (void*)base,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
		(struct sockaddr*)&sin,
		sizeof(sin));

	if (!listener) {
		fprintf(stderr, "Could not create a listener!\n");
		return 1;
	}

	signal_event = evsignal_new(base, SIGINT, signal_cb, (void*)base);

	if (!signal_event || event_add(signal_event, NULL) < 0) {
		fprintf(stderr, "Could not create/add a signal event!\n");
		return 1;
	}

	event_base_dispatch(base);

	evconnlistener_free(listener);
	event_free(signal_event);
	event_base_free(base);

	printf("done\n");
	return 0;
}

static void
listener_cb(struct evconnlistener* listener, evutil_socket_t fd,
	struct sockaddr* sa, int socklen, void* user_data)
{
	struct event_base* base = (struct event_base*)user_data;
	struct bufferevent* bev;
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev) {
		fprintf(stderr, "Error constructing bufferevent!");
		event_base_loopbreak(base);
		return;
	}
	usr_info[usr_num++] = bev;
	char* ip=new char;
	sockaddr_in sin;
	memcpy(&sin, sa, sizeof(sin));
	sprintf(ip, inet_ntoa(sin.sin_addr));
	bufferevent_setcb(bev, conn_readcb, conn_writecb, conn_eventcb,ip);
	bufferevent_enable(bev, EV_WRITE);
	bufferevent_enable(bev, EV_READ);
}
static void
conn_readcb(struct bufferevent* bev, void* user_data)
{
	char* ip = (char*)user_data;
	char buf[1024]={0};
	int fd;
	struct timeval us;
	struct tm t;
	char date_time[64];
	char gid_time[64];
	int len = bufferevent_read(bev, buf, sizeof(buf));
	//const char *w="OK";
	//bufferevent_write(bev, w, strlen(w)+1);
	if (bev == usr_info[0])
	{
		bufferevent_write(usr_info[1], buf, len);
	}
	else
	{
		bufferevent_write(usr_info[0], buf, len);
	}
	//д�ļ�
	// bufferevent_free(bev);
}
static void
conn_writecb(struct bufferevent* bev, void* user_data)
{

}

static void
conn_eventcb(struct bufferevent* bev, short events, void* user_data)
{
	if (events & BEV_EVENT_EOF) {
		printf("Connection closed.\n");
	}
	else if (events & BEV_EVENT_ERROR) {
		printf("Got an error on the connection: %s\n",
			strerror(errno));/*XXX win32*/
	}
	/* None of the other events can happen here, since we haven't enabled
	 * timeouts */
	bufferevent_free(bev);
}

static void
signal_cb(evutil_socket_t sig, short events, void* user_data)
{
	struct event_base* base = (struct event_base*)user_data;
	struct timeval delay = { 2, 0 };

	printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

	event_base_loopexit(base, &delay);
}
