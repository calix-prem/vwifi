#include <iostream> // cout

#include <string.h> //strlen

#include "config.h"
#include "csocketclient.h"
#include "types.h"
#include "ccoordinate.h" // CCoordinate
#include "cinfowifi.h"

using namespace std;

void Help(char* nameOfProg)
{
	cout<<nameOfProg<<" [order]"<<endl;
	cout<<" with [order] :"<<endl;
	cout<<"	ls"<<endl;
	cout<<"		- List the Clients"<<endl;
	cout<<"	set cid x y z"<<endl;
	cout<<"		- Change the coordinate of the Client with cid"<<endl;
	cout<<"	loss yes/no"<<endl;
	cout<<"		- loss yes : packets can be lost"<<endl;
	cout<<"		- loss no : no packets can be lost"<<endl;
	cout<<"	show"<<endl;
	cout<<"		- Display the status of loss and list of Clients"<<endl;
	cout<<"	status"<<endl;
	cout<<"		- Display the status of the configuration of vwifi-server"<<endl;
	cout<<"	distance cid1 cid2"<<endl;
	cout<<"		- Distance in meters between the Client with cid1 and the Client with cid2"<<endl;
	cout<<"	close"<<endl;
	cout<<"		- Close all the connections with Wifi Clients"<<endl;

	cout<<" -v or --version"<<endl;
	cout<<"	Display the version of "<<nameOfProg<<endl;
}

int AskList(TPort port_number)
{
	CSocketClientINET socket;

	socket.Init(ADDRESS_IP,port_number);

	if( ! socket.ConnectLoop() )
	{
		cerr<<"Error : ls : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_LIST;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : ls : socket.Send : order"<<endl;
		return 1;
	}

	TIndex number;
	err=socket.Read((char*)&number,sizeof(number));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : ls : socket.Read : number"<<endl;
		return 1;
	}

	CInfoWifi info;
	for(TIndex i=0; i<number;i++)
	{
		err=socket.Read((char*)&info,sizeof(info));
		if( err == SOCKET_ERROR )
		{
			cerr<<"Error : ls : socket.Read : CInfoWifi"<<endl;
			return 1;
		}
		cout<<info<<endl;
	}

	socket.Close();

	return 0;
}

int ChangeCoordinate(char *prog_name, TPort port_number, int argc, char *argv[])
{
	if( argc != 4 )
	{
			cerr<<"Error : set : the number of parameter is uncorrect"<<endl;
			Help(prog_name);
			return 1;
	}

	TCID cid=atoi(argv[0]);

	if( cid < TCID_GUEST_MIN )
	{
			cerr<<"Error : set : the CID must be greater than or equal to "<<TCID_GUEST_MIN<<endl;
			return 1;
	}

	TValue x=atoi(argv[1]);
	TValue y=atoi(argv[2]);
	TValue z=atoi(argv[3]);
	CCoordinate coo(x,y,z);

	cout<<cid<<" "<<coo<<" "<<endl;

	CSocketClientINET socket;

	socket.Init(ADDRESS_IP,port_number);

	if( ! socket.ConnectLoop() )
	{
		cerr<<"Error : set : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_CHANGE_COORDINATE;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : set : socket.Send : order"<<endl;
		return 1;
	}
	err=socket.Send((char*)&cid,sizeof(cid));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : set : socket.Send : cid"<<endl;
		return 1;
	}
	err=socket.Send((char*)&coo,sizeof(coo));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : set : socket.Send : "<<coo<<endl;
		return 1;
	}

	socket.Close();

	return 0;
}

