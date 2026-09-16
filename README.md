/***********************************************************************************
/***********************************************************************************
Linux System Health Monitor

A lightweight real-time Linux system monitoring application developed in C using Linux/POSIX APIs, POSIX threads, TCP sockets, HTTP, JSON, and a browser-based dashboard.
The application collects system health information, stores the latest measurements in a thread-safe monitoring engine, exposes the information through a custom HTTP server, and presents the results through a live web dashboard.



/***********************************************************************************
/***********************************************************************************
Features:
* CPU usage monitoring
* Memory usage monitoring
* Root filesystem disk usage monitoring
* Network interface monitoring
* Network interface IP addresses
* Network RX/TX byte counters
* Hostname detection
* Operating system information
* Kernel version
* CPU architecture
* Metric state tracking:
     ->UNKNOWN
     ->VALID
     ->ERROR
* Timestamp for each monitored metric
* Multithreaded monitoring using POSIX threads
* Thread-safe shared monitoring data using pthread_mutex
* Custom TCP/HTTP server implemented using POSIX socket APIs
* JSON API endpoint at /api/status
* Live browser dashboard
* Automatic dashboard refresh every 5 seconds
* Scheduled log maintenance using cron
* systemd service support for automatic startup after system boot



/***********************************************************************************
/***********************************************************************************
Architecture

                         Linux System
                              |
             +----------------+----------------+
             |                |                |
        /proc/stat      /proc/meminfo     statvfs()
             |                |                |
             |                |                |
             +----------------+----------------+
                              |
                    getifaddrs()
                    /proc/net/dev
                              |
                              v
                    Monitoring Modules
                              |
                              v
                     Monitor Engine
                     POSIX Threads
                              |
                              v
                    monitor_data_t
                     + pthread_mutex
                              |
                              v
                    Custom HTTP Server
                       TCP Port 8080
                              |
                        /api/status
                              |
                             JSON
                              |
                              v
                       JavaScript fetch()
                              |
                              v
                    Web Dashboard



/***********************************************************************************
/***********************************************************************************
Technologies:
Core:
C, Linux, POSIX APIs, GCC, pthreads, Linux system interfaces

Networking:
TCP sockets, IPv4, HTTP, JSON

Frontend:
HTML, CSS, JavaScript, Fetch API

System Management:
systemd, cron



/***********************************************************************************
/***********************************************************************************
Monitoring Engine

The monitoring engine maintains a shared monitor_data_t structure.
The monitoring engine starts separate pthreads for:
* CPU
* Memory
* Disk
* Network

Each thread:
1. Collects data independently.
2. Locks the shared mutex.
3. Updates its corresponding monitoring data.
4. Updates the metric state.
5. Stores the latest update timestamp.
6. Unlocks the mutex.
7. Waits for the next monitoring cycle.

* The monitoring interval is currently 5 seconds.
* The mutex is held only while shared data is copied or updated. This avoids keeping the lock during slower operations such as socket communication.



/***********************************************************************************
/***********************************************************************************
Project Installation
->Building the Monitor
    * Go to the project directory:
	cd ~/Documents/linux-health-monitor

    * Build the standalone monitor:
	cd monitor
	make
	
->Run it:
	./system_monitor

	* The standalone monitor is useful for testing the monitoring engine independently from the web server.

->Building the HTTP Server
    * Go to:
	cd ~/Documents/linux-health-monitor/backend/src
    * Build:
	gcc -Wall -Wextra \
	    -I../../monitor/include \
	    http_server.c \
	    ../../monitor/src/monitor_engine.c \
	    ../../monitor/src/system_info.c \
	    ../../monitor/src/memory_monitor.c \
	    ../../monitor/src/cpu_monitor.c \
	    ../../monitor/src/disk_monitor.c \
	    ../../monitor/src/network_monitor.c \
	    -pthread \
	    -o http_server
->Starting the backend service
    * cd ~/Documents/linux-health-monitor/backend/src
    * ./http_server


->Open the dashboard in a browser:
	http://localhost:8080



/***********************************************************************************
/***********************************************************************************
Project Goal:
The project demonstrates how low-level Linux system information can be collected in C, processed concurrently using POSIX threads, exposed through a custom HTTP interface, and presented through a live browser dashboard without relying on a large backend framework.
