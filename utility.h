int len_list=0;						//lunghezza della stringa formata dai nomi di tutti i file

struct nomeFile
{
	char nome[50];
	struct nomeFile *next;	
};

void reverse(char s[])				//inverte l'ordine delle lettere di una stringa
{
	int i, j;
	char c;
	for (i = 0, j = strlen(s)-1; i<j; i++, j--) 
	{
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

void itoa(int n, char s[])					//Integer to ASCII
{
	int i, sign;
	if ((sign = n) < 0)  /* record sign */
		n = -n;          /* make n positive */
	i = 0;
	do        /* generate digits in reverse order */
	{
		s[i++] = n % 10 + '0';   /* get next digit */
	} 
	while ((n /= 10) > 0);     /* delete it */
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
}

char* riceviMessaggio(int sk)
{
	char buffer[1024];
	char lens[5];
	int ret=recv(sk,(void*)lens,5,0);
	int leni=atol(lens);
	ret=recv(sk,(void*)buffer,leni,0);
	char *msg=(char*) malloc(leni);
	int i;
	for(i=0;i<leni;i++)
	{
		msg[i]=buffer[i];
	}
	msg[leni]='\0';
	if(ret==-1 || ret<strlen(msg))
    {
    	printf("Errore nella ricezione dei dati\n");
    	return "NULL";
    }
	return msg;
}

void inviaMessaggio(int sk,char msg[],int nbyte)		//invia dimensione del dato e dato
{
	char lis[5];
	itoa(nbyte,lis);
	int ret=send(sk,(void*)lis,5,0);
	ret=send(sk,(void*)msg,nbyte,0);
    if(ret==-1 || ret<strlen(msg))
    {
    	printf("Errore nell'invio dei dati\n");
    }
    return;
}

char* estraiNomeFile(char path[])			//estrae il nome del file dal path
{
	char *temp=NULL;
	char *name;
	int i;
	int count=0;
	for(i=0;i<strlen(path);i++)				//conto il numero di slash del path
	{
		if(path[i]=='/')
		{
			count++;
		}
	}
	if(count==0)
	{
		return path;	
	}
	temp=strtok(path,"/");
	if(count==1)
	{
		return temp;
	}
	else
	{
		while(temp!=NULL)
		{
			temp=strtok(NULL,"/");
			if(temp!=NULL)
			{
				name=temp;
			}
		}
		return name;
	}
}

struct nomeFile *inserimentoInLista(struct nomeFile *lista,char nome[])
{
	struct nomeFile *p;
	struct nomeFile *temp;
	if(lista==NULL)
	{
		p=(struct nomeFile *)malloc(sizeof(struct nomeFile));
		strcpy(p->nome,nome);
		len_list=len_list+strlen(p->nome);
		p->next=NULL;
		return p;	
	}
	p=lista;
	if(strcmp(p->nome,nome)==0)			//verifico che in lista non ci siano elementi con lo stesso nome
	{
		return lista;
	}
	while(p->next!=NULL)
	{
		if(strcmp(p->nome,nome)==0)
		{
			return lista;
		}
		p=p->next;
	}
	temp=(struct nomeFile *)malloc(sizeof(struct nomeFile));
	strcpy(temp->nome,nome);
	len_list=len_list+strlen(temp->nome);
	temp->next=NULL;
	p->next=temp;
	
	return lista;
}

char *stampaLista(struct nomeFile *lista)
{
	if(lista==NULL)
	{
		return "\0";	
	}

	struct nomeFile *p;

	char *listaCat=(char*) malloc(len_list);
	p=lista;
	while(p!=NULL)
	{
		strcat(listaCat,p->nome);
		strcat(listaCat," ");
		p=p->next;
	}
	return listaCat;
}

struct nomeFile *svuotaLista(struct nomeFile* lista)		//elimina i file dall'elenco e dall'hd
{
	char eliminazione[60];
	struct nomeFile *p=lista;
	while(lista!=NULL)
	{
		strcpy(eliminazione,"rm ");
		lista=lista->next;
		p->next=NULL;
		system(strcat(eliminazione,p->nome));
		free(p);
		p=lista;
	}
	return lista;
}

int invioFile(char pathFile[],int sk,char comando[])
{
	char csize[5];
	int ret,size;
	FILE *fp;
	fp=fopen(pathFile,"r");
	if(fp==NULL)
	{
		printf("Impossibile leggere il file\n");
		return 1;
	}
	struct stat info;
	ret=stat(pathFile,&info);
	size=info.st_size;				//grandezza del file in byte
	itoa(size,csize);
	if(strcmp(comando,"NULL")!=0)
	{
		inviaMessaggio(sk,comando,strlen(comando));
	};
	char path[50];
	strcpy(path,estraiNomeFile(pathFile));
    inviaMessaggio(sk,path,strlen(path));
    inviaMessaggio(sk,csize,strlen(csize));
    
    if(strcmp(riceviMessaggio(sk),"KO")==0)
    {
    	return 2;
    }
    
	while(!feof(fp)) 
	{
		char buffer[1024];
		ret=fread(buffer,1,sizeof(buffer)-1,fp);
		buffer[ret]='\0';
		inviaMessaggio(sk,buffer,ret);
	}
	printf("File %s inviato con successo\n",path);
	fclose(fp);
	return 0;
}

int scriviFile(char nome[],int sk,int size)
{
	FILE *fp;
	int ret,i;
	
	fp=fopen(nome,"w");
	if(fp==NULL)
	{
		inviaMessaggio(sk,"KO",2);		//Impossibile scrivere il file
		return 1;	
	}
	inviaMessaggio(sk,"OK",2);
	for(i=0;i!=size;)
	{
		int temp=size-(1023+i);
		if(temp<0)
		{
			temp=temp*(-1);
			ret=fwrite(riceviMessaggio(sk),1,size-i,fp);
			i=i+1023-temp;
			continue;	
		}
		ret=fwrite(riceviMessaggio(sk),1,1023,fp);
		i=i+1023;
	}
	fclose(fp);
	return 0;	
}

void stampaHelp()
{
	printf("Nome: compressor-client\n");
	printf("Sinossi: compressor-client <host-remoto> <porta>\n");
	printf("Descrizione: client per la compressione di file\n");
	printf("Comandi:\n\n");
	printf("help: visualizza questa guida.\n\n");
	printf("configure-compressor [compressor]: configura il server in modo da impostare l'algoritmo di compressione scelto (bzip2, gnuzip).\n\n");
	printf("configure-name [name]: imposta il nome dell'archivio che si vuole ricevere dal server.\n\n");
	printf("show-configuration: visualizza il nome dell'archivio e il compressore impostati.\n\n");
	printf("send [file]: invia il file al server per la compressione. Se vengono inviati piu' file con lo stesso nome verra' immesso nell'archivio solo l'ultimo file inviato al server.\n\n");
	printf("compress [path]: crea un file compresso con l'algoritmo specificato dal comando 'configure-compressor' e con il nome impostato tramite il comando 'configure-name'. Se nel path specificato e' presente un file con lo stesso nome ed estensione impostati nel programma esso sara' sovrascritto. Il file conterra' i file inviati al server. Il path deve terminare con il carattere '/' . Per utilizzare il path dal quale si esegue il client immettere come argomento '.'. \n\n");
	printf("quit: termina l'esecuzione del programma.\n\n");
	return;
}



//INIZIO SEZIONE DATI E METODI PER THREAD



//semaforo globale inizializzato
pthread_mutex_t request_mutex=PTHREAD_MUTEX_INITIALIZER;


//variabile condition globale
pthread_cond_t got_request=PTHREAD_COND_INITIALIZER;

int num_requests=0; //numero delle richieste pendenti

struct request		//struttura di una richiesta
{
	int socket;
	char ip[15];
	struct request* next;
};

struct request* list_requests=NULL;			//prima richiesta
struct request* last_request=NULL;			//ultima richiesta

void add_request(int request_num,char ip[],pthread_mutex_t* p_mutex,pthread_cond_t* p_cond_var)
{
	int rc;
	struct request* a_request;
	a_request=(struct request*)malloc(sizeof(struct request));
	if(!a_request)
	{
		printf("Errore\n");
		exit(1);	
	}
	a_request->socket=request_num;
	strcpy(a_request->ip,ip);
	a_request->next=NULL;
	
	//blocco il semaforo per assicurarmi accesso atomico alla lista
	rc=pthread_mutex_lock(p_mutex);
	
	if(num_requests==0)
	{
		list_requests=a_request;
		last_request=a_request;
	}
	else
	{
		last_request->next=a_request;
		last_request=a_request;
	}
	num_requests++;
	
	//sblocco il semaforo
	rc=pthread_mutex_unlock(p_mutex);
	
	//invio segnale alla variabile condition - c'è una nuova richiesta da gestire
	rc=pthread_cond_signal(p_cond_var);
}

struct request* get_request(pthread_mutex_t* p_mutex)
{
	int rc;
	struct request* a_request;
	
	rc=pthread_mutex_lock(p_mutex);
	
	if(num_requests>0)
	{
		a_request=list_requests;
		list_requests=a_request->next;
		if(list_requests==NULL)
		{
			last_request=NULL;
		}
		num_requests--;
	}
	else
	{
		a_request=NULL;
	}
	
	rc=pthread_mutex_unlock(p_mutex);
	
	return a_request;
}

void handle_request(struct request* a_request,int thread_id)
{
	char cread[10];
	char sid[3];
	itoa(thread_id,sid);
	strcpy(cread,"mkdir ");
	system(strcat(cread,sid));
	
	if(a_request)
	{
		printf("Thread %d sta gestendo la richiesta del client %s\n",thread_id,a_request->ip);
	}
	struct nomeFile *lista;
	lista=NULL;
	char compType[7];
	char confName[60];
	
	strcpy(compType,"gnuzip");
	strcpy(confName,"archivio");
	
	while(1)
	{	
		char *comando=riceviMessaggio(a_request->socket);		//il server attende un comando dal client
		int deleted=0;											//mi dice se i file utente sono già stati cancellati o no
			
		if(strcmp(comando,"help")==0)
		{
			printf("CLIENT %s eseguito comando help\n",a_request->ip);
			continue;
		}
			
		if(strcmp(comando,"configure-compressor")==0)
		{
			char *argomento=riceviMessaggio(a_request->socket);
			strcpy(compType,argomento);
			inviaMessaggio(a_request->socket,"Compressore configurato correttamente\n",strlen("Compressore configurato correttamente\n"));
			free(argomento);
			free(comando);
			printf("CLIENT %s eseguito comando configure-compressor %s\n",a_request->ip,compType);
			continue;	
		}
		if(strcmp(comando,"configure-name")==0)
		{
			strcpy(confName,riceviMessaggio(a_request->socket));
			inviaMessaggio(a_request->socket,"Nome configurato correttamente\n",strlen("Nome configurato correttamente\n"));
			free(comando);
			printf("CLIENT %s eseguito comando configure-name %s\n",a_request->ip,confName);
			continue;	
		}
		if(strcmp(comando,"show-configuration")==0)
		{
			inviaMessaggio(a_request->socket,confName,strlen(confName));
			inviaMessaggio(a_request->socket,compType,strlen(compType));
			printf("CLIENT %s eseguito comando show-configuration\n",a_request->ip);
			continue;	
		}
		if(strcmp(comando,"send")==0)
		{
			deleted=1;
			char nome[60];
			itoa(thread_id,nome);
			strcat(nome,"/");
			strcat(nome,riceviMessaggio(a_request->socket));
			int size=atoi(riceviMessaggio(a_request->socket));
			if(scriviFile(nome,a_request->socket,size)!=0)
			{
				strtok(nome,"/");
				strcpy(nome,strtok(NULL,"/"));
				printf("SERVER: errore durante la scrittura del file %s inviato dal client %s\n",nome,a_request->ip);
			}
			else
			{
				lista=inserimentoInLista(lista,nome);
				strtok(nome,"/");
				strcpy(nome,strtok(NULL,"/"));
				printf("SERVER: ricevuto il file %s dal client %s\n",nome,a_request->ip);
			}
			continue;	
		}
		if(strcmp(comando,"compress")==0)
		{
			if(lista==NULL)				//non ci sono file da comprimere
			{
				inviaMessaggio(a_request->socket,"NO",strlen("NO"));
				continue;
			}
			inviaMessaggio(a_request->socket,"SI",strlen("SI"));
			int i;
			printf("%d\n",len_list);
			char *comandi=(char*)malloc(sizeof(char)*(len_list+15));
			char *estensioni[]={".tgz ",".bz2 ",NULL};
			if(strcmp(compType,"gnuzip")==0)
			{
				strcpy(comandi,"tar -czf ");
				i=0;
			}
			else
			{
				strcpy(comandi,"tar -cjf ");
				i=1;
			}
			printf("SERVER: creazione del file compresso %s%s richiesta dal client %s\n",confName,estensioni[i],a_request->ip);
			system(strcat(strcat(strcat(comandi,confName),estensioni[i]),stampaLista(lista)));
			lista=svuotaLista(lista);
			if(invioFile(strncat(confName,estensioni[i],4),a_request->socket,"NULL")==0)
			{
				printf("SERVER: spedito il file %s richiesto dal client %s\n",confName,a_request->ip);
				inviaMessaggio(a_request->socket,confName,strlen(confName));
				strcpy(comandi,"rm ");
				system(strcat(comandi,confName));
				char *temp=strtok(confName,".");
				strcpy(confName,temp);
			}
			free(comandi);
			free(comando);
			deleted=1;
			continue;	
		}
		if(strcmp(comando,"quit")==0)
		{
			printf("SERVER: client %s disconnesso\n",a_request->ip);
			free(comando);
			if(deleted==0)
			{
				lista=svuotaLista(lista);
			}
			strcpy(cread,"rmdir ");
			system(strcat(cread,sid));
			close(a_request->socket);
			return;	
		}
    	printf("Il client ha chiuso la connessione in modo anomalo, termino\n");
    	if(deleted==0)
    	{
    		lista=svuotaLista(lista);
    	}
    	strcpy(cread,"rmdir ");
		system(strcat(cread,sid));
    	close(a_request->socket);
    	return;	
	}
}

void* handle_request_loop(void* data)
{
	int rc;
	struct request* a_request;
	int thread_id=*((int*)data);
	
	printf("Starting thread %d\n",thread_id);
	
	while(1)
	{
		rc=pthread_mutex_lock(&request_mutex);
		if(num_requests>0)
		{
			a_request=get_request(&request_mutex);
			if(a_request)
			{
				rc=pthread_mutex_unlock(&request_mutex);
				handle_request(a_request,thread_id);
				free(a_request);
			}
		}
		else
		{
			rc=pthread_cond_wait(&got_request, &request_mutex);
		}
	}
}

