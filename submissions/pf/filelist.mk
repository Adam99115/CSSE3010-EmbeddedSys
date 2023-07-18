########################################################################
# LIBRARY SOURCES - MUST BE IN THE SAME FOLDER as main.c (DO NOT CHANGE)
########################################################################

# Set mylib folder path.
# Do not change the MYLIB_PATH variable name.
# ONLY use relative file paths that start with $(SOURCELIB_ROOT)../
# DO NOT USE absolute file paths (e.g. /home/users/myuser/mydir)
# e.g. MYLIBPATH=$(SOURCELIB_ROOT)/../mylib
MYLIB_PATH=$(SOURCELIB_ROOT)/../repo/mylib
SSD1306_PATH=$(SOURCELIB_ROOT)/components/peripherals/ssd1306/nucleo-f429

# Set folder path with header files to include.
# ONLY use relative file paths that start with $(SOURCELIB_ROOT)../
# DO NOT USE absolute file paths (e.g. /home/users/myuser/mydir)
CFLAGS += -I$(MYLIB_PATH)
CFLAGS += -I$(SSD1306_PATH)


# List all c files locations that must be included (use space as separate 
# e.g. LIBSRCS += path_to/file1.c path_to/file2.c)
# ONLY use relative file paths that start with $(SOURCELIB_ROOT)../
# DO NOT USE absolute file paths (e.g. /home/users/myuser/mydir)

LIBSRCS += $(wildcard $(SSD1306_PATH)/*.c)
LIBSRCS += $(FREERTOS_PATH)/portable/MemMang/heap_4.c
LIBSRCS += $(MYLIB_PATH)/s4582547_keypad.c
LIBSRCS += $(MYLIB_PATH)/s4582547_hamming.c
LIBSRCS += $(MYLIB_PATH)/s4582547_ascsys.c
LIBSRCS += $(MYLIB_PATH)/s4582547_ascext.c
LIBSRCS += $(MYLIB_PATH)/s4582547_oled.c
LIBSRCS += $(MYLIB_PATH)/s4582547_txradio.c
