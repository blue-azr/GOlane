/*
 * dante_wrapper.c
 * 基礎 Dante API C Wrapper for Go integration
 * 
 * 支援背景設備掃描和自動列表更新
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
// Dante API headers
#include "audinate/dante_api.h"
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

// 函數宣告
int dante_init(void);
int dante_init_with_interface(const char* interface_name);
void dante_cleanup(void);
const char* dante_get_last_error(void);
int dante_connect_local_device(void);
int dante_is_device_connected(void);
int dante_get_device_name(char* buffer, int buffer_size);
int dante_get_tx_channel_count(void);
int dante_get_rx_channel_count(void);
int dante_get_tx_channel_name(int channel_index, char* buffer, int buffer_size);
int dante_run_basic_test(void);

// 設備資訊結構定義
typedef struct {
    int id;
    char name[64];
    char model[64]; 
    char product_version[32];
    char dante_version[32];
    char ip_address[16];
    int link_speed;
    char secondary_ip[16];
    int secondary_speed;
    int is_valid;
} dante_device_info_t;

// 新增的背景掃描功能
int dante_start_device_scan(void);
int dante_stop_device_scan(void);
int dante_get_discovered_device_count(void);
int dante_get_device_info(int index, dante_device_info_t* info);
int dante_refresh_device_scan(void);
int dante_process_events_briefly(void);
int dante_get_current_device_list(void);

// 全域變數
static dapi_t* g_dapi = NULL;
static dante_runtime_t* g_runtime = NULL;
static dr_devices_t* g_devices = NULL;
static dr_device_t* g_device = NULL;
static aud_env_t* g_env = NULL;

// 設備瀏覽相關
static db_browse_t* g_browse = NULL;
static db_browse_config_t g_browse_config;
static int g_device_scan_active = 0;
static int g_background_scanning = 0;

// 回調函數和狀態追蹤
static int g_device_ready = 0;

// 錯誤處理
static char g_error_buffer[256];

// 設備列表管理
#define MAX_DEVICES 32
static dante_device_info_t g_discovered_devices[MAX_DEVICES];
static int g_device_count = 0;

//==============================================================================
// 回調函數 - 自動更新設備列表
//==============================================================================
/**
 * 網路變更回調函數 - 當有設備加入/離開時自動更新列表
 * 修復 Auto-IP 環境下的 IP 和 MAC 地址獲取問題
 */
