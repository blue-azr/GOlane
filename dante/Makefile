# Makefile for RTD1619B Dante Network System

GO = go
CC = gcc

# Dante SDK 路徑 (當前目錄)
DANTE_BASE = .

# Dante API 編譯參數
DAPI_INC = -I$(DANTE_BASE)/include/audinate -I$(DANTE_BASE)/include
DAPI_LIBS = $(DANTE_BASE)/lib/libdapi.a $(DANTE_BASE)/lib/libcurl.a $(DANTE_BASE)/lib/libjansson.a $(DANTE_BASE)/lib/libssl.a $(DANTE_BASE)/lib/libcrypto.a $(DANTE_BASE)/lib/libz.a -L$(DANTE_BASE)/redist -ldns_sd -ldl -lpthread -lstdc++ -lm

DAPI_WARNS = -Wall -Wundef -Wcast-align -Wwrite-strings -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations
DAPI_CFLAGS = -g -std=c99 $(DAPI_WARNS) -fPIC

# 目標檔案
TARGET_GO = danteCS
WRAPPER_LIB = libdante_wrapper.a
WRAPPER_SRC = dante_wrapper.c
GO_SRC = main.go

.PHONY: all clean wrapper run help

all: wrapper $(TARGET_GO)

# 編譯 C wrapper 靜態庫
wrapper: $(WRAPPER_LIB)

$(WRAPPER_LIB): $(WRAPPER_SRC)
	@echo "🔨 Building C wrapper library..."
	$(CC) $(DAPI_CFLAGS) $(DAPI_INC) -c -o dante_wrapper.o $(WRAPPER_SRC)
	ar rcs $@ dante_wrapper.o
	@echo "✅ C wrapper library built: $(WRAPPER_LIB)"

# 編譯 Go 程式（使用 CGO）
$(TARGET_GO): $(GO_SRC) $(WRAPPER_LIB)
	@echo "🔨 Building Go application with Dante SDK..."
	CGO_CFLAGS="$(DAPI_INC)" \
	CGO_LDFLAGS="-L. -ldante_wrapper $(DAPI_LIBS)" \
	$(GO) build -o $(TARGET_GO) $(GO_SRC)
	@echo "✅ Go application built: $(TARGET_GO)"

# 運行程式
run: $(TARGET_GO)
	@echo "🚀 Starting RTD1619B Dante Network System..."
	./$(TARGET_GO)

# 清理
clean:
	@echo "🧹 Cleaning build files..."
	rm -f $(TARGET_GO) $(WRAPPER_LIB) *.o
	@echo "✅ Clean completed"

# 檢查環境
check-env:
	@echo "=== Environment Check ==="
	@echo "Current directory: $(shell pwd)"
	@echo "Dante SDK path: $(DANTE_BASE)"
	@test -d $(DANTE_BASE)/include && echo "✓ include directory found" || (echo "✗ include directory not found" && exit 1)
	@test -d $(DANTE_BASE)/lib && echo "✓ lib directory found" || (echo "✗ lib directory not found" && exit 1)
	@test -d $(DANTE_BASE)/redist && echo "✓ redist directory found" || (echo "✗ redist directory not found" && exit 1)
	@test -f $(WRAPPER_SRC) && echo "✓ C wrapper source found" || (echo "✗ C wrapper source not found" && exit 1)
	@test -f $(GO_SRC) && echo "✓ Go source found" || (echo "✗ Go source not found" && exit 1)
	@echo "========================="

# 幫助
help:
	@echo "Available targets:"
	@echo "  all       - Build C wrapper and Go application"
	@echo "  wrapper   - Build only C wrapper library"
	@echo "  run       - Build and run the application"
	@echo "  clean     - Remove build files"
	@echo "  check-env - Check build environment"
	@echo "  help      - Show this help"