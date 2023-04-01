
CC=g++
CC_FLAGS=-std=gnu++03 -g -D__MACH=1 -D__unix__=1 -O0

DIST=./dist/
SRC=./src/
INC=./include
TARGET=$(DIST)/zumo

SRC_FILES=$(shell find $(SRC) -name "*.cc")
OBJ_FILES=$(patsubst $(SRC)/%.cc, $(DIST)/%.o, $(SRC_FILES))

INC_DIRS= $(INC) ./mock/ \
#	.pio/libdeps/32u416m/PubSubClient/src \
#	$(PIO_DIR)/packages/framework-arduino-avr/cores/arduino/ \
#	$(PIO_DIR)/packages/toolchain-atmelavr/avr/include

#INC_DIRS+=$(shell find $(PIO_DIR) -wholename "*/include")

CC_FLAGS += $(addprefix -I, $(INC_DIRS))

LIB_DIRS=
LINKS=

all: $(TARGET)

dir_guard=@mkdir -p $(@D)

$(TARGET): $(OBJ_FILES)
	$(dir_guard)
	@$(CC) -o $@ $^ $(LIB_DIRS) $(LINKS)

$(DIST)/%.o: $(SRC)/%.cc
	$(dir_guard)
	@echo $(CC_FLAGS)
	@$(CC) -c $(CC_FLAGS) -o $@ $^
