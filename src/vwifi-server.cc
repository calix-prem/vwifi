#include <iostream> // cout

#include <string.h> // strcmp

#include "config.h"
#include "cwifiserver.h"
#include "cctrlserver.h"
#include "cselect.h"
#include "cdynbuffer.h"

using namespace std;

CDynBuffer Buffer; // Buffer to stock received values

void ForwardData(CWifiServer* serverMaster, bool masterSendToOwnClients, CWifiServer* serverSecond, bool serverSecondForwardWithoutLoss, CWifiServer* serverThird, bool serverThirdIsWithoutCoordinate, CSelect* scheduler)
{
	TDescriptor socket;

	int valread;
	TPower power;

	for ( TIndex i = 0 ; i < serverMaster->GetNumberClient() ; )
	{
		socket = (*serverMaster)[i];

		if( ! serverMaster->IsEnable(i) )
		{
			//Somebody disconnected

			//Close the socket
			if( masterSendToOwnClients )
			{
				cout<<"Client disconnected : "; serverMaster->ShowInfoWifi(i) ; cout<<endl;
			}
			else
			{	// if masterSendToOwnClients is false, it is surely Spy
				cout<<"Spy disconnected"<<endl;
			}
			serverMaster->CloseClient(i);

			//del master socket to set
			scheduler->DelNode(socket);

			continue;
		}

		if( scheduler->DescriptorHasAction(socket) )
		{
			//Check if it was for closing , and also read the
			//incoming message

			// read the power
			valread = serverMaster->Read( socket , (char*)&power, sizeof(power));
			if ( valread <= 0 )
			{
				//Close the socket
				if( masterSendToOwnClients )
				{
					cout<<"Client disconnected : "; serverMaster->ShowInfoWifi(i) ; cout<<endl;
				}
				else
				{	// if masterSendToOwnClients is false, it is surely Spy
					cout<<"Spy disconnected"<<endl;
				}
				serverMaster->CloseClient(i);

				//del master socket to set
				scheduler->DelNode(socket);

				continue;
			}

			// read the data
			valread = serverMaster->ReadBigData( socket , &Buffer);
			if ( valread <= 0 )
			{
				//Close the socket
				cout<<"Client disconnected : "; serverMaster->ShowInfoWifi(i) ; cout<<endl;
				serverMaster->CloseClient(i);

				//del master socket to set
				scheduler->DelNode(socket);

				continue;
			}

			if( masterSendToOwnClients )
				if( serverMaster->GetNumberClient() > 1 )
				{
#ifdef _VERBOSE2
					cout<<"Server "<<serverMaster->GetPort()<<" forward "<<valread<<" bytes from own client "; serverMaster->ShowInfoWifi(i); cout<<" to "<< serverMaster->GetNumberClient()-1 << " others owns clients" <<endl;
#endif
					serverMaster->SendAllOtherClients(i,power,Buffer.GetBuffer(),valread);
				}

			if( serverSecond->GetNumberClient() > 0 )
			{
				if( serverSecondForwardWithoutLoss )
				{
#ifdef _VERBOSE2
					cout<<"Server "<<serverSecond->GetPort()<<" forward "<<valread<<" bytes from client "; serverMaster->ShowInfoWifi(i); cout<<" of "<< serverMaster->GetPort() << " to all "<< serverSecond->GetNumberClient() << " clients without loss" <<endl;
#endif
					serverSecond->SendAllClientsWithoutLoss(power,Buffer.GetBuffer(),valread);
				}
				else
				{
#ifdef _VERBOSE2
					cout<<"Server "<<serverSecond->GetPort()<<" forward "<<valread<<" bytes from client "; serverMaster->ShowInfoWifi(i); cout<<" of "<< serverMaster->GetPort() << " to all "<< serverSecond->GetNumberClient() << " clients" <<endl;
#endif
					CCoordinate coo=*(serverMaster->GetReferenceOnInfoWifiByIndex(i));
					serverSecond->SendAllClients(coo,power,Buffer.GetBuffer(),valread);
				}
			}

			if( serverThird->GetNumberClient() > 0 )
			{
				if( serverThirdIsWithoutCoordinate )
				{
#ifdef _VERBOSE2
					cout<<"Server "<<serverThird->GetPort()<<" forward "<<valread<<" bytes from client "; serverMaster->ShowInfoWifi(i); cout<<" of "<< serverMaster->GetPort() << " to all "<< serverThird->GetNumberClient() << " clients without loss" <<endl;
#endif
					serverThird->SendAllClientsWithoutLoss(power,Buffer.GetBuffer(),valread);
				}
				else
				{
#ifdef _VERBOSE2
					cout<<"Server "<<serverThird->GetPort()<<" forward "<<valread<<" bytes from client "; serverMaster->ShowInfoWifi(i); cout<<" of of "<< serverMaster->GetPort() << " to all "<< serverSecond->GetNumberClient() << " clients" <<endl;
#endif
					CCoordinate coo=*(serverMaster->GetReferenceOnInfoWifiByIndex(i));
					serverThird->SendAllClients(coo,power,Buffer.GetBuffer(),valread);
				}
			}
		}

		i++;
	}
}

