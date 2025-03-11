$(info Setting up build rule.)

LIBS=-lbfd
SANITIZED_DIR=$(ARTIFACTS_DIR)/instrumented_binary

$(TARGET): $(OBJ_FILES)
	@echo "$(PURPLE)Linking objects files [$(BLUE)$(subst $(SPACE),$(COMMA_SPACE),$(OBJ_FILES))$(PURPLE)] into binary $(GREEN)$@$(END_COLOR)"
	gcc -I$(INCLUDE_DIR) $(LIBS) $(GCC_CFLAGS) -o $@ $^
	@ mkdir -p $(SANITIZED_DIR)
	@echo "$(PURPLE)Building the instrumented binary$(END_COLOR)"
	@echo "$(PURPLE) -> Address Sanitizer$(END_COLOR)"
	clang-19 -I$(INCLUDE_DIR) $(LIBS) $(CLANG_CFLAGS) -fsanitize=address -o $(SANITIZED_DIR)/$(notdir $@)_addr_san $(subst .o,_clang.o,$^)
	@echo "$(PURPLE) -> Memory Sanitizer$(END_COLOR)"
	clang-19 -I$(INCLUDE_DIR) $(LIBS) $(CLANG_CFLAGS) -fsanitize=memory -o $(SANITIZED_DIR)/$(notdir $@)_mem_san $(subst .o,_clang.o,$^)
	@echo "$(PURPLE) -> Undefined Behavior Sanitizer$(END_COLOR)"
	clang-19 -I$(INCLUDE_DIR) $(LIBS) $(CLANG_CFLAGS) -fsanitize=undefined -o $(SANITIZED_DIR)/$(notdir $@)_ub_san $(subst .o,_clang.o,$^)
	@echo "$(RED)Do not forget to execute those different sanitized programs with different control flow paths.$(END_COLOR)"

build_pipeline/%.o: %.c
	@echo "$(PURPLE)Building object file $@ for file $^$(END_COLOR)"
	gcc -I$(INCLUDE_DIR) $(GCC_CFLAGS) -o $@ -c $^
	clang-19 -I$(INCLUDE_DIR) $(CLANG_CFLAGS) -o $(subst .o,_clang.o,$@) -c $^