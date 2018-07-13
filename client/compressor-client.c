#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "../utility.h"

int main (int argc, const char * argv[]) 
{
	printf("REMOTE COMPRESSOR client\n");
	struct sockaddr_in srv_addr;
	int ret,sk;
	
	if(strlen(argv[1])>15 || strlen(argv[2])>5)		//prevenzione buffer overflow
	{
		printf("Parametri formalmente invalidi\n");
		return(1);	
	}
	
	sk=socket(PF_INET,SOCK_STREAM,0);				//Inizializzo il socket
	memset(&srv_addr,0,sizeof(srv_addr));
	srv_addr.sin_family=AF_INET;
	srv_addr.sin_port=htons((int)atol(argv[2]));
	ret=inet_pton(AF_INET,argv[1],&srv_addr.sin_addr);
	
	ret=connect(sk,(struct sockaddr*) &srv_addr,sizeof(srv_addr));
    
    if(ret==0)										//Controllo connessione
    {
    	printf("Connesso al server\n");	
    }
    else
    {
    	printf("Impossibile connettersi al server, termino\n");
    	return 1;	
    }
	char comando[25];
	char argomento[50];
	
	printf("Pronto, digita help per visualizzare i comandi disponibili.\n");
	printf("ATTENZIONE: i comandi e gli argomenti non possono superare i 50 caratteri\n");
  
    for(;;)											//Menù di selezione
    {
    	printf("remote-compressor>");
    	scanf("%25s",comando);						//scanf con limite a 25 caratteri contro buffer overflow

    	if(strcmp(comando,"help")==0)
    	{
    		stampaHelp();
    		inviaMessaggio(sk,comando,strlen(comando));
    		continue;
    	}
    	if(strcmp(comando,"configure-compressor")==0)
    	{
    		scanf("%10s",argomento);
    		if(strcmp(argomento,"bzip2")==0 || strcmp(argomento,"gnuzip")==0)
    		{
    			inviaMessaggio(sk,comando,strlen(comando));
    			inviaMessaggio(sk,argomento,strlen(argomento));
    			printf("%s",riceviMessaggio(sk));	
    		}
    		else
    		{
    			printf("Valore dell'argomento errato, i valori possibili sono: bzip2 gnuzip\n");	
    		}
    		continue;
    	}
    	if(strcmp(comando,"configure-name")==0)
    	{
    		scanf("%50s",argomento);
    		inviaMessaggio(sk,comando,strlen(comando));
    		inviaMessaggio(sk,argomento,strlen(argomento));
    		printf("%s",riceviMessaggio(sk));
    		continue;
    	}
    	if(strcmp(comando,"show-configuration")==0)
    	{
    		inviaMessaggio(sk,comando,strlen(comando));
    		printf("Nome: %s\n",riceviMessaggio(sk));
    		printf("Compressore: %s\n",riceviMessaggio(sk));
    		continue;
    	}
    	if(strcmp(comando,"send")==0)
    	{
    		scanf("%50s",argomento);
    		if(invioFile(argomento,sk,comando)==2)
    		{
    			printf("Errore di scrittura file nel server\n");
    		}
    		continue;
    	}
    	if(strcmp(comando,"compress")==0)
    	{
    		char path[110];
    		scanf("%50s",argomento);
    		inviaMessaggio(sk,comando,strlen(comando));
    		if(strcmp(riceviMessaggio(sk),"NO")==0)
    		{
    			printf("Non ci sono file da comprimere, utilizza il comando send per inviare file al server\n");
    			continue;	
    		}
    		if(strcmp(argomento,".")==0)
    		{
    			strcpy(path,riceviMessaggio(sk));
    		}
    		else
    		{
    			strcpy(path,argomento);
    			strcat(path,riceviMessaggio(sk));	
    		}
    		int size=atoi(riceviMessaggio(sk));
    		if(scriviFile(path,sk,size)==1)
    		{
    			printf("Errore durante la scrittura del file inviato dal server\n");		
    		}
    		else
    		{
    			printf("Archivio %s ricevuto con successo\n",riceviMessaggio(sk));	
    		}
    		continue;
    	}
    	if(strcmp(comando,"quit")==0)
    	{
    		printf("Termino\n");
    		inviaMessaggio(sk,comando,strlen(comando));
    		close(sk);
    		return 0;	
    	}
    	printf("Comando non valido, digita help per visualizzare i comandi disponibili\n");
    }
    return 0;
}

