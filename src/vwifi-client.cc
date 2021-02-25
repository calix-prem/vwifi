#include "cwificlient.h"

#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <string.h> // strcmp

#include <memory>

enum STATE {

	STARTED=1,
	STOPPED ,
	SUSPENDED 
};

CKernelWifi* wifiClient;

enum STATE  _state = STOPPED ;


void  signal_handler(int signal_num)
{

	switch(signal_num)
	{
		case SIGINT :
		case SIGTERM :
		case SIGQUIT :
			
			std::cout << signal_num << std::endl ;
			wifiClient->stop() ;
			_state = STOPPED ;
			break ;

		case SIGTSTP:

			std::cout << "This signal is ignored" << std::endl ;
			break ;
		

		default :
			std::cerr << "Signal not handled" << std::endl ;
	
	}

	std::cout << "OUT SWITCH" << std::endl ;
}

static const char usage[] = "Usage: vwifi-client [-h] [-v|--version] [-i IP_ADDR] [-p PORT]";

int main (int argc , char ** argv){

	/* Handle signals */
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGHUP, SIG_IGN);
	signal(SIGTSTP, signal_handler);
	//signal(SIGCONT, signal_handler);

	int arg_idx = 1;
	std::string ip_addr;
	TPort port_number = 0;

	while (arg_idx < argc)
	{
		if( ! strcmp("-v", argv[arg_idx]) || ! strcmp("--version", argv[arg_idx]) )
		{
			std::cout<<"Version : "<<VERSION<<std::endl;
			return 0;
		}
		else if( ! strcmp("-i", argv[arg_idx]) && (arg_idx + 1) < argc)
		{
			ip_addr = std::string(argv[arg_idx+1]);
			arg_idx++;
		}
		else if( ! strcmp("-p", argv[arg_idx]) && (arg_idx + 1) < argc)
		{
			port_number = std::stoi(argv[arg_idx+1]);
			arg_idx++;
		}
		else if( ! strcmp("-h", argv[arg_idx]) )
		{
			std::cerr<<usage<<std::endl;
			return 1;
		}
		else
		{
			std::cerr<<"Error : too many parameters"<<std::endl;
			std::cerr<<usage<<std::endl;
			return 1;
		}

		arg_idx++;
	}

	if( port_number == 0 )
	{
		if ( ip_addr.empty() )
		{
			port_number = WIFI_CLIENT_PORT_VHOST;
		}
		else
		{
			port_number = WIFI_CLIENT_PORT_INET;
		}
	}

	if( ! ip_addr.empty() )
	{

		wifiClient=new CWifiClient<CSocketClientINET>;
		((CWifiClient<CSocketClientINET>*)wifiClient)->Init(ip_addr.c_str(), port_number);
	}
	else
	{
		wifiClient=new CWifiClient<CSocketClientVHOST>;
		((CWifiClient<CSocketClientVHOST>*)wifiClient)->Init(port_number);
	}

	if(!wifiClient->start())
		std::cout << "Starting process aborted" << std::endl ;



	std::cout << "Good Bye (:-)" << std::endl ; 

	_exit(EXIT_SUCCESS);

}