int ChangePacketLoss(char *prog_name, TPort port_number, int argc, char *argv[])
{
	if( argc != 1 )
	{
			cerr<<"Error : loss : the number of parameter is uncorrect"<<endl;
			Help(prog_name);
			return 1;
	}

	int value;
	if ( ! strcmp(argv[0],"yes") )
		value=1;
	else if ( ! strcmp(argv[0],"no") )
		value=0;
	else
	{
			cerr<<"Error : loss : the value can only be \"yes\" or \"no\""<<endl;
			return 1;
	}

	CSocketClientINET socket;

	socket.Init(ADDRESS_IP,port_number);

	if( ! socket.ConnectLoop() )
	{
		cerr<<"Error : loss : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_PACKET_LOSS;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : loss : socket.Send : order"<<endl;
		return 1;
	}
	err=socket.Send((char*)&value,sizeof(value));
	if( err == SOCKET_ERROR )
	{
		if ( value  )
			cerr<<"Error : loss : socket.Send : yes"<<endl;
		else
			cerr<<"Error : loss : socket.Send : no"<<endl;

		return 1;
	}

	socket.Close();

	return 0;
}

int AskStatus(TPort port_number)
{
	CSocketClientINET socket;

	cout<<"CTRL : IP : "<<ADDRESS_IP<<endl;
	cout<<"CTRL : Port : "<<CTRL_PORT<<endl;

	socket.Init(ADDRESS_IP,port_number);

	if( ! socket.ConnectLoop() )
	{
		cerr<<"Error : status : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_STATUS;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Send : Order"<<endl;
		return 1;
	}

	bool loss;
	err=socket.Read((char*)&loss,sizeof(loss));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Read : Loss"<<endl;
		return 1;
	}
	cout<<"SRV : PacketLoss : ";
	if ( loss )
		cout<<"Enable"<<endl;
	else
		cout<<"Disable"<<endl;

	// VHOST

	TPort port;
	err=socket.Read((char*)&port,sizeof(port));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Read : Port VHOST"<<endl;
		return 1;
	}
	cout<<"SRV VHOST : Port : "<<port<<endl;

	TIndex size;
	err=socket.Read((char*)&size,sizeof(size));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Read : Size VHOST"<<endl;
		return 1;
	}
	cout<<"SRV VHOST : SizeOfDisconnected : "<<size<<endl;

	// INET

	err=socket.Read((char*)&port,sizeof(port));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Read : Port INET"<<endl;
		return 1;
	}
	cout<<"SRV INET : Port : "<<port<<endl;

	err=socket.Read((char*)&size,sizeof(size));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Read : Size INET"<<endl;
		return 1;
	}
	cout<<"SRV INET : SizeOfDisconnected : "<<size<<endl;

	// SPY

	bool spyIsConnected;
	err=socket.Read((char*)&spyIsConnected,sizeof(spyIsConnected));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Read : spyIsConnected"<<endl;
		return 1;
	}
	cout<<"SPY : ";
	if ( spyIsConnected )
		cout<<"Connected"<<endl;
	else
		cout<<"Disconnected"<<endl;

	socket.Close();

	return 0;
}

int AskShow(TPort port_number)
{
	CSocketClientINET socket;

	socket.Init(ADDRESS_IP,port_number);

	if( ! socket.ConnectLoop() )
	{
		cerr<<"Error : show : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_SHOW;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : show : socket.Send : Order"<<endl;
		return 1;
	}

	bool loss;
	err=socket.Read((char*)&loss,sizeof(loss));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : show : socket.Read : Loss"<<endl;
		return 1;
	}
	cout<<"PacketLoss : ";
	if ( loss )
		cout<<"Enable"<<endl;
	else
		cout<<"Disable"<<endl;

	bool spyIsConnected;
	err=socket.Read((char*)&spyIsConnected,sizeof(spyIsConnected));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : show : socket.Read : spyIsConnected"<<endl;
		return 1;
	}
	cout<<"Spy : ";
	if ( spyIsConnected )
		cout<<"Connected"<<endl;
	else
		cout<<"Disconnected"<<endl;

	socket.Close();

	cout<<"----------------"<<endl;

	return AskList(port_number);
}