static void browse_network_changed_callback(const db_browse_t* browse) {
    printf("Network changed - auto-updating device list\n");
    
    // 清空現有列表
    memset(g_discovered_devices, 0, sizeof(g_discovered_devices));
    g_device_count = 0;
    
    const db_browse_network_t* network = db_browse_get_network(browse);
    if (!network) {
        return;
    }
    
    uint16_t device_count = db_browse_network_get_num_devices(network);
    
    for (uint16_t i = 0; i < device_count && i < MAX_DEVICES; i++) {
        const db_browse_device_t* device = db_browse_network_device_at_index(network, i);
        if (!device) continue;
        
        dante_device_info_t* info = &g_discovered_devices[g_device_count];
        
        // 填充設備資訊
        info->id = g_device_count + 1;
        info->is_valid = 1;
        
        // 設備名稱
        const char* name = db_browse_device_get_name(device);
        if (name) {
            snprintf(info->name, sizeof(info->name), "%s", name);
        } else {
            snprintf(info->name, sizeof(info->name), "Unknown Device %d", info->id);
        }
        
        // 預設名稱（通常是型號）
        // 嘗試獲取更好的型號資訊
        const char* router_info = db_browse_device_get_router_info(device);
        const dante_id64_t* mf_id = db_browse_device_get_manufacturer_id(device);
        const dante_id64_t* model_id = db_browse_device_get_model_id(device);
        const char* default_name = db_browse_device_get_default_name(device);

        if (router_info && strlen(router_info) > 0) {
            // 優先使用 router_info (如 "ULTIMOX4")
            snprintf(info->model, sizeof(info->model), "%s", router_info);
        } else if (mf_id && model_id) {
            // 次選：組合製造商和型號 ID
            char mf_buf[DANTE_ID64_DNSSD_BUF_LENGTH];
            char model_buf[DANTE_ID64_DNSSD_BUF_LENGTH];
            dante_id64_to_dnssd_text(mf_id, mf_buf);
            dante_id64_to_dnssd_text(model_id, model_buf);
            snprintf(info->model, sizeof(info->model), "%s-%s", mf_buf, model_buf);
        } else if (default_name) {
            // 最後選擇：使用 default_name
            snprintf(info->model, sizeof(info->model), "%s", default_name);
        } else {
            snprintf(info->model, sizeof(info->model), "Unknown Model");
        }

        // 版本資訊
        const dante_version_t* router_version = db_browse_device_get_router_version(device);
        if (router_version) {
            snprintf(info->dante_version, sizeof(info->dante_version), 
                    "%u.%u.%u", router_version->major, router_version->minor, router_version->bugfix);
        } else {
            snprintf(info->dante_version, sizeof(info->dante_version), "Unknown");
        }
        
        // 修正其他無法獲得的欄位
        snprintf(info->product_version, sizeof(info->product_version), "N/A");

        //IP
      
      printf("[DEBUG] Getting IP for device '%s' using routing API...\n", info->name);
        
        dr_device_t* routing_device = NULL;
        aud_error_t result = dr_device_open_remote(g_devices, info->name, &routing_device);
        
        if (result == AUD_SUCCESS && routing_device) {
            printf("[DEBUG] Successfully opened routing connection to '%s'\n", info->name);
            
            // 等待設備解析完成（最多等 3 秒）
            int max_wait_attempts = 30; // 3 秒，每次 100ms
            dr_device_state_t state;
            
            for (int attempt = 0; attempt < max_wait_attempts; attempt++) {
                state = dr_device_get_state(routing_device);
                
                if (state == DR_DEVICE_STATE_RESOLVED || state == DR_DEVICE_STATE_ACTIVE) {
                    printf("[DEBUG] Device '%s' resolved after %d attempts (state: %d)\n", 
                           info->name, attempt, state);
                    break;
                }
                
                if (state == DR_DEVICE_STATE_ERROR) {
                    printf("[ERROR] Device '%s' entered error state\n", info->name);
                    break;
                }
                
                // 處理一些 runtime 事件，讓解析繼續
                if (g_runtime) {
                    dante_runtime_process(g_runtime);
                }
                
                usleep(100000); // 等待 100ms
            }
            
            // 現在嘗試取得 IP 位址
            if (state == DR_DEVICE_STATE_RESOLVED || state == DR_DEVICE_STATE_ACTIVE) {
                dante_ipv4_address_t device_address;
                aud_error_t addr_result = dr_device_get_address(routing_device, &device_address);
                
                if (addr_result == AUD_SUCCESS) {
                    uint32_t ip_addr = ntohl(device_address.host);
                    snprintf(info->ip_address, sizeof(info->ip_address), 
                            "%u.%u.%u.%u", 
                            (ip_addr >> 24) & 0xFF,
                            (ip_addr >> 16) & 0xFF,
                            (ip_addr >> 8) & 0xFF,
                            ip_addr & 0xFF);
                    
                    printf("[INFO] Device '%s' IP: %s\n", info->name, info->ip_address);
                } else {
                    printf("[ERROR] Failed to get address for device '%s': %d\n", info->name, addr_result);
                    snprintf(info->ip_address, sizeof(info->ip_address), "0.0.0.0");
                }
            } else {
                printf("[WARN] Device '%s' did not resolve in time (final state: %d)\n", info->name, state);
                snprintf(info->ip_address, sizeof(info->ip_address), "0.0.0.0");
            }
            
            // 關閉 routing device 連接
            dr_device_close(routing_device);
            
        } else {
            printf("[ERROR] Failed to open routing connection to device '%s': %d\n", info->name, result);
            snprintf(info->ip_address, sizeof(info->ip_address), "0.0.0.0");
        }




                info->link_speed = -1;  // 用 -1 表示無效
                g_device_count++;
            }
            
            printf("Device list updated - now has %d devices\n", g_device_count);
        }


//==============================================================================
// 基礎初始化和清理
//==============================================================================

/**
 * 初始化 Dante API 環境
 * @return 0 成功, -1 失敗
 */

int dante_init(void) {
    // 使用 NULL 或空字串表示使用預設介面
    return dante_init_with_interface(NULL);
}

