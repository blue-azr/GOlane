package main

/*
#cgo CFLAGS: -I./include/audinate -I./include
#cgo LDFLAGS: -L./lib -ldapi -L./redist -ldns_sd -lcurl -ljansson -lssl -lcrypto -lz -ldl -lpthread -lstdc++ -lm

#include <stdlib.h>

// Dante API 基礎函數聲明
int dante_init(void);
int dante_init_with_interface(const char* interface_name);
void dante_cleanup(void);
const char* dante_get_last_error(void);
int dante_connect_local_device(void);
int dante_is_device_connected(void);
int dante_get_device_name(char* buffer, int buffer_size);
int dante_get_tx_channel_count(void);
int dante_get_rx_channel_count(void);

// 設備掃描函數
int dante_start_device_scan(void);
int dante_stop_device_scan(void);
int dante_get_discovered_device_count(void);
int dante_refresh_device_scan(void);
int dante_process_events_briefly(void);
int dante_get_current_device_list(void);

// 設備資訊結構
struct dante_device_info_t {
    int id;
    char name[64];
    char model[64]; 
    char product_version[32];
    char dante_version[32];
    char ip_address[16];
    int link_speed;
    char secondary_ip[16];
    int secondary_speed;
    char mac_address[18];
    int is_valid;
};

int dante_get_device_info(int index, struct dante_device_info_t* info);
*/
import "C"

import (
	"fmt"
	"log"
	"net"
	"os"
	"os/signal"
	"strings"
	"syscall"
	"time"
	"unsafe"
)

//==============================================================================
// 網路介面檢測和配置
//==============================================================================

// NetworkInterfaceInfo 網路介面資訊
type NetworkInterfaceInfo struct {
	Name       string   // 介面名稱 (eth0, eth1, eth2)
	MacAddress string   // MAC 地址
	IPAddress  string   // IP 地址
	NetMask    string   // 子網路遮罩
	IsUp       bool     // 是否啟用
	HasIP      bool     // 是否有 IP
}

// NetworkDetector 網路檢測器
type NetworkDetector struct {
	AllInterfaces      []NetworkInterfaceInfo
	DanteInterfaces    []NetworkInterfaceInfo
	ManagementInterface *NetworkInterfaceInfo
}

// NewNetworkDetector 創建網路檢測器
func NewNetworkDetector() *NetworkDetector {
	return &NetworkDetector{
		AllInterfaces:   []NetworkInterfaceInfo{},
		DanteInterfaces: []NetworkInterfaceInfo{},
	}
}

// DetectAllInterfaces 檢測所有網路介面
func (nd *NetworkDetector) DetectAllInterfaces() error {
	interfaces, err := net.Interfaces()
	if err != nil {
		return fmt.Errorf("failed to get network interfaces: %v", err)
	}

	log.Println("🔍 Detecting network interfaces...")
	
	for _, iface := range interfaces {
		// 跳過 loopback
		if iface.Flags&net.FlagLoopback != 0 {
			continue
		}

		info := NetworkInterfaceInfo{
			Name:       iface.Name,
			MacAddress: iface.HardwareAddr.String(),
			IsUp:       iface.Flags&net.FlagUp != 0,
			HasIP:      false,
		}

		// 獲取 IP 地址
		addrs, err := iface.Addrs()
		if err == nil && len(addrs) > 0 {
			for _, addr := range addrs {
				// 只取 IPv4 地址
				if ipnet, ok := addr.(*net.IPNet); ok && ipnet.IP.To4() != nil {
					info.IPAddress = ipnet.IP.String()
					info.NetMask = net.IP(ipnet.Mask).String()
					info.HasIP = true
					break
				}
			}
		}

		nd.AllInterfaces = append(nd.AllInterfaces, info)
		
		log.Printf("  ✓ Found: %s (MAC: %s, IP: %s, Up: %v)", 
			info.Name, info.MacAddress, info.IPAddress, info.IsUp)
	}

	return nil
}

// IdentifyDanteInterfaces 識別 Dante 網路介面
func (nd *NetworkDetector) IdentifyDanteInterfaces(danteInterfaceNames []string) {
	log.Println("🔍 Identifying Dante interfaces...")
	
	for _, info := range nd.AllInterfaces {
		for _, danteName := range danteInterfaceNames {
			if info.Name == danteName {
				nd.DanteInterfaces = append(nd.DanteInterfaces, info)
				log.Printf("  ✓ Dante interface found: %s (%s)", info.Name, info.IPAddress)
			}
		}
	}
	
	if len(nd.DanteInterfaces) == 0 {
		log.Println("  ⚠️  No Dante interfaces found!")
	}
}

