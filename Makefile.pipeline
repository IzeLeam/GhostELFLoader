include Makefile.config

# vpath %.c src

ifndef SRC_FILES
$(error MAKE variable SRC_FILES is not define, please define it in your Makefile.)
endif

ifndef INCLUDE_DIR
$(warning MAKE variable INCLUDE_DIR is not define, using root of the repo.)
$(warning Please create it in your Makefile with a value equal to the directory containing your headers if the pipeline fails.)
INCLUDE_DIR=.
endif
$(info Using $(INCLUDE_DIR) as include directory.)

EMPTY=
SPACE=$(EMPTY) $(EMPTY)
COMMA_SPACE=,$(SPACE)

RED=\e[31m
GREEN=\e[1;32m
BLUE=\e[1;34m
PURPLE=\e[1;35m
END_COLOR=\e[0m

GCC_CFLAGS=-O2 -Warray-bounds -Wsequence-point -Walloc-zero -Wnull-dereference -Wpointer-arith -Wcast-qual -Wcast-align=strict
CLANG_CFLAGS= -Wall -Wextra -Wuninitialized -Wpointer-arith -Wcast-qual -Wcast-align -fPIE

OBJ_FILES=$(patsubst %.c,./build_pipeline/%.o,$(notdir $(SRC_FILES)))
ARTIFACTS_DIR=./artifacts
TARGET=$(ARTIFACTS_DIR)/isos_loader

pipeline_build: $(TARGET)
	@echo "$(PURPLE)Project built.$(END_COLOR)"

pipeline_analyze: analyze_artifacts
	@echo "$(PURPLE)Project analyzed.$(END_COLOR)"

ifdef BUILD
include /root/include/build.make
else ifdef ANALYZE
include /root/include/analyze.make
endif