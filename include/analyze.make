$(info Setting up analyze rule.)

CLANG_TIDY_CHECKS=clang-analyzer-*
CLANG_TIDY=-checks=$(CLANG_TIDY_CHECKS) -- -I$(INCLUDE_DIR)
SARIF_FILES=$(addsuffix .sarif,$(notdir $(SRC_FILES)))

analyze_artifacts: $(OBJ_FILES)
	@mv $(SARIF_FILES) $(ARTIFACTS_DIR)/

build_pipeline/%.o: %.c
	@echo "$(PURPLE)Running GCC's static analyzer on file $^$(END_COLOR)"
	gcc -I$(INCLUDE_DIR) -fanalyzer -c -o /dev/null $^
	gcc -I$(INCLUDE_DIR) -fanalyzer -fdiagnostics-format=sarif-file -c -o /dev/null $^
	@echo "$(PURPLE)Running clang-tidy on file $^$(END_COLOR)"
	clang-tidy-19 $^ $(CLANG_TIDY)