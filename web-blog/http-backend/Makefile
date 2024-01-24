CC = gcc
CFLAGS = -Wall -O2
SRC = ./http.c
TARGET = ./httpd

LIB_JSON_PATH = ../../lib-json/
LIB_LIST_PATH = ../../lib-list/
LIB_TREE_PATH = ../../lib-tree/

OBJ_JSON = $(wildcard $(LIB_JSON_PATH)json.c)
OBJ_LIST = $(wildcard $(LIB_LIST_PATH)list.c)
OBJ_TREE = $(wildcard $(LIB_TREE_PATH)tree.c)

all: $(TARGET)

$(TARGET): $(SRC) $(OBJ_JSON) $(OBJ_LIST) $(OBJ_TREE) 
	$(CC) $(CFLAGS) -o $@ $^ -I$(LIB_JSON_PATH) -I$(LIB_LIST_PATH) -I$(LIB_TREE_PATH) 

clean:
	rm -f $(TARGET)