int dante_init_with_interface(const char* interface_name) {
    aud_error_t result;
    
    printf("Initializing Dante API...\n");
    
    // 建立 DAPI 環境
    result = dapi_new(&g_dapi);
    if (result != AUD_SUCCESS) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), 
                "Failed to create DAPI: %d", result);
        return -1;
    }
   
    // 取得 runtime 和 env
    g_env = dapi_get_env(g_dapi);
    g_runtime = dapi_get_runtime(g_dapi);
    if (!g_runtime || !g_env) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Failed to get runtime/env");
        dapi_delete(g_dapi);
        g_dapi = NULL;
        return -1;
    }
    
    // 建立設備管理器
    result = dr_devices_new_dapi(g_dapi, &g_devices);
    if (result != AUD_SUCCESS) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), 
                "Failed to create device manager: %d", result);
        dapi_delete(g_dapi);
        g_dapi = NULL;
        return -1;
    }
    
    // 初始化瀏覽配置
    db_browse_config_init_defaults(&g_browse_config);

     // 配置要使用的網卡
    if (interface_name && interface_name[0] != '\0') {
        printf("[INFO] Configuring browse to use interface: %s\n", interface_name);
        
        // 將網卡名稱轉換為 interface index
        aud_interface_identifier_t iface;
        memset(&iface, 0, sizeof(iface));
        iface.flags = AUD_INTERFACE_IDENTIFIER_FLAG_NAME;
        aud_strlcpy(iface.name, interface_name, AUD_INTERFACE_NAME_LENGTH);
        
        // 轉換名稱為 index
        result = aud_interface_get_identifiers(g_env, &iface, 1);
        if (result != AUD_SUCCESS) {
            printf("[WARN] Failed to resolve interface '%s': %d\n", interface_name, result);
            printf("[WARN] Will use default network settings\n");
        } else {
            // 設置到 browse config
            g_browse_config.interface_indexes[0] = iface.index;
            g_browse_config.num_interface_indexes = 1;
            printf("[INFO] Interface '%s' resolved to index %u\n", interface_name, iface.index);
        }
    } else {
        printf("[INFO] Using default network interface (auto-select)\n");
    }
    
    printf("Dante API initialized successfully\n");
    return 0;
}

/**
 * 清理 Dante API 資源
 */
void dante_cleanup(void) {
    printf("Cleaning up Dante API...\n");
    
    // 停止設備掃描
    if (g_browse) {
        dante_stop_device_scan();
    }
    
    if (g_device) {
        dr_device_close(g_device);
        g_device = NULL;
    }
    
    if (g_devices) {
        dr_devices_delete(g_devices);
        g_devices = NULL;
    }
    
    if (g_dapi) {
        dapi_delete(g_dapi);
        g_dapi = NULL;
    }
    
    g_runtime = NULL;
    g_env = NULL;
    g_device_ready = 0;
    g_device_scan_active = 0;
    g_background_scanning = 0;
    g_device_count = 0;
    memset(g_discovered_devices, 0, sizeof(g_discovered_devices));
    
    printf("Dante API cleanup completed\n");
}

/**
 * 取得最後錯誤訊息
 */
const char* dante_get_last_error(void) {
    return g_error_buffer;
}

//==============================================================================
// 設備連接和管理
//==============================================================================

/**
 * 連接到本地 Dante 設備
 * @return 0 成功, -1 失敗
 */
int dante_connect_local_device(void) {
    aud_error_t result;
    
    if (!g_devices) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Dante not initialized");
        return -1;
    }
    
    printf("Connecting to local Dante device...\n");
    
    // 開啟本地設備連接
    result = dr_device_open_local(g_devices, &g_device);
    if (result != AUD_SUCCESS) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), 
                "Failed to connect to local device: %d", result);
        return -1;
    }
    
    // 等待設備就緒
    int timeout = 50; // 5秒超時
    while (timeout-- > 0) {
        dr_device_state_t state = dr_device_get_state(g_device);
        if (state == DR_DEVICE_STATE_ACTIVE) {
            g_device_ready = 1;
            printf("Local device connected successfully\n");
            return 0;
        }
        sleep(1); // 1秒
    }
    
    snprintf(g_error_buffer, sizeof(g_error_buffer), "Device connection timeout");
    return -1;
}

/**
 * 檢查設備是否已連接
 * @return 1 已連接, 0 未連接
 */
int dante_is_device_connected(void) {
    if (!g_device) return 0;
    
    dr_device_state_t state = dr_device_get_state(g_device);
    return (state == DR_DEVICE_STATE_ACTIVE) ? 1 : 0;
}

/**
 * 取得設備名稱
 * @param buffer 輸出緩衝區
 * @param buffer_size 緩衝區大小
 * @return 0 成功, -1 失敗
 */
int dante_get_device_name(char* buffer, int buffer_size) {
    if (!g_device || !dante_is_device_connected()) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Device not connected");
        return -1;
    }
    
    const char* name = dr_device_get_name(g_device);
    if (!name) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Failed to get device name");
        return -1;
    }
    
    snprintf(buffer, buffer_size, "%s", name);
    return 0;
}

//==============================================================================
// 基礎路由資訊
//==============================================================================

/**
 * 取得 TX 通道數量
 * @return 通道數量, -1 表示錯誤
 */
