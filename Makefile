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
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*.cc)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SRC_FILES)))

# Identify sender / receiver objects
SENDER_SRC := $(SRC_DIR)/sender.cpp
RECEIVER_SRC := $(SRC_DIR)/receiver.cpp

SENDER_OBJ := $(BUILD_DIR)/sender.o
RECEIVER_OBJ := $(BUILD_DIR)/receiver.o

# Remove sender.o from core for sender build
CORE_OBJ := $(filter-out $(SENDER_OBJ) $(RECEIVER_OBJ),$(OBJ_FILES))

# =============================
# Test discovery
# =============================
TEST_SRC := $(wildcard $(TEST_DIR)/*.cc)
TEST_OBJ := $(patsubst $(TEST_DIR)/%.cc,$(BUILD_DIR)/%.o,$(TEST_SRC))
TEST_BIN := $(BUILD_DIR)/test_headers

# =============================
# Executables
# =============================
SENDER_BIN := sender
RECEIVER_BIN := receiver

# =============================
# Default rule
# =============================
all: $(SENDER_BIN) $(RECEIVER_BIN) test_headers

# =============================
# Build sender
# =============================
$(SENDER_BIN): $(SENDER_OBJ) $(filter-out $(RECEIVER_OBJ),$(CORE_OBJ))
	$(CXX) $^ -o $@ $(LDFLAGS) -lz

# =============================
# Build receiver
# =============================
$(RECEIVER_BIN): $(RECEIVER_OBJ) $(filter-out $(SENDER_OBJ),$(CORE_OBJ))
	$(CXX) $^ -o $@ $(LDFLAGS) -lz

# =============================
# Build test binary
# =============================
$(TEST_BIN): $(CORE_OBJ) $(TEST_OBJ)
	$(CXX) $^ -lz -o $@ -lgtest -lgtest_main -pthread $(LDFLAGS)

test_headers: $(TEST_BIN)
	cp $(TEST_BIN) test_headers

# =============================
# Compile rules
# =============================
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CXX_INC) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CXX_INC) -c $< -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CXX_INC) -c $< -o $@

# =============================
# Create build directory
# =============================
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# =============================
# Cleaning
# =============================
clean:
	rm -rf $(BUILD_DIR) test_headers $(SENDER_BIN) $(RECEIVER_BIN)

.PHONY: all clean test