int vwifi_server(TPort port_number, TPort spy_port_number, TPort ctrl_port_number)
{
	TDescriptor socket;

	CSelect scheduler;

	TPort vsock_port = WIFI_CLIENT_PORT_VHOST;
	TPort tcp_port = WIFI_CLIENT_PORT_INET;

	if (0 != port_number)
	{
		vsock_port = port_number;
		tcp_port = port_number;
	}

	if (0 == spy_port_number)
	{
		spy_port_number = WIFI_SPY_PORT;
	}

	if (0 == ctrl_port_number)
	{
		ctrl_port_number = CTRL_PORT;
	}

	CWifiServer wifiGuestVHostServer(AF_VSOCK);
	cout<<"CLIENT VHOST : ";
	wifiGuestVHostServer.Init(vsock_port);
	if( ! wifiGuestVHostServer.Listen(WIFI_MAX_DECONNECTED_CLIENT) )
	{
		cerr<<"Error : wifiGuestVHostServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	CWifiServer wifiGuestINETServer(AF_INET);
	cout<<"CLIENT TCP : ";
	wifiGuestINETServer.Init(tcp_port);
	if( ! wifiGuestINETServer.Listen(WIFI_MAX_DECONNECTED_CLIENT) )
	{
		cerr<<"Error : wifiGuestINETServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	cout<<"SPY : ";
	CWifiServer wifiSpyServer(AF_INET);
	wifiSpyServer.SetPacketLoss(false);
	wifiSpyServer.Init(spy_port_number);
	if( ! wifiSpyServer.Listen(1) )
	{
		cerr<<"Error : wifiSpyServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	cout<<"CTRL : ";
	CCTRLServer ctrlServer(&wifiGuestVHostServer, &wifiGuestINETServer, &wifiSpyServer,&scheduler);
	ctrlServer.Init(ctrl_port_number);
	if( ! ctrlServer.Listen() )
	{
		cerr<<"Error : ctrlServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	cout<<"Size of disconnected : "<<WIFI_MAX_DECONNECTED_CLIENT<<endl;

	if( wifiGuestVHostServer.CanLostPackets() )
		cout<<"Packet loss : Enable"<<endl;
	else
		cout<<"Packet loss : disable"<<endl;

	//add master socket to set
	scheduler.AddNode(wifiGuestVHostServer);
	scheduler.AddNode(wifiGuestINETServer);
	scheduler.AddNode(wifiSpyServer);
	scheduler.AddNode(ctrlServer);

	while( true )
	{
		//wait for an activity on one of the sockets , timeout is NULL ,
		//so wait indefinitely
		if( scheduler.Wait() == SCHEDULER_ERROR )
		{
			cerr<<"Error : scheduler.Wait"<<endl;
			return 1;
		}
		else {

			//If something happened on the master socket ,
			//then its an incoming connection
			if( scheduler.DescriptorHasAction(wifiGuestVHostServer) )
			{
				socket = wifiGuestVHostServer.Accept();
				if ( socket == SOCKET_ERROR )
				{
					cerr<<"Error : wifiGuestVHostServer.Accept"<<endl;
					exit(EXIT_FAILURE);
				}

				//add child sockets to set
				scheduler.AddNode(socket);

				//inform user of socket number - used in send and receive commands
				cout<<"New connection from Client VHost : "; wifiGuestVHostServer.ShowInfoWifi(wifiGuestVHostServer.GetNumberClient()-1) ; cout<<endl;
			}

			if( scheduler.DescriptorHasAction(wifiGuestINETServer) )
			{
				socket = wifiGuestINETServer.Accept();
				if ( socket == SOCKET_ERROR )
				{
					cerr<<"Error : wifiGuestINETServer.Accept"<<endl;
					exit(EXIT_FAILURE);
				}

				//add child sockets to set
				scheduler.AddNode(socket);

				//inform user of socket number - used in send and receive commands
				cout<<"New connection from Client TCP : "; wifiGuestINETServer.ShowInfoWifi(wifiGuestINETServer.GetNumberClient()-1) ; cout<<endl;
			}

			if( scheduler.DescriptorHasAction(wifiSpyServer) )
			{
				socket = wifiSpyServer.Accept();
				if ( socket == SOCKET_ERROR )
				{
					cerr<<"Error : wifiSpyServer.Accept"<<endl;
					exit(EXIT_FAILURE);
				}

				//add child sockets to set
				scheduler.AddNode(socket);

				//inform user of socket number - used in send and receive commands
				cout<<"New connection from Spy"<<endl;
			}

			if( scheduler.DescriptorHasAction(ctrlServer) )
			{
				ctrlServer.ReceiveOrder();
			}

			//else its some IO operation on some other socket

			ForwardData(&wifiGuestVHostServer, true, &wifiGuestINETServer, false, &wifiSpyServer, true, &scheduler);
			ForwardData(&wifiGuestINETServer, true, &wifiGuestVHostServer, false, &wifiSpyServer, true, &scheduler);

			ForwardData(&wifiSpyServer, false, &wifiGuestVHostServer, true, &wifiGuestINETServer, false, &scheduler);

		}
	}

	return 0;
}

static const char usage[] = "Usage: vwifi-server [-h] [-p PORT] [-s SPY_PORT] [-c CTRL_PORT]";

int main(int argc, char** argv)
{
	int arg_idx = 1;
	TPort port_number = 0;
	TPort spy_port_number = 0;
	TPort ctrl_port_number = 0;

	while (arg_idx < argc)
	{
		if( ! strcmp("-v", argv[1]) || ! strcmp("--version", argv[1]) )
		{
			cout<<"Version : "<<VERSION<<endl;
			return 0;
		}
		else if( ! strcmp("-p", argv[arg_idx]) && (arg_idx + 1) < argc)
		{
			port_number = std::stoi(argv[arg_idx+1]);
			arg_idx++;
		}
		else if( ! strcmp("-s", argv[arg_idx]) && (arg_idx + 1) < argc)
		{
			spy_port_number = std::stoi(argv[arg_idx+1]);
			arg_idx++;
		}
		else if( ! strcmp("-c", argv[arg_idx]) && (arg_idx + 1) < argc)
		{
			ctrl_port_number = std::stoi(argv[arg_idx+1]);
			arg_idx++;
		}
		else if( ! strcmp("-h", argv[arg_idx]) )
		{
			std::cerr<<usage<<std::endl;
			return 1;
		}
		else
		{
			std::cerr<<"Error : unknown parameter"<<std::endl;
			std::cerr<<usage<<std::endl;
			return 1;
		}

		arg_idx++;
	}

	return vwifi_server(port_number, spy_port_number, ctrl_port_number);
}

