#include "utils.h"

void configure_logger() {
	LOGGER = log_create("logger.log","tp-lissandra",0,LOG_LEVEL_INFO);
	LOG_ERROR = log_create("log_error.log","tp-lissandra",1,LOG_LEVEL_ERROR);
	LOGGER_METRICS = log_create("logger_metrics.log", "log_metrics", 0, LOG_LEVEL_INFO);
	log_info(LOGGER, "Inicia Proceso");
}

void exit_gracefully(int exit_code){
	if(exit_code == EXIT_FAILURE){
		log_error(LOGGER,strerror(errno));
	}
	else{
		log_info(LOGGER,"Proceso termino correctamente");
	}
	log_destroy(LOGGER);
	exit(exit_code);
}

char *consistencia2string(Consistencias consistencia){
	char* str=malloc(4);
	switch(consistencia){
		case EC:
			strcpy(str,"EC");
		break;
		case SC:
			strcpy(str,"SC");
		break;
		case SHC:
			strcpy(str,"SHC");
		break;
		default:
			strcpy(str,"S/D");
	}
	return str;
}

int string2consistencia(char* consistencia){
	if(strcmp(consistencia,"EC")==0){
		return EC;
	}
	if(strcmp(consistencia,"SC")==0){
		return SC;
	}
	if(strcmp(consistencia,"SHC")==0){
		return SHC;
	}
	return DISP;
}

int monitorNode(char * node,int mode,int(*callback)(void)){
	char buffer[EVENT_BUF_LEN];
	int infd = inotify_init();
	if (infd < 0) {
		log_error(LOGGER,"Problemas al crear el inotify para %s, %s",node,strerror(errno));
		return 1;
	}
	int wfd = inotify_add_watch(infd,node,mode);
	while(1){
		int length = read(infd,buffer,EVENT_BUF_LEN);
		if (length < 0) {
			log_error(LOGGER,"Problemas al leer el inotify de %s, %s",node,strerror(errno));
			inotify_rm_watch(infd,wfd);
			close(infd);
			return 1;
		}
		if((*callback)()!=0){
			inotify_rm_watch(infd,wfd);
			close(infd);
			return 1;
		}
		inotify_rm_watch(infd,wfd);
		wfd = inotify_add_watch(infd,node,mode);
	}
	inotify_rm_watch(infd,wfd);
	close(infd);
	return 0;
}

void print_guenguencha(){
	printf("@                                `:,,,:`                      @\n@                             ;'+;#++:',:                     @\n@                         ,;',++#'#+#'++;:`                   @\n@                         ;'+;++###+#+++++'.                  @\n@                   ,'+;';'##++++####+#+#++;:                 @\n@                   ;'+++,++'+########++++++'.                @\n@                  ;++;++;++####+##++++++++#'+                @\n@                ';'++###+++#######++++++++++++++#            @\n@                '+;+###+++++#################++++',          @\n@               '+####++++++###########################++     @\n@              ;++####++++######+++++++++++##############+    @\n@             ;'+#####++##########++++++++++++++###@######+#  @\n@             +'#####++##++++++++++++++++++++++####@#######+` @\n@            .'+#+####+''''''''+++++++++''+++######@######### @\n@            `++#+###'''''#######++''''+'''+++##+##@#######++ @\n@            .'+####''''@########+''''''''''''''+#@########+  @\n@            ;'++###'''@@@@@@@@####+''''''####+'''#@########+ @\n@            @+++###'''##++    ####''''''#######''#@########+ @\n@           `++#+#+#'''####+++###+++';'''#@@#@#@''@@@#######+ @\n@            '+#++#@'''''+######++'';;;''@@@   @@''@@######++ @\n@           @@@@@##@';;:;;''+++'''';::;''@@@@@@@@+@@@@#####+. @\n@           @@@@@#@;:::::;;''''''';;::;;;'+@@@@+++@@@@####++  @\n@          @@@@##@#:,,,::;;'''';;;::::;;;''++###+'@@@@####+,  @\n@          @@@@@@@':,,,::;;'''';;::::::;;;''++++''@@@@####+   @\n@          +@@@@@;,;::'#+'';;;;;;;;;::;;;::''++';:@#:         @\n@          '@@@@@;::::'++;;;;;;;;';;;;;;;;''''+';,'`          @\n@            @@@@:,:::'++#+':;;;'''''';;'''''++':;'           @\n@            @@@@,::;:'++##'#;;;;''''+'''';;;'#';;;           @\n@             @@@;::':'+++ '#::'''''''''';;' ++';';           @\n@              ;@+::;:;+';  +:,;''''''''''#  #';'',           @\n@                 :;::;+';  ';,;:''''''';    +':';:           @\n@                 :;:;:'';;   ;,;::::''''   #;;;+''           @\n@                 :::;:'';;;   ;;;::;;'    +';,'''            @\n@                 ::;;:;';;:             :;+;;:               @\n@                 :::;;;;;;:;           ;:'';',               @\n@              ::::::;;;::;;;;        ;;;;;                   @\n@             ,:::::;;;;;::;;;;;     ;;;;:                    @\n@            :,:::::;;;;;;:;;;;;'''''';;;                     @\n@  ######'::,::;::::;;;;;;;:;;;;;;;;;;;;:                     @\n@#########;:,::;;:::;;;''';;;;;;;;;;;;;;                      @\n@@########+:,:;;;;;;;;;''''';;;;;;;;;;';';;'+##               @\n@@#@#######+,,;;;;;;;;''''''''''';;;;;;;:;:;+###              @\n@@@@@######@':;;;;;;;''''''''''++++;;;;::+;;+######           @\n@@@#@@#####@@';';;;;;;'''''''''''';;;;;:'##:+#########+       @\n@@@@@@@@@@@##@+;;;;;;;'''''''''''';;;;;:+@#'+###########`     @\n@@@@@@@@@@@@@@@#';;;;;'''''''''''';;;;;:@##++###########+     @\n@@@@@@@@@@@@@@@@@';;;'''''''''''';;;;;;:####+#############    @\n");

}

char *get_local_ip(void){
	int fd;
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	memcpy(ifr.ifr_name, "enp0s3", IFNAMSIZ-1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);
	strcpy(LOCAL_IP,inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	return LOCAL_IP;
}