// AutoConfigureFromSystem 自動從系統配置網路
func (nd *NetworkDetector) AutoConfigureFromSystem() error {
	// 1. 檢測所有網路介面
	if err := nd.DetectAllInterfaces(); err != nil {
		return err
	}
	
	// 2. 指定 Dante 介面名稱
	danteInterfaceNames := []string{
		"enxf8e43bd6309e",  // Dante1 網卡
		"enxf8e43bd55df6",  // JC add Dante 網卡
		// 未來 Dante2 網卡可以在這裡添加
	}
	
	nd.IdentifyDanteInterfaces(danteInterfaceNames)
	
	return nil
}

// GetDanteConfig 根據檢測結果生成 Dante 配置
func (nd *NetworkDetector) GetDanteConfig(index int) (*NetworkConfig, error) {
	if index >= len(nd.DanteInterfaces) {
		return nil, fmt.Errorf("Dante interface index %d out of range", index)
	}
	
	info := nd.DanteInterfaces[index]
	
	if !info.HasIP {
		return nil, fmt.Errorf("interface %s has no IP address", info.Name)
	}
	
	config := &NetworkConfig{
		InterfaceName: info.Name,
		MacAddress:    info.MacAddress,
		IPAddress:     info.IPAddress,
		NetworkType:   fmt.Sprintf("dante%d", index+1),
		Enabled:       info.IsUp,
	}
	
	return config, nil
}

// GetInterfaceByName 根據名稱獲取介面資訊
func (nd *NetworkDetector) GetInterfaceByName(name string) *NetworkInterfaceInfo {
	for i, info := range nd.AllInterfaces {
		if info.Name == name {
			return &nd.AllInterfaces[i]
		}
	}
	return nil
}

// ValidateInterfaceForDante 驗證介面是否適合用於 Dante
func (nd *NetworkDetector) ValidateInterfaceForDante(interfaceName string) error {
	for _, info := range nd.AllInterfaces {
		if info.Name == interfaceName {
			if !info.IsUp {
				return fmt.Errorf("interface %s is DOWN", interfaceName)
			}
			if !info.HasIP {
				return fmt.Errorf("interface %s has no IP address", interfaceName)
			}
			if info.MacAddress == "" {
				return fmt.Errorf("interface %s has no MAC address", interfaceName)
			}
			return nil
		}
	}
	return fmt.Errorf("interface %s not found", interfaceName)
}

// ListAvailableInterfaces 列出所有可用介面
func (nd *NetworkDetector) ListAvailableInterfaces() {
	fmt.Println("\n📋 Available Network Interfaces:")
	fmt.Println("────────────────────────────────────────────────────────────────")
	fmt.Printf("%-10s %-18s %-15s %-10s\n", "NAME", "MAC", "IP", "STATUS")
	fmt.Println("────────────────────────────────────────────────────────────────")
	
	for _, info := range nd.AllInterfaces {
		status := "DOWN"
		if info.IsUp {
			status = "UP"
		}
		
		ip := info.IPAddress
		if ip == "" {
			ip = "N/A"
		}
		
		fmt.Printf("%-10s %-18s %-15s %-10s\n", 
			info.Name, info.MacAddress, ip, status)
	}
	fmt.Println("────────────────────────────────────────────────────────────────\n")
}

// SuggestNetworkConfiguration 建議網路配置
func (nd *NetworkDetector) SuggestNetworkConfiguration() {
	fmt.Println("💡 Suggested Network Configuration:")
	fmt.Println("════════════════════════════════════════════════════════════════")
	
	// 檢查是否有足夠的介面
	upInterfaces := 0
	for _, info := range nd.AllInterfaces {
		if info.IsUp && info.HasIP {
			upInterfaces++
		}
	}
	
	if upInterfaces < 3 {
		fmt.Printf("⚠️  Warning: Only %d interfaces are UP with IP. RTD1619B requires 3 interfaces.\n", upInterfaces)
		fmt.Println("\nRecommended setup:")
		fmt.Println("  • eth0: Management (Telnet) - External network")
		fmt.Println("  • eth1: Dante Domain 1 - Audio network 1")
		fmt.Println("  • eth2: Dante Domain 2 - Audio network 2")
	} else {
		fmt.Println("✓ Sufficient interfaces available")
		
		// 建議配置
		fmt.Println("\nSuggested assignment:")
		count := 0
		for _, info := range nd.AllInterfaces {
			if !info.IsUp || !info.HasIP {
				continue
			}
			
			role := ""
			switch count {
			case 0:
				role = "Management (Telnet)"
			case 1:
				role = "Dante Domain 1"
			case 2:
				role = "Dante Domain 2"
			default:
				role = "Unused"
			}
			
			if role != "Unused" {
				fmt.Printf("  • %s (%s) → %s\n", info.Name, info.IPAddress, role)
			}
			count++
		}
	}
	
	fmt.Println("════════════════════════════════════════════════════════════════\n")
}

