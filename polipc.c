#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <newt.h>

int main(void)
{
 int socketout, socketin,start=0,count=0,i=0,rst;
 time_t tiempo;
 struct timeval espera;
 fd_set set;
 newtComponent formulario,componente[12];
 struct newtExitStruct salida;

 
 char mensaje [1024],maxspeed[30]; 	//for sending messages
 struct sockaddr_in direccion;
 struct ip_mreq mcaddr;
 
 //Aqui creo los sockets
 
 socketout =socket(AF_INET,SOCK_DGRAM,0);
 socketin  =socket(AF_INET,SOCK_DGRAM,0);
 
    newtInit();
    newtCls();
    newtPushHelpLine(" <Tab>/<Alt-Tab> switch   |   <Space> Slect  |   <F12> Exit");
    newtDrawRootText(0, 0, "Polip Internet Traffic Control");
    newtCenteredWindow(70,10,"Polip Speed");
    formulario=newtForm(NULL,NULL,0);
    componente[0]=newtLabel(5,2,"Kb/s");
    componente[1]=newtScale(5,4,60,10000);
    componente[2]=newtButton(45,6,"Start");
    componente[3]=newtButton(55,6,"Exit ");
    i=0;
    for (i=0;i<4;i++)
    newtFormAddComponent(formulario,componente[i]);

 // Socket para recibir los datos
 
 memset(&direccion,0,sizeof(direccion));
 direccion.sin_family =AF_INET;
 direccion.sin_addr.s_addr =inet_addr("230.0.0.2");
 direccion.sin_port =htons(20002);

 mcaddr.imr_multiaddr.s_addr = inet_addr("230.0.0.2");
 mcaddr.imr_interface.s_addr = htonl(INADDR_ANY);

 rst=setsockopt(socketin,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mcaddr,sizeof(mcaddr));
 rst=bind(socketin,(struct sockaddr *)&direccion,sizeof(direccion));
 if(rst==-1){
  newtWinMessage("PolipIP","OK", strerror(errno));
 }
 
 memset(&direccion,0,sizeof(direccion));
 direccion.sin_family =AF_INET;
 direccion.sin_addr.s_addr =inet_addr("230.0.0.1");
 direccion.sin_port =htons(20002);
  
 tiempo=time(NULL);
 
 while (1)
 {
  if(start==1){
	/*
	*If start is 1 get packets with select
	*Si Start es 1 voy a buscar paquetes con select
	*/
	espera.tv_sec=1;
	espera.tv_usec=0;
	FD_ZERO(&set);
	FD_SET(socketin,&set);
	
	rst=select(FD_SETSIZE,&set,NULL,NULL,&espera);
	
	if(rst==1){
		memset(mensaje,'\0',sizeof(mensaje));
		read(socketin,&mensaje,512);
		/*
		strstart=(strchr(strstr(mensaje,"192.168.1.6"),'/')+1);
		strncpy(speed,strstart,strcspn(strstart,"/"));
		newtScaleSet(componente[1],strtol(speed,NULL,10));
	        flota=strtod(speed,NULL);
	     	flota=flota/1024;
		sprintf(speed,"%5.2f Kb/s",flota);
		newtLabelSetText(componente[0],speed);
		memset(speed,'\0',sizeof(speed));
		*
		*/
		newtLabelSetText(componente[0],mensaje);
		count=0;
	}
	/*
	*Si select no recibe aumeton count si no recibo por 5 segundos mando otro start
	*/
	else if(rst==0){
		if(count>10){
			sprintf(mensaje,"Start");
			rst=sendto(socketout,mensaje,strlen(mensaje),0,(struct sockaddr *)&direccion,sizeof(direccion));
			newtLabelSetText(componente[0],"Disconnected!!!!!");
			count=0;
		}
		else
			count++;
	}
	
	/*
	Every 5 minuts send start for keepalive
	Cada 5 minutos mando un start para keepalive
	*/
	if (difftime(time(NULL),tiempo)>300){
		sprintf(mensaje,"Start");
		rst=sendto(socketout,mensaje,strlen(mensaje),0,(struct sockaddr *)&direccion,sizeof(direccion));
		if(rst==-1){
			newtWinMessage("PolipIP","OK", strerror(errno));
		}
		tiempo=time(NULL);
	}

	  
  }
      
    memset(&salida,0,sizeof(salida));
    /*
     Libera el Form luego de 1 segundos
     Realese the from after 1 sec
    */
    newtFormSetTimer(formulario,1000); 
    newtFormRun(formulario,&salida);
    
    
    if(salida.u.co==componente[3] || salida.reason==0)
    {
      sprintf(mensaje,"Stop");
      rst=sendto(socketout,mensaje,strlen(mensaje),0,(struct sockaddr*)&direccion,sizeof(direccion));
      if(rst==-1){
        newtWinMessage("PolipIP","OK", strerror(errno));
       }
     break;    
    }
    
    if(salida.u.co==componente[2])
    {
       if(start==0){
     	  sprintf(mensaje,"Start");
          rst=sendto(socketout,mensaje,strlen(mensaje),0,(struct sockaddr*)&direccion,sizeof(direccion));
      	  if(rst==-1){
		  newtWinMessage("PolipIP","OK", strerror(errno));
	  }

		  espera.tv_sec=5;
		  espera.tv_usec=0;
		  FD_ZERO(&set);
		  FD_SET(socketin,&set);
  
  		  rst=select(FD_SETSIZE,&set,NULL,NULL,&espera);
		  if(rst==1){
		  
		     read(socketin,&mensaje,512);
		     if(strchr(mensaje,' ')!=NULL)
		   	   strncpy(maxspeed,mensaje,strcspn(mensaje," "));

		     start=1;
	             newtFormDestroy(formulario);
     		     componente[0]=newtLabel(5,2,"Kb/s");
		     componente[1]=newtScale(5,4,60,strtol(maxspeed,NULL,10));
		     componente[2]=newtButton(45,6,"Stop");
		     componente[3]=newtButton(55,6,"Exit ");

     		     formulario=newtForm(NULL,NULL,0);
     		     i=0;
		     for (i=0;i<4;i++)
			   newtFormAddComponent(formulario,componente[i]);
		}
       }
       else if(start==1){
		 sprintf(mensaje,"Stop");
		 rst=sendto(socketout,mensaje,strlen(mensaje),0,(struct sockaddr *)&direccion,sizeof(direccion));
		 if(rst==-1){
			 newtWinMessage("PolipIP","OK", strerror(errno));
		  }
	          start=0;
	          newtFormDestroy(formulario);
     		  componente[0]=newtLabel(5,2,"Kb/s");
		  componente[1]=newtScale(5,4,60,10000);
		  componente[2]=newtButton(45,6,"Start");
		  componente[3]=newtButton(55,6,"Exit ");

     		  formulario=newtForm(NULL,NULL,0);
     		  i=0;
		  for (i=0;i<4;i++)
			newtFormAddComponent(formulario,componente[i]);
       }
    }
   }

 close (socketout);
 close (socketin);
 newtFinished();
 return 0;
}
