#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include <mongoc/mongoc.h>

#include "child.h"

int child_main(int client_fd, char *addr) {

	//MONGO STUFF//
	
	bson_t *command, reply, *insert;
	char *str;
	bool retval;

	mongoc_init();

	bson_error_t error;
	const char *uri_string = "mongodb://localhost:27017";
	mongoc_uri_t *uri = mongoc_uri_new_with_error(uri_string, &error);

	if (!uri) {
		fprintf(stderr, 
				"failed to parse URI: %s\n"
				"error message:       %s\n",
				uri_string,
				error.message);
		return 1;
	}
	
	mongoc_client_t *client = mongoc_client_new_from_uri(uri);
	if (!client) return 1;

	mongoc_client_set_appname(client, "wisdom-server");

	mongoc_database_t *database = mongoc_client_get_database(client, "wisdom");
	mongoc_collection_t *collection = mongoc_client_get_collection(
			client,
			"wisdom",
			"test"
	);

	bson_t *document = NULL;
	bson_oid_t oid = {0};
	time_t raw_time;
	struct tm *now;
	

	char buf[100];
	size_t num;
	for (;;) {

		num = recv(client_fd, buf, 100, 0);	
		if (num < 1) break;
		buf[num] = '\0';

		if (!strcmp(buf, "PING")) {
			printf("Received: PING\n");
			printf("Sending: PONG\n");
			send(client_fd, "PONG", 4, 0);

			// Get current time (according to server)
			raw_time = time(NULL);
			now = localtime(&raw_time);

			document = bson_new();
			bson_oid_init(&oid, NULL);
			BSON_APPEND_OID(document, "_id", &oid);
			BSON_APPEND_DATE_TIME(document, "timestamp", mktime(now));
			BSON_APPEND_UTF8(document, "data", buf);
			BSON_APPEND_UTF8(document, "response", "PONG");

			if (!mongoc_collection_insert_one(
						collection, document, NULL, NULL, &error)) {

				fprintf(stderr, "%s\n", error.message);
			}

			bson_destroy(document);
		}
	}

	mongoc_collection_destroy(collection);
	mongoc_client_destroy(client);
	mongoc_cleanup();
	document = NULL;

	printf("Child closing!\n");
}