// CheckNetworkIsolation 檢查 Dante 網路是否隔離
func (nd *NetworkDetector) CheckNetworkIsolation() {
	if len(nd.DanteInterfaces) < 2 {
		return
	}
	
	fmt.Println("🔒 Checking network isolation...")
	
	dante1IP := nd.DanteInterfaces[0].IPAddress
	dante2IP := nd.DanteInterfaces[1].IPAddress
	
	dante1Net := strings.Join(strings.Split(dante1IP, ".")[0:3], ".")
	dante2Net := strings.Join(strings.Split(dante2IP, ".")[0:3], ".")
	
	if dante1Net == dante2Net {
		fmt.Println("  ⚠️  WARNING: Dante1 and Dante2 are on the same network segment!")
		fmt.Println("  This may cause broadcast storms and interference.")
		fmt.Println("  Recommended: Use different network segments (e.g., 10.1.0.x and 10.2.0.x)")
	} else {
		fmt.Println("  ✓ Dante networks are properly isolated")
	}
	fmt.Println()
}

//==============================================================================
// 核心網路配置
//==============================================================================

// NetworkConfig 網路介面配置
type NetworkConfig struct {
	InterfaceName string // 網路介面名稱 (eth1)
	MacAddress    string // MAC 地址
	IPAddress     string // IP 地址
	NetworkType   string // "dante1"
	Enabled       bool   // 是否啟用
}

//==============================================================================
// Dante 網域管理器
//==============================================================================

// DanteDomain 代表一個 Dante 網域
type DanteDomain struct {
	Name          string
	NetworkConfig NetworkConfig
	Initialized   bool
	DeviceCount   int
}

// NewDanteDomain 創建新的 Dante 網域
func NewDanteDomain(name string, config NetworkConfig) *DanteDomain {
	return &DanteDomain{
		Name:          name,
		NetworkConfig: config,
		Initialized:   false,
		DeviceCount:   0,
	}
}

// Initialize 初始化 Dante 網域
func (d *DanteDomain) Initialize() error {
	log.Printf("🔧 Initializing Dante Domain: %s on %s (%s)", 
		d.Name, d.NetworkConfig.InterfaceName, d.NetworkConfig.IPAddress)
	
	// 傳遞網卡名稱給 Dante SDK
	interfaceName := C.CString(d.NetworkConfig.InterfaceName)
	defer C.free(unsafe.Pointer(interfaceName))
	
	result := C.dante_init_with_interface(interfaceName)
	if result != 0 {
		errorMsg := C.GoString(C.dante_get_last_error())
		return fmt.Errorf("dante_init_with_interface failed: %s", errorMsg)
	}
	
	log.Printf("✅ Dante API initialized on %s", d.NetworkConfig.InterfaceName)
	
	d.Initialized = true
	log.Printf("✅ Dante Domain %s ready for network scanning", d.Name)
	return nil
}

// StartDeviceScan 開始設備掃描
func (d *DanteDomain) StartDeviceScan() error {
	if !d.Initialized {
		return fmt.Errorf("domain %s not initialized", d.Name)
	}
	
	log.Printf("🔍 [%s] Starting device scan on %s", d.Name, d.NetworkConfig.InterfaceName)
	
	// 調用 Dante SDK 開始設備掃描
	result := C.dante_start_device_scan()
	if result != 0 {
		errorMsg := C.GoString(C.dante_get_last_error())
		return fmt.Errorf("dante_start_device_scan failed: %s", errorMsg)
	}
	
	log.Printf("✅ Device scan started")
	
	// 啟動背景事件處理
	go d.processEventsLoop()
	
	return nil
}

// processEventsLoop 背景事件處理循環
func (d *DanteDomain) processEventsLoop() {
	ticker := time.NewTicker(500 * time.Millisecond)
	defer ticker.Stop()
	
	for d.Initialized {
		select {
		case <-ticker.C:
			C.dante_process_events_briefly()
		}
	}
}

// RefreshDevices 刷新設備列表
func (d *DanteDomain) RefreshDevices() {
	if !d.Initialized {
		return
	}
	
	log.Printf("🔄 [%s] Refreshing device list...", d.Name)
	
	// 刷新掃描結果
	C.dante_refresh_device_scan()
	
	// 獲取設備數量
	d.DeviceCount = int(C.dante_get_discovered_device_count())
	
	log.Printf("📊 [%s] Found %d devices", d.Name, d.DeviceCount)
}

