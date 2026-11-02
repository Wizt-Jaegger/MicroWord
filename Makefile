CC       ?= cc
CFLAGS   ?= -std=c99 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS += -Isrc -MMD -MP          # -Isrc: includes look like "core/util.h"

BIN      = microword
BUILD    = build

# main.c + every .c in a subfolder of src/ (new files are picked up automatically)
SRC      = src/main.c $(wildcard src/*/*.c)
OBJ      = $(SRC:src/%.c=$(BUILD)/%.o)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

$(BUILD)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD) $(BIN)

# Compares every language file against english.txt (missing / extra keys)
check-lang:
	@for f in Idiomas/*.txt; do \
	  case $$f in */english.txt|*/idiomas.txt) continue;; esac; \
	  grep -E '^[a-z0-9_.]+=' Idiomas/english.txt | cut -d= -f1 | sort > $(BUILD).k_en; \
	  grep -E '^[a-z0-9_.]+=' $$f | cut -d= -f1 | sort > $(BUILD).k_x; \
	  echo "$$f  missing: [$$(comm -23 $(BUILD).k_en $(BUILD).k_x | tr '\n' ' ')]  extra: [$$(comm -13 $(BUILD).k_en $(BUILD).k_x | tr '\n' ' ')]"; \
	done; rm -f $(BUILD).k_en $(BUILD).k_x

-include $(OBJ:.o=.d)
.PHONY: clean check-lang
