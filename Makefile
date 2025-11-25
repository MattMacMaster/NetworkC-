# =============================
# Compiler and flags
# =============================
CXX       := g++
CXXFLAGS  := -Wall -O2 -MMD -MP
LDFLAGS   := -lX11
CXX_INC   := -Iinclude -Itest

# =============================
# Directories
# =============================
SRC_DIR   := src
TEST_DIR  := test
BUILD_DIR := build

# =============================
# Source discovery
# =============================
SRC_CC    := $(wildcard $(SRC_DIR)/*.cc)
SRC_CPP   := $(wildcard $(SRC_DIR)/*.cpp)
SRC_FILES := $(SRC_CC) $(SRC_CPP)

OBJ_CC    := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SRC_CC))
OBJ_CPP   := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_CPP))
OBJ_FILES := $(OBJ_CC) $(OBJ_CPP)

# Sender / test separation
SENDER_SRC := $(SRC_DIR)/sender.cpp
SENDER_OBJ := $(BUILD_DIR)/sender.o
CORE_OBJ   := $(filter-out $(SENDER_OBJ),$(OBJ_FILES))

# Test files
TEST_FILES := $(wildcard $(TEST_DIR)/*.cc)
TEST_OBJ   := $(patsubst $(TEST_DIR)/%.cc,$(BUILD_DIR)/%.o,$(TEST_FILES))
TEST_BIN   := $(BUILD_DIR)/test_headers

# Executables
SENDER_BIN := sender

# =============================
# Default rule
# =============================
all: test_headers $(SENDER_BIN)

# =============================
# test_headers binary (build + top-level)
# =============================
$(TEST_BIN): $(CORE_OBJ) $(TEST_OBJ) | $(BUILD_DIR)
	$(CXX) $^ -o $@ -lgtest -lgtest_main -pthread $(LDFLAGS)

# Copy test binary to project root
test_headers: $(TEST_BIN)
	cp $(TEST_BIN) test_headers

# =============================
# sender binary (links -lz)
# =============================
$(SENDER_BIN): $(SENDER_OBJ) $(CORE_OBJ) | $(BUILD_DIR)
	$(CXX) $^ -o $@ $(LDFLAGS) -lz

# =============================
# Compile rules
# =============================
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CXX_INC) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CXX_INC) -c $< -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CXX_INC) -c $< -o $@

# =============================
# Test convenience target
# =============================
test: test_headers
	./test_headers

# =============================
# Directory creation
# =============================
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# =============================
# Cleaning
# =============================
clean:
	rm -rf $(BUILD_DIR) test_headers $(SENDER_BIN)

.PHONY: all clean test