int dante_get_tx_channel_count(void) {
    if (!g_device || !dante_is_device_connected()) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Device not connected");
        return -1;
    }
    
    return dr_device_num_txchannels(g_device);
}

/**
 * 取得 RX 通道數量
 * @return 通道數量, -1 表示錯誤
 */
int dante_get_rx_channel_count(void) {
    if (!g_device || !dante_is_device_connected()) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Device not connected");
        return -1;
    }
    
    return dr_device_num_rxchannels(g_device);
}

/**
 * 取得 TX 通道名稱
 * @param channel_index 通道索引 (0-based)
 * @param buffer 輸出緩衝區
 * @param buffer_size 緩衝區大小
 * @return 0 成功, -1 失敗
 */
int dante_get_tx_channel_name(int channel_index, char* buffer, int buffer_size) {
    if (!g_device || !dante_is_device_connected()) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Device not connected");
        return -1;
    }
    
    dr_txchannel_t* tx_channel = dr_device_txchannel_at_index(g_device, channel_index);
    if (!tx_channel) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Invalid TX channel index: %d", channel_index);
        return -1;
    }
    
    const char* name = dr_txchannel_get_canonical_name(tx_channel);
    if (!name) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Failed to get TX channel name");
        return -1;
    }
    
    snprintf(buffer, buffer_size, "%s", name);
    return 0;
}

//==============================================================================
// 背景設備掃描和發現功能
//==============================================================================

/**
 * 啟動背景設備掃描（非阻塞）
 * @return 0 成功, -1 失敗
 */
int dante_start_device_scan(void) {
    aud_error_t result;
    
    if (!g_env) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Dante API not initialized");
        return -1;
    }
    
    if (g_browse) {
        printf("Device scan already active\n");
        return 0;
    }
    
    printf("Starting background device scan...\n");
    
    // 建立瀏覽物件，掃描媒體設備和控制設備
    db_browse_types_t browse_types = DB_BROWSE_TYPE_MEDIA_DEVICE | DB_BROWSE_TYPE_CONMON_DEVICE;
    
    result = db_browse_new(g_env, browse_types, &g_browse);
    if (result != AUD_SUCCESS) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), 
                "Failed to create browse object: %d", result);
        return -1;
    }
    
    // 設置最大 socket 數量
    result = db_browse_set_max_sockets(g_browse, 32);
    if (result != AUD_SUCCESS) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), 
                "Failed to set max sockets: %d", result);
        db_browse_delete(g_browse);
        g_browse = NULL;
        return -1;
    }
    
    // 設置回調函數 - 關鍵！自動更新列表
    db_browse_set_network_changed_callback(g_browse, browse_network_changed_callback);
    
    // 使用配置啟動瀏覽
    result = db_browse_start_config(g_browse, &g_browse_config);
    if (result != AUD_SUCCESS) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), 
                "Failed to start browse: %d", result);
        db_browse_delete(g_browse);
        g_browse = NULL;
        return -1;
    }
    
    g_device_scan_active = 1;
    g_background_scanning = 1;
    printf("Background device scan started successfully\n");
    
    return 0;
}

/**
 * 停止設備掃描
 * @return 0 成功, -1 失敗
 */
int dante_stop_device_scan(void) {
    if (!g_browse) {
        return 0; // 已經停止
    }
    
    printf("Stopping device scan...\n");
    
    db_browse_stop(g_browse);
    db_browse_delete(g_browse);
    g_browse = NULL;
    g_device_scan_active = 0;
    g_background_scanning = 0;
    
    printf("Device scan stopped\n");
    return 0;
}

/**
 * GO 調用這個來處理事件（短時間，非阻塞）
 * @return 0 成功, -1 失敗
 */
int dante_process_events_briefly(void) {
    if (!g_runtime || !g_background_scanning) {
        return 0;
    }
    
    // 只處理短時間的事件，然後立即返回
    for (int i = 0; i < 5; i++) {  // 0.5秒
        aud_error_t result = dante_runtime_process(g_runtime);
        if (result != AUD_SUCCESS && result != AUD_ERR_DONE) {
            // 忽略非嚴重錯誤
        }
        usleep(100000); // 100ms
    }
    
    return 0;
}

/**
 * GO 調用這個來取得當前設備列表（立即返回）
 * @return 設備數量
 */
int dante_get_current_device_list(void) {
    // 不做掃描，只返回當前已知的設備數量
    return g_device_count;
}

/**
 * 手動觸發設備列表更新
 * @return 0 成功, -1 失敗
 */
int dante_refresh_device_scan(void) {
    // 這個函數現在只是手動觸發一次列表更新
    if (g_browse) {
        browse_network_changed_callback(g_browse);
    }
    return 0;
}

