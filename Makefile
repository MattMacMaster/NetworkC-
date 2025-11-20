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
# Targets and source discovery
# =============================
TARGETS   := test_headers

# Find source files
SRC_FILES := $(wildcard $(SRC_DIR)/*.cc)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SRC_FILES))

# Find test files
TEST_FILES := $(wildcard $(TEST_DIR)/*.cc)
TEST_OBJ   := $(patsubst $(TEST_DIR)/%.cc,$(BUILD_DIR)/%.o,$(TEST_FILES))
TEST_BIN   := $(BUILD_DIR)/test_headers  # Test binary

# =============================
# Default rule
# =============================
all: $(TARGETS)

# =============================
# Link step for programs (test binary)
# =============================
$(TARGETS): $(OBJ_FILES) $(TEST_OBJ) | $(BUILD_DIR)
	$(CXX) $^ -o $@ -lgtest -lgtest_main -pthread $(LDFLAGS)

# =============================
# Compile rules
# =============================
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CXX_INC) -c $< -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CXX_INC) -c $< -o $@

# =============================
# Test target (Google Test)
# =============================
test: $(TEST_BIN)
	$(TEST_BIN)

$(TEST_BIN): $(TEST_OBJ) $(OBJ_FILES) | $(BUILD_DIR)
	$(CXX) $^ -o $@ -lgtest -lgtest_main -pthread $(LDFLAGS)

# =============================
# Directory creation
# =============================
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# =============================
# Cleaning
# =============================
clean:
	rm -rf $(BUILD_DIR) $(TARGETS)

.PHONY: all clean test
