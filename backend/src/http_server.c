#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../../monitor/include/monitor_engine.h"

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void)
{
    int server_fd;
    struct sockaddr_in server_addr;

    /* =========================================================
     * CREATE SOCKET
     * =========================================================
     */

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }

    printf("Socket created successfully\n");

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    /* =========================================================
     * BIND
     * =========================================================
     */

    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        return 1;
    }

    printf("Server bound to port %d\n", PORT);

    /* =========================================================
     * LISTEN
     * =========================================================
     */

    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("HTTP server listening on port %d...\n", PORT);

    /* =========================================================
     * INITIALIZE MONITOR ENGINE
     * =========================================================
     */

    monitor_data_t monitor_data;

    if (monitor_engine_init(&monitor_data) != 0)
    {
        fprintf(
            stderr,
            "Failed to initialize monitor engine\n"
        );

        close(server_fd);
        return 1;
    }

    if (monitor_engine_start(&monitor_data) != 0)
    {
        fprintf(
            stderr,
            "Failed to start monitor engine\n"
        );

        monitor_engine_destroy(&monitor_data);
        close(server_fd);

        return 1;
    }

    printf("Monitoring engine started\n");

    /* =========================================================
     * HTTP SERVER SUPERLOOP
     * =========================================================
     */

    while (1)
    {
        int client_fd;

        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        char buffer[BUFFER_SIZE];

        /* -----------------------------------------------------
         * ACCEPT CLIENT
         * -----------------------------------------------------
         */

        client_fd = accept(
            server_fd,
            (struct sockaddr *)&client_addr,
            &client_len
        );

        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }

        printf("\nClient connected\n");

        /* -----------------------------------------------------
         * RECEIVE HTTP REQUEST
         * -----------------------------------------------------
         */

        memset(buffer, 0, sizeof(buffer));

        int bytes_received = read(
            client_fd,
            buffer,
            sizeof(buffer) - 1
        );

        if (bytes_received <= 0)
        {
            close(client_fd);
            continue;
        }

        printf("----- HTTP REQUEST -----\n");
        printf("%s", buffer);
        printf("-----------------------\n");

        /* -----------------------------------------------------
         * PARSE METHOD AND PATH
         * -----------------------------------------------------
         */

        char method[16];
        char path[256];

        if (sscanf(
                buffer,
                "%15s %255s",
                method,
                path) != 2)
        {
            close(client_fd);
            continue;
        }

        printf("Method: %s\n", method);
        printf("Path: %s\n", path);

        /* =====================================================
         * API: /api/status
         * =====================================================
         */

	if (strcmp(path, "/api/status") == 0)
	{
	    char json[8192];

	    /*
	     * ---------------------------------------------------------
	     * Copy shared monitoring data while holding the mutex
	     * ---------------------------------------------------------
	     */

	    pthread_mutex_lock(&monitor_data.mutex);

	    /* System information */
	    system_info_t system = monitor_data.system;

	    /* CPU */
	    double cpu =
		monitor_data.cpu.usage_percent;

	    metric_state_t cpu_state =
		monitor_data.cpu_status.state;

	    time_t cpu_updated =
		monitor_data.cpu_status.last_updated;

	    /* Memory */
	    double memory =
		monitor_data.memory.usage_percent;

	    metric_state_t memory_state =
		monitor_data.memory_status.state;

	    time_t memory_updated =
		monitor_data.memory_status.last_updated;

	    /* Disk */
	    double disk =
		monitor_data.disk.usage_percent;

	    metric_state_t disk_state =
		monitor_data.disk_status.state;

	    time_t disk_updated =
		monitor_data.disk_status.last_updated;

	    /* Network */
	    network_info_t network =
		monitor_data.network;

	    metric_state_t network_state =
		monitor_data.network_status.state;

	    time_t network_updated =
		monitor_data.network_status.last_updated;

	    pthread_mutex_unlock(&monitor_data.mutex);

	    /*
	     * ---------------------------------------------------------
	     * Convert states to strings
	     * ---------------------------------------------------------
	     */

	    const char *cpu_state_str =
		(cpu_state == METRIC_STATE_VALID) ? "VALID" :
		(cpu_state == METRIC_STATE_ERROR) ? "ERROR" :
		"UNKNOWN";

	    const char *memory_state_str =
		(memory_state == METRIC_STATE_VALID) ? "VALID" :
		(memory_state == METRIC_STATE_ERROR) ? "ERROR" :
		"UNKNOWN";

	    const char *disk_state_str =
		(disk_state == METRIC_STATE_VALID) ? "VALID" :
		(disk_state == METRIC_STATE_ERROR) ? "ERROR" :
		"UNKNOWN";

	    const char *network_state_str =
		(network_state == METRIC_STATE_VALID) ? "VALID" :
		(network_state == METRIC_STATE_ERROR) ? "ERROR" :
		"UNKNOWN";

	    /*
	     * ---------------------------------------------------------
	     * Start JSON
	     * ---------------------------------------------------------
	     */

	    int offset = snprintf(
		json,
		sizeof(json),

		"{"
		"\"system\":{"
		    "\"hostname\":\"%s\","
		    "\"os_name\":\"%s\","
		    "\"kernel_version\":\"%s\","
		    "\"architecture\":\"%s\""
		"},"

		"\"cpu\":{"
		    "\"usage\":%.2f,"
		    "\"state\":\"%s\","
		    "\"updated\":%ld"
		"},"

		"\"memory\":{"
		    "\"usage\":%.2f,"
		    "\"state\":\"%s\","
		    "\"updated\":%ld"
		"},"

		"\"disk\":{"
		    "\"usage\":%.2f,"
		    "\"state\":\"%s\","
		    "\"updated\":%ld"
		"},"

		"\"network\":{"
		    "\"state\":\"%s\","
		    "\"interface_count\":%d,"
		    "\"updated\":%ld,"
		    "\"interfaces\":[",

		system.hostname,
		system.os_name,
		system.kernel_version,
		system.architecture,

		cpu,
		cpu_state_str,
		(long)cpu_updated,

		memory,
		memory_state_str,
		(long)memory_updated,

		disk,
		disk_state_str,
		(long)disk_updated,

		network_state_str,
		network.interface_count,
		(long)network_updated
	    );

	    /*
	     * ---------------------------------------------------------
	     * Add network interfaces
	     * ---------------------------------------------------------
	     */

	    for (int i = 0; i < network.interface_count; i++)
	    {
		network_interface_t *iface =
		    &network.interfaces[i];

		int written = snprintf(
		    json + offset,
		    sizeof(json) - (size_t)offset,

		    "%s{"
		        "\"name\":\"%s\","
		        "\"ip_address\":\"%s\","
		        "\"is_up\":%d,"
		        "\"rx_bytes\":%llu,"
		        "\"tx_bytes\":%llu"
		    "}",

		    (i > 0) ? "," : "",

		    iface->name,
		    iface->ip_address,
		    iface->is_up,
		    iface->rx_bytes,
		    iface->tx_bytes
		);

		if (written < 0)
		{
		    close(client_fd);
		    continue;
		}

		offset += written;

		if (offset >= (int)sizeof(json))
		{
		    break;
		}
	    }

	    /*
	     * ---------------------------------------------------------
	     * Finish JSON
	     * ---------------------------------------------------------
	     */

	    int written = snprintf(
		json + offset,
		sizeof(json) - (size_t)offset,

		"]"
		"}"
		"}"
	    );

	    if (written < 0)
	    {
		close(client_fd);
		continue;
	    }

	    offset += written;

	    int json_length = offset;

	    /*
	     * ---------------------------------------------------------
	     * Validate JSON buffer
	     * ---------------------------------------------------------
	     */

	    if (json_length <= 0 ||
		json_length >= (int)sizeof(json))
	    {
		const char *response =
		    "HTTP/1.1 500 Internal Server Error\r\n"
		    "Content-Type: text/plain\r\n"
		    "Content-Length: 21\r\n"
		    "Connection: close\r\n"
		    "\r\n"
		    "Internal Server Error";

		write(
		    client_fd,
		    response,
		    strlen(response)
		);

		close(client_fd);
		continue;
	    }

	    /*
	     * ---------------------------------------------------------
	     * Build HTTP response header
	     * ---------------------------------------------------------
	     */

	    char header[512];

	    int header_length = snprintf(
		header,
		sizeof(header),

		"HTTP/1.1 200 OK\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: %d\r\n"
		"Connection: close\r\n"
		"\r\n",

		json_length
	    );

	    /*
	     * ---------------------------------------------------------
	     * Send response
	     * ---------------------------------------------------------
	     */

	    write(
		client_fd,
		header,
		header_length
	    );

	    write(
		client_fd,
		json,
		json_length
	    );

	    printf("API response sent\n");

	    close(client_fd);

	    printf("Client disconnected\n");

	    continue;
	}
    
        /* =====================================================
         * STATIC FILE ROUTING
         * =====================================================
         */

        const char *file_path;

        if (strcmp(path, "/") == 0)
        {
            file_path = "../../frontend/index.html";
        }
        else if (strcmp(path, "/style.css") == 0)
        {
            file_path = "../../frontend/style.css";
        }
        else if (strcmp(path, "/script.js") == 0)
        {
            file_path = "../../frontend/script.js";
        }
        else
        {
            /* -------------------------------------------------
             * 404 RESPONSE
             * -------------------------------------------------
             */

            const char *response =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 13\r\n"
                "Connection: close\r\n"
                "\r\n"
                "404 Not Found";

            write(
                client_fd,
                response,
                strlen(response)
            );

            printf(
                "404 - Resource not found: %s\n",
                path
            );

            close(client_fd);

            printf("Client disconnected\n");

            continue;
        }

        printf("Serving: %s\n", file_path);

        /* =====================================================
         * OPEN FILE
         * ========================================================= */

        FILE *file = fopen(file_path, "r");

        if (file == NULL)
        {
            perror("fopen");

            const char *response =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 13\r\n"
                "Connection: close\r\n"
                "\r\n"
                "404 Not Found";

            write(
                client_fd,
                response,
                strlen(response)
            );

            close(client_fd);

            printf("Client disconnected\n");

            continue;
        }

        /* =====================================================
         * GET FILE SIZE
         * ========================================================= */

        fseek(file, 0, SEEK_END);

        long file_size = ftell(file);

        fseek(file, 0, SEEK_SET);

        if (file_size < 0)
        {
            fclose(file);
            close(client_fd);
            continue;
        }

        /* =====================================================
         * ALLOCATE FILE BUFFER
         * ========================================================= */

        char *file_data =
            malloc((size_t)file_size + 1);

        if (file_data == NULL)
        {
            perror("malloc");

            fclose(file);
            close(client_fd);

            continue;
        }

        /* =====================================================
         * READ FILE
         * ========================================================= */

        size_t bytes_read = fread(
            file_data,
            1,
            (size_t)file_size,
            file
        );

        file_data[bytes_read] = '\0';

        fclose(file);

        /* =====================================================
         * DETERMINE CONTENT TYPE
         * ========================================================= */

        const char *content_type;

        if (strcmp(path, "/style.css") == 0)
        {
            content_type = "text/css";
        }
        else if (strcmp(path, "/script.js") == 0)
        {
            content_type = "application/javascript";
        }
        else
        {
            content_type = "text/html";
        }

        /* =====================================================
         * BUILD HTTP HEADER
         * ========================================================= */

        char header[512];

        int header_length = snprintf(
            header,
            sizeof(header),

            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n"
            "\r\n",

            content_type,
            bytes_read
        );

        /* =====================================================
         * SEND FILE RESPONSE
         * ========================================================= */

        write(
            client_fd,
            header,
            header_length
        );

        write(
            client_fd,
            file_data,
            bytes_read
        );

        printf("HTTP response sent\n");

        free(file_data);

        /* -----------------------------------------------------
         * CLOSE CLIENT
         * -----------------------------------------------------
         */

        close(client_fd);

        printf("Client disconnected\n");
    }

    /* Never reached while superloop runs */
    monitor_engine_destroy(&monitor_data);

    close(server_fd);

    return 0;
}