// ShowDevices 顯示設備列表
func (d *DanteDomain) ShowDevices() {
	fmt.Printf("\n=== %s Device List ===\n", d.Name)
	fmt.Printf("Interface: %s (%s)\n", d.NetworkConfig.InterfaceName, d.NetworkConfig.IPAddress)
	fmt.Printf("Total Devices: %d\n", d.DeviceCount)
	
	if d.DeviceCount > 0 {
		fmt.Println("\nID  Name                 Model            IP Address       MAC Address       Dante Ver")
		fmt.Println("─────────────────────────────────────────────────────────────────────────────────────────")
		
		for i := 0; i < d.DeviceCount; i++ {
			var cInfo C.struct_dante_device_info_t
			
			result := C.dante_get_device_info(C.int(i), &cInfo)
			if result != 0 {
				continue
			}
			
			fmt.Printf("%-3d %-20s %-16s %-16s %-17s %s\n",
				int(cInfo.id),
				C.GoString(&cInfo.name[0]),
				C.GoString(&cInfo.model[0]),
				C.GoString(&cInfo.ip_address[0]),
				C.GoString(&cInfo.mac_address[0]),
				C.GoString(&cInfo.dante_version[0]))
		}
	}
	
	fmt.Println("==========================\n")
}

// Cleanup 清理資源
func (d *DanteDomain) Cleanup() {
	if d.Initialized {
		log.Printf("🧹 Cleaning up Dante Domain: %s", d.Name)
		C.dante_stop_device_scan()
		C.dante_cleanup()
		d.Initialized = false
	}
}

//==============================================================================
// 主函數
//==============================================================================

func main() {
	// 打印啟動橫幅
	fmt.Println("=========================================")
	fmt.Println("   RTD1619B Dante Single Network Test")
	fmt.Println("   Version: 1.0.0")
	fmt.Println("=========================================")
	fmt.Println()
	
	// ============================================
	// 步驟 1: 網路介面自動檢測
	// ============================================
	log.Println("Step 1: Network Interface Detection")
	detector := NewNetworkDetector()
	
	if err := detector.AutoConfigureFromSystem(); err != nil {
		log.Fatalf("❌ Network detection failed: %v", err)
	}
	
	// 列出所有可用介面
	detector.ListAvailableInterfaces()
	
	// 網路配置建議
	detector.SuggestNetworkConfiguration()
	
	// ============================================
	// 步驟 2: 選擇 Dante 介面
	// ============================================
	log.Println("Step 2: Configure Dante Interface")
	
	var config *NetworkConfig
	var err error
	
	// 使用檢測到的 Dante 介面
	if len(detector.DanteInterfaces) > 0 {
		log.Printf("✓ Using Dante interface: %s", detector.DanteInterfaces[0].Name)
		config, err = detector.GetDanteConfig(0)
		if err != nil {
			log.Fatalf("❌ Failed to get Dante config: %v", err)
		}
	} else {
		log.Fatal("❌ Dante interface 'enxf8e43bd6309e' not found. Please check network connection.")
	}
	
	// 顯示選定的配置
	fmt.Println("\n✓ Selected Dante Configuration:")
	fmt.Printf("  Interface: %s\n", config.InterfaceName)
	fmt.Printf("  IP:        %s\n", config.IPAddress)
	fmt.Printf("  MAC:       %s\n", config.MacAddress)
	fmt.Printf("  Enabled:   %v\n", config.Enabled)
	fmt.Println()
	
	// 設置信號處理
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)
	
	// ============================================
	// 步驟 3: 初始化 Dante
	// ============================================
	log.Println("Step 3: Initializing Dante API...")
	dante1 := NewDanteDomain("Dante1", *config)
	
	if err := dante1.Initialize(); err != nil {
		log.Fatalf("❌ Initialization failed: %v", err)
	}
	
	// ============================================
	// 步驟 4: 開始設備掃描
	// ============================================
	log.Println("Step 4: Starting device scan...")
	if err := dante1.StartDeviceScan(); err != nil {
		log.Printf("⚠️  Device scan warning: %v", err)
	}
	
	// ============================================
	// 步驟 5: 等待設備發現
	// ============================================
	log.Println("Step 5: Waiting for device discovery...")
	time.Sleep(3 * time.Second)
	
	// ============================================
	// 步驟 6: 刷新設備列表
	// ============================================
	log.Println("Step 6: Refreshing device list...")
	dante1.RefreshDevices()
	
	// ============================================
	// 步驟 7: 顯示設備
	// ============================================
	dante1.ShowDevices()
	
	// 持續運行
	log.Println("✅ System ready. Press Ctrl+C to exit")
	
	// 定期刷新設備列表
	ticker := time.NewTicker(10 * time.Second)
	go func() {
		for range ticker.C {
			dante1.RefreshDevices()
			dante1.ShowDevices()
		}
	}()
	
	// 等待退出信號
	<-sigChan
	fmt.Println("\n\n🛑 Shutting down...")
	ticker.Stop()
	
	// 清理 Dante 資源
	dante1.Cleanup()
	
	log.Println("✅ Shutdown completed")
}