/**
 * 取得發現的設備數量
 * @return 設備數量
 */
int dante_get_discovered_device_count(void) {
    return g_device_count;
}

/**
 * 取得指定設備的詳細資訊
 * @param index 設備索引 (0-based)
 * @param info 輸出的設備資訊結構
 * @return 0 成功, -1 失敗
 */
int dante_get_device_info(int index, dante_device_info_t* info) {
    if (!info) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Invalid info pointer");
        return -1;
    }
    
    if (index < 0 || index >= g_device_count) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), 
                "Invalid device index: %d (available: 0-%d)", index, g_device_count - 1);
        return -1;
    }
    
    if (!g_discovered_devices[index].is_valid) {
        snprintf(g_error_buffer, sizeof(g_error_buffer), "Device at index %d is not valid", index);
        return -1;
    }
    
    // 複製設備資訊
    *info = g_discovered_devices[index];
    return 0;
}

//==============================================================================
// 測試/除錯函數
//==============================================================================

/**
 * 執行基礎系統測試
 * @return 0 所有測試通過, -1 有測試失敗
 */
int dante_run_basic_test(void) {
    printf("\n=== Dante Basic Test ===\n");
    
    // 測試 1: 初始化
    printf("Test 1: Initialization... ");
    if (dante_init() != 0) {
        printf("FAILED: %s\n", dante_get_last_error());
        return -1;
    }
    printf("PASSED\n");
    
    // 測試 2: 設備連接
    printf("Test 2: Device connection... ");
    if (dante_connect_local_device() != 0) {
        printf("FAILED: %s\n", dante_get_last_error());
        dante_cleanup();
        return -1;
    }
    printf("PASSED\n");
    
    // 測試 3: 設備資訊
    printf("Test 3: Device info... ");
    char device_name[64];
    if (dante_get_device_name(device_name, sizeof(device_name)) == 0) {
        printf("PASSED (Device: %s)\n", device_name);
    } else {
        printf("FAILED: %s\n", dante_get_last_error());
        dante_cleanup();
        return -1;
    }
    
    // 測試 4: 通道數量
    printf("Test 4: Channel counts... ");
    int tx_count = dante_get_tx_channel_count();
    int rx_count = dante_get_rx_channel_count();
    if (tx_count >= 0 && rx_count >= 0) {
        printf("PASSED (TX: %d, RX: %d)\n", tx_count, rx_count);
    } else {
        printf("FAILED: %s\n", dante_get_last_error());
        dante_cleanup();
        return -1;
    }
    
    // 測試 5: 第一個 TX 通道名稱
    if (tx_count > 0) {
        printf("Test 5: First TX channel name... ");
        char channel_name[64];
        if (dante_get_tx_channel_name(0, channel_name, sizeof(channel_name)) == 0) {
            printf("PASSED (Channel 0: %s)\n", channel_name);
        } else {
            printf("FAILED: %s\n", dante_get_last_error());
        }
    }
    
    // 測試 6: 設備掃描
    printf("Test 6: Device scan... ");
    if (dante_start_device_scan() == 0) {
        printf("PASSED\n");
        
        // 等待設備被發現
        printf("Waiting for devices to be discovered (5 seconds)...\n");
        sleep(5);
        
        // 刷新掃描結果
        printf("Test 7: Refresh scan results... ");
        if (dante_refresh_device_scan() == 0) {
            int device_count = dante_get_discovered_device_count();
            printf("PASSED (Found %d devices)\n", device_count);
            
            // 顯示每個設備的詳細資訊
            for (int i = 0; i < device_count; i++) {
                dante_device_info_t info;
                if (dante_get_device_info(i, &info) == 0) {
                    printf("  Device %d: %s (%s) - Dante %s\n", 
                           i, info.name, info.model, info.dante_version);
                }
            }
        } else {
            printf("FAILED: %s\n", dante_get_last_error());
        }
        
        dante_stop_device_scan();
    } else {
        printf("FAILED: %s\n", dante_get_last_error());
    }
    
    printf("\n=== All Tests Completed ===\n");
    
    // 保持連接，不清理，讓後續 API 可以使用
    return 0;
}

//==============================================================================
// 主要測試入口點 (可選)
//==============================================================================

#ifdef DANTE_WRAPPER_STANDALONE
int main(int argc, char* argv[]) {
    printf("Dante Wrapper Basic Test\n");
    
    int result = dante_run_basic_test();
    
    if (result == 0) {
        printf("\nAll tests passed! Press Enter to exit...\n");
        getchar();
    }
    
    dante_cleanup();
    return result;
}
#endif