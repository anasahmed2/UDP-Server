/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include "app_udpclient.h" // Keep original header for build compatibility

#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

/* FreeRTOS Headers for Threading and Mutex */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <kernel/dpl/TaskP.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/CacheP.h>
#include "enet_apputils.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#define SERVER_UDP_PORT     (8016)
#define MAX_LOGS            (30)  // Store the last 100 logs
#define TX_BUFFER_SIZE      (1024)
#define RX_BUFFER_SIZE      (128)

/* Requirement: Data structure with at least 3 items */
typedef struct {
    uint32_t timestamp;
    uint8_t  type;          // 0 = INFO, 1 = WARN, 2 = ERROR
    float    temperature;
    float    voltage;
} LogItem_t;

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
static LogItem_t gLogBuffer[MAX_LOGS];
static uint32_t gLogIndex = 0;
static uint32_t gTotalLogs = 0;
static SemaphoreHandle_t gLogMutex = NULL;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* THREAD 1: Data Generator Task (Runs every 100ms) */
static void AppSocket_dataGeneratorTask(void *pArg)
{
    uint32_t currentTimeMs = 0;

    EnetAppUtils_print("Data Generator Thread Started...\r\n");

    while (1)
    {
        /* Lock Mutex to prevent UDP server from reading while we write */
        if (xSemaphoreTake(gLogMutex, portMAX_DELAY) == pdTRUE)
        {
            // Generate simulated telemetry data
            gLogBuffer[gLogIndex].timestamp = currentTimeMs;
            gLogBuffer[gLogIndex].type = rand() % 3; 
            gLogBuffer[gLogIndex].temperature = 20.0f + ((float)(rand() % 100) / 10.0f);
            gLogBuffer[gLogIndex].voltage = 5.0f - ((float)(rand() % 10) / 10.0f);

            gLogIndex = (gLogIndex + 1) % MAX_LOGS; // Circular buffer wrap
            if (gTotalLogs < MAX_LOGS) gTotalLogs++;

            currentTimeMs += 100;

            xSemaphoreGive(gLogMutex); /* Unlock Mutex */
        }

        /* Sleep exactly 100ms per requirement */
        ClockP_sleep(0); // Yield
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/* THREAD 2: UDP Server Task (Listens on Port 8016) */
static void AppSocket_udpServerTask(void *pArg)
{
    int32_t sock = -1, ret = 0;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char rx_buffer[RX_BUFFER_SIZE];
    char tx_buffer[TX_BUFFER_SIZE];

    EnetAppUtils_print("UDP Server Thread Started on Port %d...\r\n", SERVER_UDP_PORT);

    /* Create the socket */
    sock = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        EnetAppUtils_print("ERR: unable to open socket\r\n");
        vTaskDelete(NULL);
    }

    /* Bind the socket to Port 8016 */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = PP_HTONS(SERVER_UDP_PORT);
    server_addr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);

    ret = lwip_bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0)
    {
        EnetAppUtils_print("ERR: socket bind failed\r\n");
        lwip_close(sock);
        vTaskDelete(NULL);
    }

    /* Main Server Loop */
    while (1)
    {
        // Block and wait for ANY packet from Thomas's Client
        int len = lwip_recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
                                (struct sockaddr *)&client_addr, &client_len);

        if (len > 0)
        {
            rx_buffer[len] = '\0'; // Null terminate received string
            tx_buffer[0] = '\0';   // Clear transmit buffer
            
            EnetAppUtils_print("Received a trigger packet! Sending all logs...\r\n");

            /* Lock Mutex to safely read logs */
            if (xSemaphoreTake(gLogMutex, portMAX_DELAY) == pdTRUE)
            {
                // Send all available logs currently in the buffer
                int start = (gTotalLogs < MAX_LOGS) ? 0 : gTotalLogs - MAX_LOGS;
                
                for (int i = start; i < gTotalLogs; i++)
                {
                    char temp[64];
                    int idx = i % MAX_LOGS;
                    
                    // Format: timestamp, type, temperature, voltage
                    snprintf(temp, sizeof(temp), "%u,%d,%.1f,%.1f\n",
                             gLogBuffer[idx].timestamp, gLogBuffer[idx].type,
                             gLogBuffer[idx].temperature, gLogBuffer[idx].voltage);
                    
                    // Append to transmit buffer
                    strcat(tx_buffer, temp);
                }

                xSemaphoreGive(gLogMutex); /* Unlock Mutex */
            }

            /* Send the data back to Thomas's Client */
            lwip_sendto(sock, tx_buffer, strlen(tx_buffer), 0,
                        (struct sockaddr *)&client_addr, client_len);
        }
    }
}

/* Replaces AppSocket_startClient in the original file */
void AppSocket_startServer(void)
{
    /* Create Mutex */
    gLogMutex = xSemaphoreCreateMutex();
    EnetAppUtils_assert(gLogMutex != NULL);

    /* Spawn Thread 1: Data Generator */
    sys_thread_new("LogGenTask", AppSocket_dataGeneratorTask, NULL, 
                   DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

    /* Spawn Thread 2: UDP Server */
    sys_thread_new("UdpSrvTask", AppSocket_udpServerTask, NULL, 
                   DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
}
