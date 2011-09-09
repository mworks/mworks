#include <stdlib.h>
#include <stdio.h>
#include <scarab.h>

ScarabDatum *longEvent(char *name, long value, long time, void *memctx);

#define	serr(srv)	\
	errInt(	scarab_session_geterr((srv)), \
			scarab_session_getoserr((srv)) \
		)

#define	errInt(code, oserr)  \
	printErr(	(code), \
				scarab_moderror((code)), scarab_strerror((code)), \
				(oserr), \
				scarab_os_strerror((oserr)) \
			)

#define	printErr(errcode, mod, msg, oserr, osmsg) \
		do{ \
		fprintf(stderr,"ERROR: SCR-%5.5i: %s: %s\n", \
				(errcode), (mod), (msg)); \
		if ((oserr)) { \
		fprintf(stderr, "OSERR: %i: %s\n", \
				(oserr), (osmsg)); \
		} \
		}while(0)

int test_server(char *uri, void *memctx) {
	ScarabSession	*srv;
	ScarabSession	*client;
    ScarabDatum	 *spike_event, *reset_event, 
                 *trialend_event, *trialstart_event;
    int i;

	printf("Binding to %s...\n", uri);
	srv = scarab_session_listen(memctx, uri);
	if (!srv) {
		printErr(0,"SCARAB", "Session could not be allocated.", 0, "");
		return 1;
	}
	if (scarab_session_geterr(srv)) {
		serr(srv);
		return 1;
	}
	for(;;)	{
		client = scarab_session_accept(memctx, srv);
		if (!srv) {
			printErr(0,"SCARAB",
                            "Client session could not be allocated.", 0, "");
			continue;
		}
		if (scarab_session_geterr(client)) {
			serr(client);
			continue;
		}
        
		printf("[ACCEPT]\n");
        scarab_write(client, longEvent("taskMode", 1, 1, memctx));
        
        int flag = 0;
        for(i = 0; i <40000; i += 1000) {
            if (scarab_session_geterr(client)) {
                serr(client);
                flag = 1;
                printf("[DISCONNECT]\n");
                continue;
            }
            trialend_event = scarab_dict_new(memctx, 3, NULL);
                                        
            scarab_dict_put(trialend_event, scarab_new_string(memctx,"name"), 
                                        scarab_new_string(memctx,"trialEnd"));
            scarab_dict_put(trialend_event, scarab_new_string(memctx,"value"), 
                                                scarab_new_integer(memctx,1));
            scarab_dict_put(trialend_event, scarab_new_string(memctx, "time"), 
                                                scarab_new_integer(memctx, i));

            trialstart_event = scarab_dict_new(memctx, 3, NULL);
                                        
            scarab_dict_put(trialstart_event, scarab_new_string(memctx,"name"),
                                    scarab_new_string(memctx,"trialStart"));
            scarab_dict_put(trialstart_event, scarab_new_string(memctx,"value"),
                                              scarab_new_integer(memctx,1));
            scarab_dict_put(trialstart_event, scarab_new_string(memctx, "time"),
                                             scarab_new_integer(memctx, i));

            reset_event = scarab_dict_new(memctx, 3, NULL);
                                        
            scarab_dict_put(reset_event, scarab_new_string(memctx,"name"),
                                            scarab_new_string(memctx,"reset"));
            scarab_dict_put(reset_event, scarab_new_string(memctx,"value"),
                                                scarab_new_integer(memctx,1));
            scarab_dict_put(reset_event, scarab_new_string(memctx, "time"),
                                                scarab_new_integer(memctx, i));

            spike_event = scarab_dict_new(memctx, 3, NULL);
                                        
            scarab_dict_put(spike_event, scarab_new_string(memctx,"name"),
                                    scarab_new_string(memctx,"spikeZero"));
            scarab_dict_put(spike_event, scarab_new_string(memctx,"value"),
                                            scarab_new_integer(memctx,1));
            scarab_dict_put(spike_event, scarab_new_string(memctx, "time"),
                                            scarab_new_integer(memctx, i+200));
                    
            scarab_write(client, trialend_event);
            //usleep(1000000);
            scarab_write(client, trialstart_event);
            scarab_write(client, reset_event);
            scarab_write(client, spike_event);
            //sleep(2000000);
        }
        if(flag){
            printf("[DISCONNECT]\n");
            continue;
        }
	//	scarab_session_close(client);
        printf("[DISCONNECT]\n");
	}
	return 0;
}

ScarabDatum *longEvent(char *name, long value, long time, void *memctx) {
    ScarabDatum *theevent;
    theevent = scarab_dict_new(memctx, 3, NULL);

    scarab_dict_put(theevent, scarab_new_string(memctx,"name"),
                                            scarab_new_string(memctx,name));
    scarab_dict_put(theevent, scarab_new_string(memctx,"value"),
                                        scarab_new_integer(memctx,value));
    scarab_dict_put(theevent, scarab_new_string(memctx, "time"),
                                        scarab_new_integer(memctx, time));
}

int test_client(char *uri, void *memctx) {
	ScarabSession	*srv;
	ScarabDatum		*d;
    int count = 0;
	printf("Connecting to %s...\n", uri);
	srv = scarab_session_connect(memctx, uri);
	if (!srv) {
		printErr(0,"SCARAB", "Session could not be allocated.", 0, "");
		return 1;
	}

	if (scarab_session_geterr(srv)) {
		serr(srv);
		return 1;
	}

	printf("Connected!\n");
/*    while(count < 30000) {
        d = scarab_read(srv, srv->memctx);
        if(d == NULL) {
            printf("No Datum read");
        } else {
            printf("Valid Datum read");
        }
        count++;
    }*/
	d =  scarab_read(srv, srv->memctx);

	printf("  read int '%i'\n", (int)d->data.integer);
    printf("Closing session");
	scarab_session_close(srv);

	return 0;
}

int main(int argc, char *argv[]) {
	void * memctx;

    scarab_init();
	memctx = scarab_memctx_new();

	if (argv[1] && strcmp(argv[1], "client") == 0 && argv[2]) {
		return test_client(argv[2], memctx);
	} else if (argv[1] && strcmp(argv[1], "server") == 0 && argv[2]) {
		return test_server(argv[2], memctx);
	} else {
		fprintf(stderr, "usage: %s server ldobinary:tcp://server/port\n",
                                                                    argv[0]);
		fprintf(stderr, "       %s client ldobinary:tcp://server/port\n",
                                                                    argv[0]);
		return 1;
	}

	return 0;
}
