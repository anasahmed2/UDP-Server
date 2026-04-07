/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include "app_udpclient.h"

#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

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
#define SERVER_UDP_PORT  (8016)
#define MAX_LOGS         (30)
#define TX_BUFFER_SIZE   (256)
#define RX_BUFFER_SIZE   (128)

typedef struct {
    uint32_t timestamp;
    uint8_t  type;        // 0 = INFO, 1 = WARNING, 2 = ERROR
    float    temperature;
    float    voltage;
} LogItem_t;

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
static LogItem_t         gLogBuffer[MAX_LOGS];
static uint32_t          gLogIndex       = 0;
static uint32_t          gTotalLogs      = 0;
static SemaphoreHandle_t gLogMutex       = NULL;

// Handle to the UDP task so the generator can notify it directly
static TaskHandle_t      gUdpTaskHandle  = NULL;

/* ========================================================================== */
/*                    THREAD 1: Data Generator Task (100ms)                   */
/* ========================================================================== */
static void AppSocket_dataGeneratorTask(void *pArg)
{
    uint64_t currentTimeMs = 1774890000000ULL;

    EnetAppUtils_print("Data Generator Thread Started...\r\n");

    while (1)
    {
        if (xSemaphoreTake(gLogMutex, portMAX_DELAY) == pdTRUE)
        {
            // Write one new log into the circular buffer
            gLogBuffer[gLogIndex].timestamp   = (uint32_t)(currentTimeMs & 0xFFFFFFFF);
            gLogBuffer[gLogIndex].type        = rand() % 3;
            gLogBuffer[gLogIndex].temperature = 20.0f + ((float)(rand() % 100) / 10.0f);
            gLogBuffer[gLogIndex].voltage     = 5.0f  - ((float)(rand() % 10)  / 10.0f);

            gLogIndex  = (gLogIndex + 1) % MAX_LOGS;
            if (gTotalLogs < MAX_LOGS) gTotalLogs++;
            currentTimeMs += 100;

            xSemaphoreGive(gLogMutex);

            // NOTIFY the UDP task: "A fresh log is ready, send it now!"
            // This is a lightweight direct-to-task signal — no extra memory needed.
            if (gUdpTaskHandle != NULL)
            {
                xTaskNotifyGive(gUdpTaskHandle);
            }
        }

        // Requirement: update data structure every 100ms
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/* ========================================================================== */
/*                    THREAD 2: UDP Send/Receive Task                         */
/* ========================================================================== */
static void AppSocket_udpServerTask(void *pArg)
{
    int32_t sock = -1;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char rx_buffer[RX_BUFFER_SIZE];
    char tx_buffer[TX_BUFFER_SIZE];

    // Store our own task handle so the generator can notify us
    gUdpTaskHandle = xTaskGetCurrentTaskHandle();

    EnetAppUtils_print("UDP Streamer Started on Port %d...\r\n", SERVER_UDP_PORT);

    sock = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        EnetAppUtils_print("Socket creation failed!\r\n");
        vTaskDelete(NULL);
        return;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = PP_HTONS(SERVER_UDP_PORT);
    server_addr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);
    lwip_bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    EnetAppUtils_print("Waiting for client to subscribe...\r\n");

    // Block until a client sends the trigger packet
    lwip_recvfrom(sock, rx_buffer, sizeof(rx_buffer), 0,
                  (struct sockaddr *)&client_addr, &client_len);

    char client_ip[16];
    ip4addr_ntoa_r((const ip4_addr_t *)&client_addr.sin_addr.s_addr,
                   client_ip, sizeof(client_ip));
    EnetAppUtils_print("Client subscribed from %s:%d. Streaming every 100ms...\r\n",
                       client_ip, ntohs(client_addr.sin_port));

    while (1)
    {
        // SLEEP until the generator task wakes us up via xTaskNotifyGive()
        // This task consumes ZERO CPU while waiting — it is fully blocked.
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Grab the latest log from the shared buffer
        if (xSemaphoreTake(gLogMutex, portMAX_DELAY) == pdTRUE)
        {
            // Read the most recently written entry (one step behind current index)
            uint32_t last = (gLogIndex == 0) ? (MAX_LOGS - 1) : (gLogIndex - 1);
            LogItem_t *log = &gLogBuffer[last];

            int len = snprintf(tx_buffer, TX_BUFFER_SIZE,
                "[TS:%u] Type:%d | Temp:%.1fC | Volt:%.1fV\n",
                log->timestamp, log->type,
                log->temperature, log->voltage);

            xSemaphoreGive(gLogMutex);

            // Send the single fresh log immediately
            if (len > 0) {
                lwip_sendto(sock, tx_buffer, len, 0,
                            (struct sockaddr *)&client_addr, client_len);
            }
        }
    }
}

/* ========================================================================== */
/*                            AppSocket_startServer                           */
/* ========================================================================== */
void AppSocket_startServer(void)
{
    gLogMutex = xSemaphoreCreateMutex();
    EnetAppUtils_assert(gLogMutex != NULL);

    // Spawn Thread 2 FIRST so gUdpTaskHandle is set before Thread 1 starts notifying
    sys_thread_new("UdpSrvTask", AppSocket_udpServerTask, NULL,
                   DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

    // Spawn Thread 1: Data Generator
    sys_thread_new("LogGenTask", AppSocket_dataGeneratorTask, NULL,
                   DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
}