int DistanceBetweenCID(char *prog_name, TPort port_number, int argc, char *argv[])
{
	if( argc != 2 )
	{
			cerr<<"Error : distance : the number of parameter is uncorrect"<<endl;
			Help(prog_name);
			return 1;
	}

	TCID cid1=atoi(argv[0]);

	if( cid1 < TCID_GUEST_MIN )
	{
			cerr<<"Error : distance : the CID 1 must be greater than or equal to "<<TCID_GUEST_MIN<<endl;
			return 1;
	}

	TCID cid2=atoi(argv[1]);

	if( cid2 < TCID_GUEST_MIN )
	{
			cerr<<"Error : distance : the CID2 must be greater than or equal to "<<TCID_GUEST_MIN<<endl;
			return 1;
	}

	CSocketClientINET socket;

	socket.Init(ADDRESS_IP,port_number);

	if( ! socket.ConnectLoop() )
	{
		cerr<<"Error : distance : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_DISTANCE_BETWEEN_CID;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : distance : socket.Send : order"<<endl;
		return 1;
	}
	err=socket.Send((char*)&cid1,sizeof(cid1));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : distance : socket.Send : cid 1"<<endl;
		return 1;
	}
	err=socket.Send((char*)&cid2,sizeof(cid2));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : distance : socket.Send : cid 2"<<endl;
		return 1;
	}

	int codeError;
	err=socket.Read((char*)&codeError,sizeof(codeError));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : distance : socket.Read : codeError"<<endl;
		return 1;
	}

	if ( codeError == -1 )
	{
		cerr<<"Error : distance : unknown cid 1 : "<<cid1<<endl;
		return 1;
	}
	if ( codeError == -2 )
	{
		cerr<<"Error : distance : unknown cid 2 : "<<cid2<<endl;
		return 1;
	}

	TDistance distance;
	err=socket.Read((char*)&distance,sizeof(distance));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : distance : socket.Read : distance"<<endl;
		return 1;
	}

	cout<<"Distance : "<<distance<<endl;

	socket.Close();

	return 0;
}

int CloseAllClient(TPort port_number)
{
	CSocketClientINET socket;

	socket.Init(ADDRESS_IP,port_number);

	if( ! socket.ConnectLoop() )
	{
		cerr<<"Error : close : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_CLOSE_ALL_CLIENT;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : close : socket.Send : order"<<endl;
		return 1;
	}

	socket.Close();

	return 0;
}

static const char usage[] = "Usage: vwifi-ctrl [-h] [-v|--version] [-p PORT]";

int main(int argc , char *argv[])
{
	int arg_idx = 1;
	TPort port_number = 0;

	while (arg_idx < argc)
	{
		if( ! strcmp("-v", argv[arg_idx]) || ! strcmp("--version", argv[arg_idx]) )
		{
			std::cout<<"Version : "<<VERSION<<std::endl;
			return 0;
		}
		else if( ! strcmp("-p", argv[arg_idx]) && (arg_idx + 1) < argc)
		{
			port_number = std::stoi(argv[arg_idx+1]);
			arg_idx++;
		}
		else if( ! strcmp("-h", argv[arg_idx]) )
		{
			Help(argv[0]);
			return 1;
		}
		else
		{
			std::cerr<<"Error : too many parameters"<<std::endl;
			Help(argv[0]);
			return 1;
		}

		arg_idx++;
	}

	if( ! strcmp(argv[arg_idx],"ls") )
		return AskList(port_number);

	if( ! strcmp(argv[arg_idx],"set") )
		return ChangeCoordinate(argv[0], port_number, argc, argv);

	if( ! strcmp(argv[arg_idx],"loss") )
		return ChangePacketLoss(argv[0], port_number, argc, argv);

	if( ! strcmp(argv[arg_idx],"show") )
		return AskShow(port_number);

	if( ! strcmp(argv[arg_idx],"status") )
		return AskStatus(port_number);

	if( ! strcmp(argv[arg_idx],"distance") )
		return DistanceBetweenCID(argv[0], port_number, argc, argv);

	if( ! strcmp(argv[arg_idx],"close") )
		return CloseAllClient(port_number);

	cerr<<argv[0]<<" : Error : unknown order : "<<argv[1]<<endl;

	return 1;
}

