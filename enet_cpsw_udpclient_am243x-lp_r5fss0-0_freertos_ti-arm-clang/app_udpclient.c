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
#define MAX_LOGS            (30)  // Store the last 30 logs
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
/*                          THOMAS'S PROTOCOL MACROS                          */
/* ========================================================================== */
#define PACKET_MAGIC    0x1234
#define PKT_ID          0
#define PKT_CMD         2
#define PKT_SEQ         3
#define PKT_TOT         5
#define PKT_IDX         7
#define PKT_LEN         9
#define PKT_DATA        14

#define ENTRY_STATUS    0
#define ENTRY_TS        1
#define ENTRY_TYPE      9
#define ENTRY_MSG       10

#define CMD_GET_ALL_LOGS 0x00 // Matched to his CMD_GET_ALL_LOGS enum 
#define MAX_MSG_LEN      256

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
static LogItem_t gLogBuffer[MAX_LOGS];
static uint32_t gLogIndex = 0;
static uint32_t gTotalLogs = 0;
static SemaphoreHandle_t gLogMutex = NULL;

/* ========================================================================== */
/*                          Protocol Helpers                                  */
/* ========================================================================== */

/* Helper to write 16-bit int to buffer (Big/Little Endian safe) */
static void write_u16(uint8_t *b, uint16_t v) {
    b[0] = (v >> 8) & 0xFF; 
    b[1] = v & 0xFF;
}

/* Helper to write 64-bit int to buffer */
static void write_u64(uint8_t *b, uint64_t v) {
    for (int i = 7; i >= 0; i--) { 
        b[i] = v & 0xFF; 
        v >>= 8; 
    }
}

/* Helper to calculate Thomas's specific checksum format */
static uint16_t calc_checksum(const uint8_t *buf, int len) {
    uint16_t sum = 0;
    for (int i = 0; i < len - 2; i++) {
        sum += buf[i];
    }
    return sum;
}

/* Helper to read 16-bit int from Thomas's packet */
static uint16_t read_u16(const uint8_t *b) {
    return (uint16_t)((b[0] << 8) | b[1]);
}

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* THREAD 1: Data Generator Task (Runs every 100ms) */
static void AppSocket_dataGeneratorTask(void *pArg)
{
    // Generate UNIX epoch timestamp in MS to match his ISO date format
    // Starting at an arbitrary recent timestamp to look normal
    uint64_t currentTimeMs = 1774890000000; 

    EnetAppUtils_print("Data Generator Thread Started...\r\n");

    while (1)
    {
        /* Lock Mutex to prevent UDP server from reading while we write */
        if (xSemaphoreTake(gLogMutex, portMAX_DELAY) == pdTRUE)
        {
            // Generate simulated telemetry data
            gLogBuffer[gLogIndex].timestamp = (uint32_t)(currentTimeMs & 0xFFFFFFFF); 
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

/* THREAD 2: UDP Server Task (On-Demand Real-Time Streamer) */
static void AppSocket_udpServerTask(void *pArg)
{
    int32_t sock = -1;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char rx_buffer[128];
    char tx_buffer[TX_BUFFER_SIZE];

    EnetAppUtils_print("UDP Streamer Started on Port %d...\r\n", SERVER_UDP_PORT);

    sock = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        vTaskDelete(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = PP_HTONS(SERVER_UDP_PORT);
    server_addr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);

    lwip_bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    EnetAppUtils_print("Waiting for Ingest Server to subscribe...\r\n");

    /* 1. BLOCK until the Python server sends a trigger packet */
    lwip_recvfrom(sock, rx_buffer, sizeof(rx_buffer), 0,
                  (struct sockaddr *)&client_addr, &client_len);

    /* 2. We got a packet! Extract the IP automatically (No hardcoding!) */
    char client_ip[16];
    ip4addr_ntoa_r((const ip4_addr_t *)&client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip));
    EnetAppUtils_print("Server Subscribed from %s:%d. Starting live stream...\r\n", client_ip, ntohs(client_addr.sin_port));

    /* 3. Stream data continuously */
    while (1)
    {
        if (xSemaphoreTake(gLogMutex, portMAX_DELAY) == pdTRUE)
        {
            int logs_to_send = (gTotalLogs < 10) ? gTotalLogs : 10;
            int start_idx = (gTotalLogs < 10) ? 0 : gTotalLogs - 10;
            
            // Clear the entire buffer
            memset(tx_buffer, 0, TX_BUFFER_SIZE);
            int offset = 0; // Keep track of exactly where we are in the buffer

            for (int i = 0; i < logs_to_send; i++)
            {
                int actual_idx = (start_idx + i) % MAX_LOGS;
                LogItem_t *log = &gLogBuffer[actual_idx];

                // Write directly into tx_buffer at the current offset.
                // snprintf returns the number of characters it wrote.
                int written = snprintf(tx_buffer + offset, TX_BUFFER_SIZE - offset, 
                         "[TS:%u] Type:%d | Temp:%.1fC | Volt:%.1fV\n",
                         log->timestamp, log->type, log->temperature, log->voltage);
                
                // Move the offset forward so the next loop writes AFTER this line
                if (written > 0 && offset + written < TX_BUFFER_SIZE) {
                    offset += written;
                }
            }

            // Blast the chunk of logs to the Python Server
            if (offset > 0) {
                lwip_sendto(sock, tx_buffer, offset, 0,
                            (struct sockaddr *)&client_addr, client_len);
            }

            xSemaphoreGive(gLogMutex);
        }

        // Stream rate: 1 update per second
        vTaskDelay(pdMS_TO_TICKS(1000));
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
