
CC=g++
CC_FLAGS=-std=gnu++11 -g -DMCU__=0 -D__unix__=1 -O0 -DLOG_LEVEL=0  \
	-DLOG_USE_BUF=0 -DLOG_SAVE_ON_WRITE=1

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
	$(CC) -c $(CC_FLAGS) -o $@ $^
