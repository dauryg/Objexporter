objexporter: objexporter.c
	gcc objexporter.c -o objexporter -Wall -Wextra

debug: objexporter.c
	gcc objexporter.c -o objexporter -Wall -ggdb
